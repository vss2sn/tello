#ifndef BASESOCKET_HPP
#define BASESOCKET_HPP

#include <thread>
#include "asio.hpp"

class BaseSocket{
public:

  BaseSocket(asio::io_service& io_service, const std::string& drone_ip, const std::string& drone_port, const std::string& local_port);
  virtual ~BaseSocket();

protected:

  std::string local_port_, drone_ip_, drone_port_;
  asio::io_service& io_service_;
  asio::ip::udp::socket socket_;
  asio::ip::udp::endpoint endpoint_;
  std::thread io_thread;

private:

  virtual void handleResponseFromDrone(const std::error_code& error, size_t bytes_recvd){};
  // NOTE: Passing cmd by ref or as const causes problems dues to async nature of code
  virtual void handleSendCommand(const std::error_code& error, size_t bytes_sent, std::string cmd){};
};

#endif // BASESOCKET_HPP
