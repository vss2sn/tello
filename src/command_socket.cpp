#include "command_socket.hpp"
#include "utils.hpp"

CommandSocket::CommandSocket(boost::asio::io_service& io_service,
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
  boost::asio::ip::udp::resolver resolver(io_service_);
  boost::asio::ip::udp::resolver::query query(boost::asio::ip::udp::v4(), drone_ip_, drone_port_);
  boost::asio::ip::udp::resolver::iterator iter = resolver.resolve(query);
  endpoint_ = *iter;

  socket_.async_receive_from(
    boost::asio::buffer(data_, max_length_),
    endpoint_,
    boost::bind(&CommandSocket::handleResponseFromDrone,
                this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));

  boost::thread run_thread(boost::bind(&boost::asio::io_service::run, boost::ref(io_service_)));

  worker = std::thread(&CommandSocket::sendQueueCommands, this);
}

void CommandSocket::handleResponseFromDrone(const boost::system::error_code& error,
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
   LogInfo() << "Error/Nothing received" ;
 }
 socket_.async_receive_from(
   boost::asio::buffer(data_, max_length_),
   endpoint_,
   boost::bind(&CommandSocket::handleResponseFromDrone,
     this,
     boost::asio::placeholders::error,
     boost::asio::placeholders::bytes_transferred));
}

void CommandSocket::sendCommand(const std::string& cmd){
  n_retries_ = 0;
  socket_.async_send_to(
    boost::asio::buffer(cmd, cmd.size()),
    endpoint_,
    boost::bind(&CommandSocket::handleSendCommand,
      this,
      boost::asio::placeholders::error,
      boost::asio::placeholders::bytes_transferred,
      cmd));
  if(n_retries_allowed_) boost::thread run_thread(boost::bind(&CommandSocket::retry, this, cmd));
}

void CommandSocket::waitForResponse(){
  std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
  while(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start).count() < timeout_){
    if(!waiting_for_response_){
      break;
    }
    usleep(100000);
  }
  if(waiting_for_response_) LogInfo() << "Timeout - Attempt #" << n_retries_;
  if(n_retries_ == n_retries_allowed_){
    if(n_retries_allowed_ > 0){
      LogInfo() << "Exhausted retries" ;
    }
    waiting_for_response_ = false; // Timeout
  }
}

void CommandSocket::handleSendCommand(const boost::system::error_code& error,
                                      size_t bytes_sent,
                                      const std::string& cmd)
{
 if(!error && bytes_sent>0){
   LogInfo() << "Successfully sent command [" << cmd << "] to address [" << drone_ip_ << ":" << drone_port_ << "]";
   last_command_ = cmd;
 }
 else{
   LogInfo() << "Failed to send command [" << cmd <<"]";
 }
}

void CommandSocket::retry(const std::string& cmd){
  waitForResponse();
  if(!waiting_for_response_) return;
  while(n_retries_ < n_retries_allowed_ && waiting_for_response_){
    LogInfo() << "Retrying..." ;
    n_retries_++;
    socket_.async_send_to(
        boost::asio::buffer(cmd, cmd.size()),
        endpoint_,
        boost::bind(&CommandSocket::handleSendCommand,
          this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred,
          cmd));
    last_command_ = cmd;
    command_sent_time = std::chrono::system_clock::now();
    waitForResponse();
  }
}

void CommandSocket::addCommandToQueue(const std::string& cmd){
  queue_mutex_.lock();
  command_queue_.push_back(cmd);
  queue_mutex_.unlock();
  cv.notify_one();
}

void CommandSocket::executeQueue(){
  LogInfo() << "Executing queue commands";
  // need to reserfev a thread for execute queue that needs to be pausedand unpaused.
  // boost::thread run_thread(&CommandSocket::sendQueueCommands, this);
  {
    std::lock_guard<std::mutex> lk(m);
    execute_queue_ = true;
    std::cout << "executeQueue() signals data ready for processing\n";
  }
  cv.notify_one();
}

void CommandSocket::sendQueueCommands(){
  std::string cmd;
  while(true){
    std::unique_lock<std::mutex> lk(m);
    cv.wait(lk, [this]{return execute_queue_ && !command_queue_.empty();});
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
      if(cmd.substr(0,4) =="stop"){
        waiting_for_response_ = false; // to prevent retries of prev sent command if none received in spit of comman dbeing sent.
        continue;
      }
      // Run reponse thread only after sending command rather than always
      if(waiting_for_response_){
        usleep(10000); // Next send command grabs mutex before handleSendCommand.
        // The abpve should no longer be required
        LogInfo() << "Waiting to send command [" << cmd << "] as no response has been received for the previous command. Thread calling send command paused." ;
      }
      while(waiting_for_response_){
        usleep(500000);
      }
      waiting_for_response_ = true;

      sendCommand(cmd);
      command_sent_time = std::chrono::system_clock::now();
    }
    lk.unlock();
  }
}

void CommandSocket::addCommandToFrontOfQueue(const std::string& cmd){
  queue_mutex_.lock();
  command_queue_.push_front(cmd);
  queue_mutex_.lock();
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
  LogInfo() << "Removing command [" << cmd << "] from queue.";
}

void CommandSocket::DoNotLand(){
  std::chrono::system_clock::time_point start;
  while(do_not_land){
    if(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - command_sent_time).count() > do_not_land_timeout){
      sendCommand("rc 0 0 0 0");
      last_command_ = "rc 0 0 0 0";
      waiting_for_response_ = false;
      command_sent_time = std::chrono::system_clock::now();
    }
    else{
      usleep(10000000); //10 seconds, command needs to be sent ever 15
    }
  }
}

CommandSocket::~CommandSocket(){
  execute_queue_ = false;
  on_ = false;
  io_service_.stop();
  worker.join();
}
