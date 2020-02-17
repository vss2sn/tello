#include <iostream>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <chrono>

class Tello{
public:

  Tello(boost::asio::io_service& io_service, const std::string& drone_ip, const std::string& drone_port, const std::string& local_port);
  void sendCommand(const std::string& cmd);
  ~Tello();

private:

  void handleResponseFromDrone(const boost::system::error_code& error, size_t bytes_recvd);
  void handleSendCommand(const boost::system::error_code& error, size_t bytes_sent, const std::string& cmd);

  enum{ max_length_ = 1024 };
  char data_[max_length_];
  int timeout_ = 15;
  std::string last_command_, response_;
  std::string drone_ip_, drone_port_, local_port_;
  boost::asio::io_service& io_service_;
  boost::asio::ip::udp::socket socket_;
  boost::asio::ip::udp::endpoint endpoint_;


};
