#ifndef COMMANDSOCKET_H
#define COMMANDSOCKET_H

#include <chrono>
#include <deque>

#include "base_socket.hpp"

class CommandSocket : public BaseSocket {
public:

  CommandSocket(boost::asio::io_service& io_service, const std::string& drone_ip, const std::string& drone_port, const std::string& local_port, int n_retries_allowed = 1, int timeout = 7);
  void executeQueue();
  void addCommandToQueue(const std::string& cmd);
  void addCommandToFrontOfQueue(const std::string& cmd);
  void clearQueue();
  void stopQueueExecution();
  ~CommandSocket();

private:

  void handleResponseFromDrone(const boost::system::error_code& error, size_t bytes_recvd) override;
  void handleSendCommand(const boost::system::error_code& error, size_t bytes_sent, const std::string& cmd) override;
  void waitForResponse();
  void retry(const std::string& cmd);
  void sendQueueCommands();
  void sendCommand(const std::string& cmd);

  enum{ max_length_ = 1024 };
  bool received_response_ = true;
  bool execute_queue_ = false;
  char data_[max_length_];
  int timeout_, n_retries_ = 0, n_retries_allowed_;
  std::string last_command_, response_;
  std::deque<std::string> command_queue_;

};

#endif COMMANDSOCKET_H
