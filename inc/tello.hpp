#ifndef TELLO_HPP
#define TELLO_HPP

#include  <memory>

#include "command_socket.hpp"
#include "video_socket.hpp"
#include "state_socket.hpp"
#include "joystick.hpp"
#include "utils.hpp"

#ifdef USE_TERMINAL
#include "command_terminal.hpp"
#endif

class Tello{
public:
  /**
  * @brief Constructor
  * @param [in] io_service io_service object used to handle all socket communication
  * @param [in] cv_run condition variable for the lifetime of the code
  * @param [in] drone_ip ip address of drone
  * @param [in] local_drone_port local port through which commands will be sent to the drone
  * @param [in] local_video_port local port that will receive the drone video stream
  * @param [in] local_state_port local port which will receive state information from the drone
  * @param [in] camera_config_file
  * @param [in] vocabulary_file
  * @return none
  */
  Tello(asio::io_service& io_service,
        std::condition_variable& cv_run,
        const std::string drone_ip = "192.168.10.1",
        const std::string local_drone_port = "8889",
        const std::string local_video_port = "11111",
        const std::string local_state_port = "8890",
        const std::string camera_config_file = "../camera_config.yaml",
        const std::string vocabulary_file = "../orb_vocab.dbow2"
      );

  /**
  * @brief Destructor
  * @return none
  */
  ~Tello();

  // TODO: Move to private
  std::unique_ptr<CommandSocket> cs;
  std::unique_ptr<Joystick> js_;
  std::unique_ptr<VideoSocket> vs;
  std::unique_ptr<StateSocket> ss;

private:

  asio::io_service& io_service_;
  std::thread js_thread_;
  std::condition_variable& cv_run_;
  void jsToCommandThread();
  void jsToCommand(ButtonId update);
  void jsToCommand(AxisId update);
  bool run_ = true;

#ifdef USE_TERMINAL
  std::unique_ptr<Terminal> term_;
  std::thread term_thread_worker_, term_thread_fetch_;
  void terminalToCommandThread();
#endif // TERMINAL
};

#endif // TELLO_HPP
