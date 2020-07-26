#include "tello/base_socket.hpp"

BaseSocket::BaseSocket(asio::io_service &io_service,
                       const std::string &drone_ip,
                       const std::string &drone_port,
                       const std::string &local_port)
    : io_service_(io_service), local_port_(local_port), drone_ip_(drone_ip),
      drone_port_(drone_port),
      socket_(io_service_, asio::ip::udp::endpoint(asio::ip::udp::v4(),
                                                   std::stoi(local_port))) {}

BaseSocket::~BaseSocket() { socket_.close(); }
