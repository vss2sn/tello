#ifndef COMMANDSOCKET_HPP
#define COMMANDSOCKET_HPP

#include <chrono>
#include <deque>
#include <mutex>
#include <thread>
#include <condition_variable>

#include "tello/base_socket.hpp"
#include "joystick/joystick.hpp"

/**
* @class CommandSocket
* @brief Socket class that handles the communication of commands to the tello
*/
class CommandSocket : public BaseSocket {
public:
  /**
  * @brief Constructor
  * @param [in] io_service io_service object used to handle all socket communication
  * @param [in] drone_ip ip address of drone
  * @param [in] drone_port port number on the drone
  * @param [in] local_port port on the local machine used to communicate with the drone port mentioned above
  * @param [in] n_retries_allowed numebr of retries allowed if a response is not received from the drone before sending the next command in the execution queue
  * @param [in] timeout number of seconds after which a command is said to have failed to be sent
  * @return none
  */
  CommandSocket(asio::io_service& io_service, const std::string& drone_ip, const std::string& drone_port, const std::string& local_port, int n_retries_allowed = 1, int timeout = 7);

  /**
  * @brief Starts execution of the command queue
  * @return void
  */
  void executeQueue();

  /**
  * @brief Adds the command to the execution queue
  * @param [in] cmd command to be added to the end of the execution queue
  * @return void
  */
  void addCommandToQueue(const std::string& cmd);

  /**
  * @brief Adds the command to the front of the execution queue
  * @param [in] cmd command to be added to the end of the execution queue
  * @return void
  */
  void addCommandToFrontOfQueue(const std::string& cmd);

  /**
  * @brief clears the execution queue
  * @return void
  */
  void clearQueue();

  /**
  * @brief stop the queue execution
  * @return void
  */
  void stopQueueExecution();

  /**
  * @brief remove the command at the front of the execution queue
  * @return void
  */
  void removeNextFromQueue();

  /**
  * @brief will set the bool value that prevents automatic landing due to no commands sent timeout
  * @return void
  */
  void doNotAutoLand();

  /**
  * @brief will unset the bool value that prevents automatic landing due to no commands sent timeout
  * @return void
  */
  void allowAutoLand();

  /**
  * @brief sends the "emergency" command to the drone that will cause the motors to stop immediately and stops queue exection as well
  * @return void
  */
  void emergency();

  /**
  * @brief sends the "stop" command to the drone ad stops queue execution
  * @return void
  */
  void stop();

  /**
  * @brief queries whether queue execution is enabled.
  * @return bool whether queue execution is enabled
  */
  bool isExecutingQueue();

  /**
  * @brief Enables autoland and sends the command "land" to the drone.
  * @return void
  */
  void land();

  /**
  * @brief Destructor
  * @return none
  */
  ~CommandSocket();

private:

  void handleResponseFromDrone(const std::error_code& error, size_t bytes_recvd) override;
  void handleSendCommand(const std::error_code& error, size_t bytes_sent, std::string cmd) override;

  void waitForResponse();
  void retry(const std::string& cmd);
  void sendQueueCommands();
  void sendCommand(const std::string& cmd);
  void doNotAutoLandWorker();

  enum{ max_length_ = 1024 };
  bool waiting_for_response_ = false, execute_queue_ = false, dnal_ = false, on_ = true;
  char data_[max_length_];
  int timeout_, n_retries_ = 0, n_retries_allowed_ = 0, dnal_timeout = 7 /*dnal --> do not auto land*/ ;
  std::string last_command_, response_;
  std::deque<std::string> command_queue_;
  std::mutex queue_mutex_, m, dnal_mutex;
  std::condition_variable cv_execute_queue_, cv_dnal_;
  std::chrono::system_clock::time_point command_sent_time_;

  std::thread cmd_thread, dnal_thread;

  friend class Tello;
};

#endif // COMMANDSOCKET_HPP
