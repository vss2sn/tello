#include "command_socket.hpp"
#include "utils.hpp"

#ifdef USE_BOOST
#define UDP boost::asio::ip::udp
#define ASYNC_RECEIVE socket_.async_receive_from( boost::asio::buffer(data_, max_length_), endpoint_, boost::bind(&CommandSocket::handleResponseFromDrone, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
#define ASYNC_SEND socket_.async_send_to( boost::asio::buffer(cmd, cmd.size()),endpoint_, boost::bind(&CommandSocket::handleSendCommand,this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred,cmd));
#else
#define UDP asio::ip::udp
#define ASYNC_RECEIVE socket_.async_receive_from( asio::buffer(data_, max_length_), endpoint_, [&](const std::error_code& error, size_t bytes_recvd) {return handleResponseFromDrone(error, bytes_recvd);}); // [&](auto... args){return handleResponseFromDrone(args...);});
#define ASYNC_SEND socket_.async_send_to( asio::buffer(cmd, cmd.size()), endpoint_, [&](const std::error_code& error, size_t bytes_recvd) {return handleSendCommand(error, bytes_recvd, cmd);}); // [&](auto... args){return handleSendCommand(args..., cmd);});
#endif

// NOTE: Possible methods to call threads
// io_thread(boost::bind(&boost::asio::io_service::run, boost::ref(io_service_)));
// io_thread = std::thread([&]{io_service_.run();});
// io_thread = std::thread( static_cast<std::size_t(boost::asio::io_service::*)()>(&boost::asio::io_service::run), io_service);
// cmd_thread = std::thread(&CommandSocket::sendQueueCommands, this);
// cmd_thread = std::thread([&](void){sendQueueCommands();});

CommandSocket::CommandSocket(
#ifdef USE_BOOST
  boost::asio::io_service& io_service,
#else
  asio::io_service& io_service,
#endif
  const std::string& drone_ip,
  const std::string& drone_port,
  const std::string& local_port,
  int n_retries_allowed,
  int timeout
):
  BaseSocket(io_service, drone_ip, drone_port, local_port),
  timeout_(timeout),
  n_retries_allowed_(n_retries_allowed)
{
  // NOTE: Used #define UDP to handle namespace
  UDP::resolver resolver(io_service_);
  UDP::resolver::query query(UDP::v4(), drone_ip_, drone_port_);
  UDP::resolver::iterator iter = resolver.resolve(query);
  endpoint_ = *iter;
#ifdef USE_BOOST
  io_thread = boost::thread(boost::bind(&boost::asio::io_service::run, boost::ref(io_service_)));
  cmd_thread = boost::thread(boost::bind(&CommandSocket::sendQueueCommands, this));
  dnal_thread = boost::thread(boost::bind(&CommandSocket::doNotAutoLandWorker, this));
#else
  io_thread = std::thread([&]{io_service_.run();
    LogDebug() << "----------- Command socket io_service thread exits -----------";
  });
  cmd_thread = std::thread(&CommandSocket::sendQueueCommands, this);
  dnal_thread = std::thread(&CommandSocket::doNotAutoLandWorker, this);
  io_thread.detach();
  cmd_thread.detach();
  dnal_thread.detach();
#endif
  command_sent_time_ = std::chrono::system_clock::now();
  ASYNC_RECEIVE;
}

void CommandSocket::handleResponseFromDrone(
#ifdef USE_BOOST
const boost::system::error_code& error,
#else
const std::error_code& error,
#endif
size_t bytes_recvd)
{
 if(!error && bytes_recvd>0){
   waiting_for_response_ = false;
   response_ = "";
   const std::string response_temp = std::string(data_);
   //remove additional random characters sent over UDP
   // TODO: Make this better
   for(int i=0; i<bytes_recvd && isprint(response_temp[i]); ++i){
     response_+=response_temp[i];
   }
   LogInfo() << "Received response [" << response_ << "] after sending command ["<< last_command_ << "] from address [" << drone_ip_ << ":" << drone_port_ << "].";
 }
 else{
   LogWarn() << "Error/Nothing received.";
 }
 ASYNC_RECEIVE;
}

void CommandSocket::sendCommand(const std::string& cmd){
  n_retries_ = 0;
  ASYNC_SEND;
  usleep(1000); //TODO: reduce this to less than amount of time joystick waits?
}

void CommandSocket::waitForResponse(){
  std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
  while(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start).count() < timeout_ && on_){
    if(!waiting_for_response_){
      break;
    }
    usleep(100000);// TODO: replace with condition variable
  }
  if(!on_){
    waiting_for_response_ = false;
    return;
  }
  if(waiting_for_response_) LogInfo() << "Timeout - Attempt #" << n_retries_ << " for command [" << last_command_ << "].";
  if(n_retries_ == n_retries_allowed_){
    if(n_retries_allowed_ > 0){
      LogWarn() << "Exhausted retries." ;
    }
    waiting_for_response_ = false; // Timeout
  }
}

void CommandSocket::handleSendCommand(
#ifdef USE_BOOST
const boost::system::error_code& error,
#else
const std::error_code& error,
#endif
size_t bytes_sent, std::string cmd)
{
 if(!error && bytes_sent>0){
   LogInfo() << "Successfully sent command [" << cmd << "] to address [" << drone_ip_ << ":" << drone_port_ << "].";
   last_command_ = cmd;
   command_sent_time_ = std::chrono::system_clock::now();
 }
 else{
   LogDebug() << "Failed to send command [" << cmd <<"].";
 }
}

void CommandSocket::retry(const std::string& cmd){
  last_command_ = cmd;
  waitForResponse();
  if(!waiting_for_response_) return;
   while(n_retries_ < n_retries_allowed_ && waiting_for_response_ && on_){
    LogInfo() << "Retrying..." ;
    n_retries_++;
    ASYNC_SEND;
    waitForResponse();
  }
}

