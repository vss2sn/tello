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
  n_retries_allowed_(n_retries_allowed),
  timeout_(timeout)
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
  io_thread = std::thread([&]{io_service_.run();});
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
   response_ = std::string(data_);
   //remove additional random characters sent over UDP
   if(response_.substr(0,2)=="ok"){
     response_ = "ok";
   }
   else if (response_.substr(0,5)=="error"){
     response_ = "error";
   }
   else if (response_.substr(0,11)=="forced stop"){
     response_ = "forced stop";
   }
   else{
     response_ = "UNKNOWN";
   }
   LogInfo() << "Received response [" << response_ << "] after sending command ["<< last_command_ << "] from address [" << drone_ip_ << ":" << drone_port_ << "]";
 }
 else{
   LogWarn() << "Nothing received" ;
 }
 ASYNC_RECEIVE;
}

void CommandSocket::sendCommand(const std::string& cmd){
  n_retries_ = 0;
  ASYNC_SEND;
  // TODO: Check whether this works with all the updates
  // NOTE: Do not comment. Set n_retries_allowed_ to 0 if required.
#ifdef USE_BOOST
  boost::thread run_thread(boost::bind(&CommandSocket::retry, this, cmd));
#else
  std::thread run_thread([&]{retry(cmd);});
  run_thread.detach();
#endif
  usleep(1000);
}

void CommandSocket::waitForResponse(){
  std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
  while(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start).count() < timeout_){
    if(!waiting_for_response_){
      break;
    }
    usleep(100000);
  }
  if(waiting_for_response_) LogInfo() << "Timeout - Attempt #" << n_retries_ << " for command [" << last_command_ << "]";
  if(n_retries_ == n_retries_allowed_){
    if(n_retries_allowed_ > 0){
      LogWarn() << "Exhausted retries" ;
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
   LogInfo() << "Successfully sent command [" << cmd << "] to address [" << drone_ip_ << ":" << drone_port_ << "]";
   last_command_ = cmd;
 }
 else{
   LogDebug() << "Failed to send command [" << cmd <<"]";
 }
}

void CommandSocket::retry(const std::string& cmd){
  waitForResponse();
  if(!waiting_for_response_) return;
  while(n_retries_ < n_retries_allowed_ && waiting_for_response_){
    LogInfo() << "Retrying..." ;
    n_retries_++;
    ASYNC_SEND;
    last_command_ = cmd;
    command_sent_time_ = std::chrono::system_clock::now();
    waitForResponse();
  }
}

void CommandSocket::addCommandToQueue(const std::string& cmd){
  queue_mutex_.lock();
  LogInfo() << "Added command ["<< cmd<<"] to queue";
  command_queue_.push_back(cmd);
  queue_mutex_.unlock();
  cv_execute_queue_.notify_all();
}

void CommandSocket::executeQueue(){
  LogInfo() << "Executing queue commands";
  {
    std::lock_guard<std::mutex> lk(m);
    execute_queue_ = true;
  }
  cv_execute_queue_.notify_all();
}

void CommandSocket::sendQueueCommands(){
  std::string cmd;
  while(true){
    usleep(1000); // aloow others to grab lock
    {
      std::unique_lock<std::mutex> lk(m);
      cv_execute_queue_.wait(lk, [this]{return execute_queue_ && !command_queue_.empty();});
    }
    if(!on_) break;
    while(!command_queue_.empty() && execute_queue_){
        // {
        //   std::unique_lock<std::mutex> lk(m);
        //   cv_execute_queue_.wait(lk, [this]{return execute_queue_ && !command_queue_.empty();});
        // }
        // if(!on_) break;
      queue_mutex_.lock();
      cmd = command_queue_.front();
      command_queue_.pop_front();
      queue_mutex_.unlock();
      if(cmd.substr(0,5) == "delay"){
        usleep(1000000 * stoi(cmd.substr(5, cmd.size())));
        continue;
      }
      if(cmd.substr(0,4) =="stop" || cmd.substr(0,9) =="emergency"){
        waiting_for_response_ = false; // to prevent retries of prev sent command if none received in spit of comman dbeing sent.
      }
      // Run reponse thread only after sending command rather than always
      if(waiting_for_response_){
        usleep(10000); // Next send command grabs mutex before handleSendCommand.
        // TODO: The abpve should no longer be required
        LogInfo() << "Waiting to send command [" << cmd << "] as no response has been received for the previous command. Thread calling send command paused." ;
      }
      while(waiting_for_response_){
        if(!on_) break;
        usleep(500000);
      }
      waiting_for_response_ = true;
      sendCommand(cmd);
      command_sent_time_ = std::chrono::system_clock::now();
    }
  }
}

void CommandSocket::addCommandToFrontOfQueue(const std::string& cmd){
  queue_mutex_.lock();
  command_queue_.push_front(cmd);
  queue_mutex_.unlock();
}

void CommandSocket::stopQueueExecution(){
  LogInfo() << "Stopping queue execution. " << command_queue_.size() << " commands still in queue.";
  std::lock_guard<std::mutex> lk(m);
  execute_queue_ = false;
  LogDebug() << "execute_queue_ set to false";
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
  LogInfo() << "Removing command [" << cmd << "] from queue.";
}

void CommandSocket::doNotAutoLand(){
  LogDebug() << "Auto land disbled";
  std::lock_guard<std::mutex> lk(dnal_mutex);
  dnal_ = true;
  cv_dnal_.notify_all();
  LogInfo() << execute_queue_ << " " << dnal_;
}

void CommandSocket::allowAutoLand(){
  LogDebug() << "Autoland enabled";
  std::lock_guard<std::mutex> lk(dnal_mutex);
  dnal_ = false;
  cv_dnal_.notify_all();
}

// TODO: test this
void CommandSocket::doNotAutoLandWorker(){
  std::chrono::system_clock::time_point start;
  std::string cmd = "rc 0 0 0 0";
  sendCommand(cmd);
  int dnal_timeout_in_us = dnal_timeout*1000000;
  while(true){
    {
      auto a = ((!execute_queue_) || (execute_queue_ && command_queue_.empty())) && dnal_;
      LogDebug() << "Before lock: "<< a;
      std::unique_lock<std::mutex> lk(dnal_mutex);
      cv_dnal_.wait(lk, [this]{return ((!execute_queue_) || (execute_queue_ && command_queue_.empty())) && dnal_;});
      LogDebug() << "After lock " << a;
    }
    LogDebug() << "DNAL: "<< dnal_;
    if(!on_) break;
    if(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - command_sent_time_).count() > dnal_timeout){
      sendCommand(cmd);
      last_command_ = cmd;
      waiting_for_response_ = false;
      command_sent_time_ = std::chrono::system_clock::now();
    }
    else{
      usleep(dnal_timeout_in_us); //10 seconds, command needs to be sent ever 15
    }
  }
}

void CommandSocket::stop(){
  {
    std::lock_guard<std::mutex> lk(m);
    execute_queue_ = false;
  }
  sendCommand("stop");
}

void CommandSocket::emergency(){
  {
    std::lock_guard<std::mutex> lk(m);
    execute_queue_ = false;
  }
  sendCommand("emergency");
}

CommandSocket::~CommandSocket(){
  dnal_ = true; // allow dnal thread to exit
  on_ = false;
  {
    std::lock_guard<std::mutex> lk(m);
    execute_queue_ = true;
    command_queue_.push_back("stop");
    cv_execute_queue_.notify_all();
  }
}

bool CommandSocket::isExecutingQueue(){
  return execute_queue_;
}
