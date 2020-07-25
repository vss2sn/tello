#ifndef BASESOCKET_HPP
#define BASESOCKET_HPP

#include <thread>
#include "asio.hpp"

/**
* @class BaseSocket
* @brief Abstract class inhereted by the other socket classes
*/
class BaseSocket{
public:
  /**
  * @brief Constructor of base socket
  * @param [in] io_service io_service object used to handle all socket communication
  * @param [in] drone_ip ip address of drone
  * @param [in] drone_port port number on the drone
  * @param [in] local_port port on the local machine used to communicate with the drone port mentioned above
  * @return no return
  */
  BaseSocket(asio::io_service& io_service, const std::string& drone_ip, const std::string& drone_port, const std::string& local_port);

  virtual ~BaseSocket();

protected:

  std::string local_port_, drone_ip_, drone_port_;
  asio::io_service& io_service_;
  asio::ip::udp::socket socket_;
  asio::ip::udp::endpoint endpoint_;
  std::thread io_thread;

private:
  /**
  * @brief function called when response received from drone
  * @param [in] error error thrown by socket when receiving a response from drone
  * @param [in] bytes_recvd number of bytes received
  * @return void
  * @details Pure virtual function overridden in implementation classes
  */
  virtual void handleResponseFromDrone(const std::error_code& error, size_t bytes_recvd) = 0;

  // NOTE: Passing cmd by ref or as const causes problems dues to async nature of code

  /**
  * @brief function called to send command to drone
  * @param [in] error error thrown by socket when sending a command to the drone
  * @param [in] bytes_sent number of bytes sent
  * @param [in] cmd command to be sent to drone
  * @return void
  * @details Pure virtual function overridden in implementation classes
  */
  virtual void handleSendCommand(const std::error_code& error, size_t bytes_sent, std::string cmd) = 0;
};

#endif // BASESOCKET_HPP
