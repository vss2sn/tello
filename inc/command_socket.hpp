#ifndef COMMANDSOCKET_H
#define COMMANDSOCKET_H

#include <chrono>
#include <queue>

#include "base_socket.hpp"

class CommandSocket : public BaseSocket {
public:

  CommandSocket(boost::asio::io_service& io_service, const std::string& drone_ip, const std::string& drone_port, const std::string& local_port, int n_retries_allowed = 1, int timeout = 7);
  void sendCommand(const std::string& cmd);
  void executeQueue();
  void addCommandToQueue(const std::string& cmd);
  ~CommandSocket();

private:

  void handleResponseFromDrone(const boost::system::error_code& error, size_t bytes_recvd) override;
  void handleSendCommand(const boost::system::error_code& error, size_t bytes_sent, const std::string& cmd) override;
  void waitForResponse();
  void retry(const std::string& cmd);
  void sendQueueCommands();

  enum{ max_length_ = 1024 };
  bool received_response_ = true;
  char data_[max_length_];
  int timeout_, n_retries_ = 0, n_retries_allowed_;
  std::string last_command_, response_;
  std::queue<std::string> command_queue_;

};

#endif COMMANDSOCKET_H
