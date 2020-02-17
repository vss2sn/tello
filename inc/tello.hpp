#include <iostream>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <chrono>
#include <queue>

class Tello{
public:

  Tello(boost::asio::io_service& io_service, const std::string& drone_ip, const std::string& drone_port, const std::string& local_port, int n_retries_allowed = 1, int timeout = 7);
  void sendCommand(const std::string& cmd);
  void executeQueue();
  void addCommandToQueue(const std::string& cmd);
  ~Tello();

private:

  void handleResponseFromDrone(const boost::system::error_code& error, size_t bytes_recvd);
  void handleSendCommand(const boost::system::error_code& error, size_t bytes_sent, const std::string& cmd);
  void waitForResponse();
  void retry(const std::string& cmd);
  void sendQueueCommands(); 

  enum{ max_length_ = 1024 };
  bool received_response_ = true;
  char data_[max_length_];
  int timeout_, n_retries_ = 0, n_retries_allowed_;
  std::string last_command_, response_;
  std::string drone_ip_, drone_port_, local_port_;
  boost::asio::io_service& io_service_;
  boost::asio::ip::udp::socket socket_;
  boost::asio::ip::udp::endpoint endpoint_;
  std::queue<std::string> command_queue_;

};
