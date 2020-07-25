#include "tello/state_socket.hpp"
#include "utils/utils.hpp"

StateSocket::StateSocket(
  asio::io_service& io_service,
  const std::string& drone_ip,
  const std::string& drone_port,
  const std::string& local_port
):
  BaseSocket(io_service, drone_ip, drone_port, local_port)
{
  asio::ip::udp::resolver resolver(io_service_);
  asio::ip::udp::resolver::query query(asio::ip::udp::v4(), drone_ip_, drone_port_);
  asio::ip::udp::resolver::iterator iter = resolver.resolve(query);
  endpoint_ = *iter;

  socket_.async_receive_from(
    asio::buffer(data_, max_length_),
    endpoint_,
    [&](const std::error_code& error, size_t bytes_recvd)
    {return handleResponseFromDrone(error, bytes_recvd);});
    // [&](auto... args){return handleResponseFromDrone(args...);});

    io_thread = std::thread([&]{io_service_.run();});
    io_thread.detach();

}

void StateSocket::handleResponseFromDrone(const std::error_code& error, size_t bytes_recvd)
{
  if(!error && bytes_recvd>0){
    response_ = std::string(data_);
    std::replace(response_.begin(), response_.end(), ';', '\n');
    // std::cout << "Status: \n" << response_ << std::endl;
  }
  else{
    // utils_log::LogDebug() << "Error/Nothing received" ;
  }

  socket_.async_receive_from(
    asio::buffer(data_, max_length_),
    endpoint_,
    [&](const std::error_code& error, size_t bytes_recvd)
    {return handleResponseFromDrone(error, bytes_recvd);});
    // [&](auto... args){return handleResponseFromDrone(args...);});
}

StateSocket::~StateSocket(){
  socket_.close();
}

void StateSocket::handleSendCommand(const std::error_code& error, size_t bytes_sent, std::string cmd)
{
  std::cout << "StateSocket class does not implement handleSendCommand()" << std::endl;
}
