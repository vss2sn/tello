#include "tello.hpp"

Tello::Tello(boost::asio::io_service& io_service,
  const std::string& drone_ip,
  const std::string& drone_port,
  const std::string& local_port)
  :
  io_service_(io_service),
  local_port_(local_port),
  drone_ip_(drone_ip),
  drone_port_(drone_port),
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
     std::cout << "Alert! Unknown response: " << data_ << std::endl;
   }
   std::cout << "Received [response] " << response_ << " after sending [command] "<< last_command_ << " from [address] " << drone_ip_ << ":" << drone_port_ << std::endl;
 }
 else{
   std::cout << "Nothing received" << std::endl;
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
 socket_.async_send_to(
     boost::asio::buffer(cmd, cmd.size()),
     endpoint_,
     boost::bind(&Tello::handleSendCommand,
       this,
       boost::asio::placeholders::error,
       boost::asio::placeholders::bytes_transferred,
       cmd));
  std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
  while(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start).count() < timeout_){
    if(!response_.empty()){
      // std::cout << "[Response] " << response_ << std::endl;
      response_ = "";
      break;
    }
    usleep(1000000);
  }
}

void Tello::handleSendCommand(const boost::system::error_code& error,
   size_t bytes_sent, const std::string& cmd)
{
 if(!error && bytes_sent>0){
   std::cout << "Successfully sent [command] " << cmd << " to [address] " << drone_ip_ << ":" << drone_port_ << std::endl;
   last_command_ = cmd;
 }
 else{
   std::cout << "Failed to send [command] " << cmd << std::endl;
 }
}

Tello::~Tello(){
 socket_.close();
 io_service_.stop();
}