void CommandSocket::addCommandToQueue(const std::string& cmd){
  queue_mutex_.lock();
  LogInfo() << "Added command ["<< cmd<<"] to queue.";
  command_queue_.push_back(cmd);
  queue_mutex_.unlock();
  cv_execute_queue_.notify_all();
}

void CommandSocket::executeQueue(){
  LogInfo() << "Executing queue commands.";
  execute_queue_ = true;
  {
    std::lock_guard<std::mutex> lk(m);
    cv_execute_queue_.notify_all();
  }
}

void CommandSocket::sendQueueCommands(){
  std::string cmd;
  while(on_){
    usleep(1000); // allow others to grab lock
    {
      std::unique_lock<std::mutex> lk(m);
      cv_execute_queue_.wait(lk, [this]{return (execute_queue_ && !command_queue_.empty()) || !on_;});
    }
    if(!on_) break;
    while(!command_queue_.empty() && execute_queue_){
      queue_mutex_.lock();
      cmd = command_queue_.front();
      command_queue_.pop_front();
      queue_mutex_.unlock();
      if(cmd.substr(0,5) == "delay"){
        usleep(1000000 * stoi(cmd.substr(5, cmd.size())));
        continue;
      }
      else if(cmd.substr(0,4) =="stop" || cmd.substr(0,9) =="emergency"){
        waiting_for_response_ = false; // to prevent retries of prev sent command if none received in spit of comman dbeing sent.
      }
      // Run reponse thread only after sending command rather than always
      // TODO: Use condition variable for the wait for response
      if(waiting_for_response_){
        // usleep(10000); // Next send command grabs mutex before handleSendCommand.
        // TODO: The above should no longer be required
        LogInfo() << "Waiting to send command [" << cmd << "] as no response has been received for the previous command. Thread calling send command paused." ;
      }
      while(waiting_for_response_){
        if(!on_) break;
        usleep(500000);
      }
      waiting_for_response_ = true;
      sendCommand(cmd);
      // NOTE: Do not comment. Set n_retries_allowed_ to 0 if required.
      // If the commmand is rc, do not retry/wait for a response
      if(cmd.substr(0,2)!="rc"){
        #ifdef USE_BOOST
          boost::thread run_thread(boost::bind(&CommandSocket::retry, this, cmd));
        #else
          std::thread run_thread([&]{retry(cmd);});
          run_thread.detach();
        #endif
      }
    }
  }
  LogDebug() << "----------- Send queue commands thread exits -----------";
}

void CommandSocket::addCommandToFrontOfQueue(const std::string& cmd){
  queue_mutex_.lock();
  command_queue_.push_front(cmd);
  queue_mutex_.unlock();
}

void CommandSocket::stopQueueExecution(){
  LogInfo() << "Stopping queue execution. " << command_queue_.size() << " commands still in queue.";
  execute_queue_ = false;
}

void CommandSocket::clearQueue(){
  LogInfo() <<  "Clearing queue.";
  queue_mutex_.lock();
  command_queue_.clear();
  queue_mutex_.unlock();
}

void CommandSocket::removeNextFromQueue(){
  queue_mutex_.lock();
  std::string cmd = command_queue_.front();
  command_queue_.pop_front();
  queue_mutex_.unlock();
  LogInfo() << "Removed command [" << cmd << "] from queue.";
}

void CommandSocket::doNotAutoLand(){
  LogDebug() << "Automatic landing disabled.";
  dnal_ = true;
  {
    std::lock_guard<std::mutex> lk(dnal_mutex);
    cv_dnal_.notify_all();
  }
}

void CommandSocket::allowAutoLand(){
  LogDebug() << "Automatic landing enabled.";
  dnal_ = false;
  // TODO: Check whether this is required
  // {
  //   std::lock_guard<std::mutex> lk(dnal_mutex);
  //   cv_dnal_.notify_all();
  // }
}

void CommandSocket::doNotAutoLandWorker(){
  std::chrono::system_clock::time_point start;
  std::string cmd = "rc 0 0 0 0";
  sendCommand(cmd);
  int dnal_timeout_in_us = dnal_timeout*1000000;
  while(on_){
    {
      std::unique_lock<std::mutex> lk(dnal_mutex);
      cv_dnal_.wait(lk, [this]{return (((!execute_queue_) || (execute_queue_ && command_queue_.empty())) && dnal_) || !on_;});
    }
    if(!on_) break;
    if(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - command_sent_time_).count() > dnal_timeout){
      sendCommand(cmd);
      last_command_ = cmd;
      waiting_for_response_ = false;
    }
    else{
      usleep(dnal_timeout_in_us); //10 seconds, command needs to be sent ever 15
    }
  }
  LogDebug() << "----------- DNAL worker thread exits -----------";
}

void CommandSocket::stop(){
  execute_queue_ = false;
  sendCommand("stop");
}

void CommandSocket::emergency(){
  execute_queue_ = false;
  sendCommand("emergency");
}

bool CommandSocket::isExecutingQueue(){
  return execute_queue_;
}

void CommandSocket::land(){
  allowAutoLand();
  sendCommand("land");
}

CommandSocket::~CommandSocket(){
  dnal_ = true; // allow dnal thread to exit
  execute_queue_ = false; // also allow dnal thread to exit
  on_ = false;
  {
    std::lock_guard<std::mutex> lk(dnal_mutex);
    cv_dnal_.notify_all();
    usleep(1000); // allow dnal thread to exit before setting execute_queue_ to true.
  }
  execute_queue_ = true;
  {
    std::lock_guard<std::mutex> lk(m);
    cv_execute_queue_.notify_all();
  }
}
