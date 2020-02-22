#ifndef STATESOCKET_H
#define STATESOCKET_H

#include "base_socket.hpp"

class StateSocket : public BaseSocket{
public:

  StateSocket(boost::asio::io_service& io_service, const std::string& drone_ip, const std::string& drone_port, const std::string& local_port);
  virtual ~StateSocket();

private:

  virtual void handleResponseFromDrone(const boost::system::error_code& error, size_t bytes_recvd);
  virtual void handleSendCommand(const boost::system::error_code& error, size_t bytes_sent, const std::string& cmd);

  enum{ max_length_ = 1024 };
  bool received_response_ = true;
  char data_[max_length_];
  std::string response_;

};

#endif
