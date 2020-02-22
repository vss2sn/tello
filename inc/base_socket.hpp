#ifndef BASESOCKET_H
#define BASESOCKET_H

#ifdef USE_BOOST
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#else
#include <thread>
#include "asio.hpp"
#endif

class BaseSocket{
public:

#ifdef USE_BOOST
  BaseSocket(boost::asio::io_service& io_service, const std::string& drone_ip, const std::string& drone_port, const std::string& local_port);
#else
  BaseSocket(asio::io_service& io_service, const std::string& drone_ip, const std::string& drone_port, const std::string& local_port);
#endif
  virtual ~BaseSocket();

protected:

  std::string drone_ip_, drone_port_, local_port_;
#ifdef USE_BOOST
  boost::asio::io_service& io_service_;
  boost::asio::ip::udp::socket socket_;
  boost::asio::ip::udp::endpoint endpoint_;
  boost::thread io_thread;
#else
  asio::io_service& io_service_;
  asio::ip::udp::socket socket_;
  asio::ip::udp::endpoint endpoint_;
  std::thread io_thread;
#endif

private:

#ifdef USE_BOOST
  virtual void handleResponseFromDrone(const boost::system::error_code& error, size_t bytes_recvd){};
  // NOTE: Passing cmd by ref or as const causes problems dues to async nature of code
  virtual void handleSendCommand(const boost::system::error_code& error, size_t bytes_sent, std::string cmd){};
#else
  virtual void handleResponseFromDrone(const std::error_code& error, size_t bytes_recvd){};
  // NOTE: Passing cmd by ref or as const causes problems dues to async nature of code
  virtual void handleSendCommand(const std::error_code& error, size_t bytes_sent, std::string cmd){};
#endif
};

#endif
