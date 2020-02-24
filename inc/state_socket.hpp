#ifndef STATESOCKET_HPP
#define STATESOCKET_HPP

#include "base_socket.hpp"

class StateSocket : public BaseSocket{
public:
#ifdef USE_BOOST
  StateSocket(boost::asio::io_service& io_service, const std::string& drone_ip, const std::string& drone_port, const std::string& local_port);
#else
  StateSocket(asio::io_service& io_service, const std::string& drone_ip, const std::string& drone_port, const std::string& local_port);
#endif
  virtual ~StateSocket();

private:
#ifdef USE_BOOST
  virtual void handleResponseFromDrone(const boost::system::error_code& error, size_t bytes_recvd);
  virtual void handleSendCommand(const boost::system::error_code& error, size_t bytes_sent, std::string cmd);
#else
  virtual void handleResponseFromDrone(const std::error_code& error, size_t bytes_recvd);
  virtual void handleSendCommand(const std::error_code& error, size_t bytes_sent, std::string cmd);
#endif

  enum{ max_length_ = 1024 };
  bool received_response_ = true;
  char data_[max_length_];
  std::string response_;

};

#endif STATESOCKET_HPP
