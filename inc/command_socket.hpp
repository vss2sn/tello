#ifndef COMMANDSOCKET_H
#define COMMANDSOCKET_H

#include <chrono>
#include <deque>
#include <mutex>
#include <thread>
#include <condition_variable>
#include "base_socket.hpp"

class CommandSocket : public BaseSocket {
public:

#ifdef USE_BOOST
  CommandSocket(boost::asio::io_service& io_service, const std::string& drone_ip, const std::string& drone_port, const std::string& local_port, int n_retries_allowed = 1, int timeout = 7);
#else
  CommandSocket(asio::io_service& io_service, const std::string& drone_ip, const std::string& drone_port, const std::string& local_port, int n_retries_allowed = 1, int timeout = 7);
#endif
  void executeQueue();
  void addCommandToQueue(const std::string& cmd);
  void addCommandToFrontOfQueue(const std::string& cmd);
  void clearQueue();
  void stopQueueExecution();
  void removeNextFromQueue();
  void DoNotLand();
  ~CommandSocket();

private:

#ifdef USE_BOOST
  void handleResponseFromDrone(const boost::system::error_code& error, size_t bytes_recvd) override;
  void handleSendCommand(const boost::system::error_code& error, size_t bytes_sent, std::string cmd) override;
#else
  void handleResponseFromDrone(const std::error_code& error, size_t bytes_recvd) override;
  void handleSendCommand(const std::error_code& error, size_t bytes_sent, std::string cmd) override;
#endif

  void waitForResponse();
  void retry(const std::string& cmd);
  void sendQueueCommands();
  void sendCommand(std::string cmd);

  enum{ max_length_ = 1024 };
  bool waiting_for_response_ = false;
  bool execute_queue_ = false;
  char data_[max_length_];
  int timeout_, n_retries_ = 0, n_retries_allowed_;
  std::string last_command_, response_;
  std::deque<std::string> command_queue_;
  std::mutex queue_mutex_;
  std::chrono::system_clock::time_point command_sent_time;
  bool do_not_land = true;
  bool on_ = true;
  int do_not_land_timeout = 10;
#ifdef USE_BOOST
  boost::thread cmd_thread;
#else
  std::thread cmd_thread;
#endif

  std::mutex m;
  std::condition_variable cv;
};

#endif COMMANDSOCKET_H
