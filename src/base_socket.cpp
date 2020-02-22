#include "base_socket.hpp"

BaseSocket::BaseSocket(
#ifdef USE_BOOST
  boost::asio::io_service& io_service,
#else
  asio::io_service& io_service,
#endif
  const std::string& drone_ip,
  const std::string& drone_port,
  const std::string& local_port
  )
  :
  io_service_(io_service),
  local_port_(local_port),
  drone_ip_(drone_ip),
  drone_port_(drone_port),
#ifdef USE_BOOST
  socket_(io_service_, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), std::stoi(local_port))) {
#else
  socket_(io_service_, asio::ip::udp::endpoint(asio::ip::udp::v4(), std::stoi(local_port))) {
#endif
}

BaseSocket::~BaseSocket(){
  socket_.close();
}
