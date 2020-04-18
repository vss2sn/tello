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

/**
* @class Tello
* @brief Class defining the drone, associated sockets, and input methods
* @details In addition to defining instances of the sockets and input methods,
this class also acts as a single point of communication for any and all command
input and output, and encapsulates a tello
*/
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
  * @param [in] n_retries number of retries for command if no response received
  * @param [in] timeout timeout before a command is said to have failed and is resent if retries are enabled
  * @param [in] load_map_db_path path and file name from which the map must be loaded
  * @param [in] save_map_db_path path and file name to which the map must be saved
  * @param [in] mask_img_path path to pattern mask input images
  * @param [in] load_map bool for whether the map should be loaded
  * @param [in] continue_mapping continue adding to the map even when a map has been loaded
  * @param [in] scale scale for SLAM
  * @return none
  */
  Tello(asio::io_service& io_service,
        std::condition_variable& cv_run,
        const std::string drone_ip = "192.168.10.1",
        const std::string local_drone_port = "8889",
        const std::string local_video_port = "11111",
        const std::string local_state_port = "8890",
        const std::string camera_config_file = "../camera_config.yaml",
        const std::string vocabulary_file = "../orb_vocab.dbow2",
        const int n_retries = 0,
        const int timeout = 5,
        const std::string load_map_db_path = "",
        const std::string save_map_db_path = "",
        const std::string mask_img_path = "",
        bool load_map = false,
        bool continue_mapping = false,
        float scale = 1.0
      );

  /**
  * @brief Destructor
  * @return none
  */
  ~Tello();

  // TODO: Move to private

  /** \brief Unique pointer to CommandSocket object associated with this tello */
  std::unique_ptr<CommandSocket> cs;

  /** \brief Unique pointer to Joystick object associated with this tello */
  std::unique_ptr<Joystick> js_;

  /** \brief Unique pointer to VideoSocket associated with this tello */
  std::unique_ptr<VideoSocket> vs;

  /** \brief Unique pointer to StateSocket associated with this tello */
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
  void terminalToCommand(const std::string& cmd);
#endif // TERMINAL
};

#endif // TELLO_HPP
