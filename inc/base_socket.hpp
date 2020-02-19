#ifndef BASESOCKET_H
#define BASESOCKET_H

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

class BaseSocket{
public:

  BaseSocket(boost::asio::io_service& io_service, const std::string& drone_ip, const std::string& drone_port, const std::string& local_port);
  virtual ~BaseSocket();

protected:

  std::string drone_ip_, drone_port_, local_port_;
  boost::asio::io_service& io_service_;
  boost::asio::ip::udp::socket socket_;
  boost::asio::ip::udp::endpoint endpoint_;

private:

  virtual void handleResponseFromDrone(const boost::system::error_code& error, size_t bytes_recvd){};
  virtual void handleSendCommand(const boost::system::error_code& error, size_t bytes_sent, const std::string& cmd){};

};

#endif
