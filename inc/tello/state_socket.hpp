#ifndef STATESOCKET_HPP
#define STATESOCKET_HPP

#include "tello/base_socket.hpp"

/**
 * @class StateSocket
 * @brief Prints the state of the tello
 */
class StateSocket : public BaseSocket {
public:
  /**
   * @brief Constructor
   * @param [in] io_service io_service object used to handle all socket
   * communication
   * @param [in] drone_ip ip address of drone
   * @param [in] drone_port port number on the drone
   * @param [in] local_port port on the local machine used to communicate with
   * the drone port mentioned above
   * @return none
   */
  StateSocket(asio::io_service &io_service, const std::string &drone_ip,
              const std::string &drone_port, const std::string &local_port);
  /**
   * @brief Destructor
   * @return none
   */
  ~StateSocket();

private:
  virtual void handleResponseFromDrone(const std::error_code &error,
                                       size_t bytes_recvd) override;
  virtual void handleSendCommand(const std::error_code &error,
                                 size_t bytes_sent, std::string cmd) override;

  enum { max_length_ = 1024 };
  bool received_response_ = true;
  char data_[max_length_]{};
  std::string response_;
};

#endif // STATESOCKET_HPP
