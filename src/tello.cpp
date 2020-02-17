#include "tello.hpp"
#include "utils.hpp"

Tello::Tello(boost::asio::io_service& io_service,
  const std::string& drone_ip,
  const std::string& drone_port,
  const std::string& local_port,
  int n_retries_allowed,
  int timeout)
  :
  io_service_(io_service),
  local_port_(local_port),
  drone_ip_(drone_ip),
  drone_port_(drone_port),
  n_retries_allowed_(n_retries_allowed),
  timeout_(timeout),
  socket_(io_service_, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), std::stoi(local_port)))
{
  boost::asio::ip::udp::resolver resolver(io_service_);
  boost::asio::ip::udp::resolver::query query(boost::asio::ip::udp::v4(), drone_ip_, drone_port_);
  boost::asio::ip::udp::resolver::iterator iter = resolver.resolve(query);
  endpoint_ = *iter;

  socket_.async_receive_from(
    boost::asio::buffer(data_, max_length_),
    endpoint_,
    boost::bind(&Tello::handleResponseFromDrone,
                this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));

  boost::thread run_thread(boost::bind(&boost::asio::io_service::run, boost::ref(io_service_)));
}

void Tello::handleResponseFromDrone(const boost::system::error_code& error,
                       size_t bytes_recvd)
{
 // response_ = "";
 if(!error && bytes_recvd>0){
   response_ = std::string(data_);
   //remove additional random characters sent over UDP
   if(response_.substr(0,2)=="ok") response_ = "ok";
   else if (response_.substr(0,5)=="error") response_ = "error";
   else{
     response_ = "UNKNOWN";
     LogInfo() << "Alert! Unknown response: " << data_ ;
   }
   LogInfo() << "Received response [" << response_ << "] after sending command ["<< last_command_ << "] from address [" << drone_ip_ << ":" << drone_port_ << "]";
   received_response_ = true;
 }
 else{
   LogInfo() << "Nothing received" ;
 }
 socket_.async_receive_from(
   boost::asio::buffer(data_, max_length_),
   endpoint_,
   boost::bind(&Tello::handleResponseFromDrone,
     this,
     boost::asio::placeholders::error,
     boost::asio::placeholders::bytes_transferred));
}

void Tello::sendCommand(const std::string& cmd){

  // Run reponse thread only after sending command rather than always
  if(!received_response_){
    usleep(10000); // Next send command grabs mutex before handleSendCommand.
    LogInfo() << "Waiting to send [command] " << cmd << " as no response has been received for the previous command. Thread calling send command paused." ;
  }
  while(!received_response_){
    // LogInfo() << "In while";
    usleep(500000);
  }

  received_response_ = false;
  n_retries_ = 0;

 socket_.async_send_to(
     boost::asio::buffer(cmd, cmd.size()),
     endpoint_,
     boost::bind(&Tello::handleSendCommand,
       this,
       boost::asio::placeholders::error,
       boost::asio::placeholders::bytes_transferred,
       cmd));
    // boost::thread run_thread(boost::bind(&Tello::waitForResponse, this));
    if(n_retries_allowed_) boost::thread run_thread(boost::bind(&Tello::retry, this, cmd));
    // while(true){};
}

void Tello::waitForResponse(){
  std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
  while(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start).count() < timeout_){
    if(received_response_){
      // LogInfo() << "[Response] " << response_ ;
      // response_ = "";
      break;
    }
    usleep(100000);
  }
  if(!received_response_) LogInfo() << "Timeout - Attempt #" << n_retries_;
  if(n_retries_ == n_retries_allowed_){
    if(n_retries_allowed_ > 0){
      LogInfo() << "Exhausted retries" ;
    }
    received_response_ = true; // Timeout/
  }
}

void Tello::handleSendCommand(const boost::system::error_code& error,
   size_t bytes_sent, const std::string& cmd)
{
 if(!error && bytes_sent>0){
   LogInfo() << "Successfully sent command [" << cmd << "] to address [" << drone_ip_ << ":" << drone_port_ << "]";
   last_command_ = cmd;
 }
 else{
   LogInfo() << "Failed to send command [" << cmd <<"]";
 }
}

void Tello::retry(const std::string& cmd){
  waitForResponse();
  if(received_response_) return;
  while(n_retries_ < n_retries_allowed_ && !received_response_){
    LogInfo() << "Retrying..." ;
    n_retries_++;
    socket_.async_send_to(
        boost::asio::buffer(cmd, cmd.size()),
        endpoint_,
        boost::bind(&Tello::handleSendCommand,
          this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred,
          cmd));
     waitForResponse();
  }
}

Tello::~Tello(){
 socket_.close();
 io_service_.stop();
}

void Tello::addCommandToQueue(const std::string& cmd){
  command_queue_.push(cmd);
}

void Tello::executeQueue(){
  LogInfo() << "Executing queue commands";
  boost::thread run_thread(&Tello::sendQueueCommands, this);
}

void Tello::sendQueueCommands(){
  while(!command_queue_.empty()){
    sendCommand(command_queue_.front());
    command_queue_.pop();
  }
  LogInfo() << "Command queue empty.";
}
