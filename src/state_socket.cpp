#include "state_socket.hpp"
#include "utils.hpp"

StateSocket::StateSocket(boost::asio::io_service& io_service,
  const std::string& drone_ip,
  const std::string& drone_port,
  const std::string& local_port
):
  BaseSocket(io_service, drone_ip, drone_port, local_port)
{
    boost::asio::ip::udp::resolver resolver(io_service_);
    boost::asio::ip::udp::resolver::query query(boost::asio::ip::udp::v4(), drone_ip_, drone_port_);
    boost::asio::ip::udp::resolver::iterator iter = resolver.resolve(query);
    endpoint_ = *iter;

    socket_.async_receive_from(
      boost::asio::buffer(data_, max_length_),
      endpoint_,
      boost::bind(&StateSocket::handleResponseFromDrone,
                  this,
                  boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));

    boost::thread run_thread(boost::bind(&boost::asio::io_service::run, boost::ref(io_service_)));
}

void StateSocket::handleResponseFromDrone(const boost::system::error_code& error, size_t bytes_recvd)
{
  if(!error && bytes_recvd>0){
    response_ = std::string(data_);
    std::replace(response_.begin(), response_.end(), ';', '\n');
  }
  // std::cout << "Status: \n" << response_ << std::endl;
  else{
    LogInfo() << "Error/Nothing received" ;
  }

  socket_.async_receive_from(
    boost::asio::buffer(data_, max_length_),
    endpoint_,
    boost::bind(&StateSocket::handleResponseFromDrone,
                this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
}

StateSocket::~StateSocket(){
  socket_.close();
}

void StateSocket::handleSendCommand(const boost::system::error_code& error,
   size_t bytes_sent, const std::string& cmd)
{
  std::cout << "StateSocket class does not implement handleSendCommand()" << std::endl;
}
