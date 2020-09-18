#ifndef TELLO_HPP
#define TELLO_HPP

#include <memory>

#include "joystick/joystick.hpp"
#include "tello/command_socket.hpp"
#include "tello/state_socket.hpp"
#include "tello/video_socket.hpp"

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
class Tello {
public:
  /**
   * @brief Constructor
   * @param [in] drone_ip ip address of drone
   * @param [in] local_drone_port local port through which commands will be sent
   * to the drone
   * @param [in] local_video_port local port that will receive the drone video
   * stream
   * @param [in] local_state_port local port which will receive state
   * information from the drone
   * @param [in] camera_config_file
   * @param [in] vocabulary_file
   * @param [in] n_retries number of retries for command if no response received
   * @param [in] timeout timeout before a command is said to have failed and is
   * resent if retries are enabled
   * @param [in] load_map_db_path path and file name from which the map must be
   * loaded
   * @param [in] save_map_db_path path and file name to which the map must be
   * saved
   * @param [in] mask_img_path path to pattern mask input images
   * @param [in] load_map bool for whether the map should be loaded
   * @param [in] continue_mapping continue adding to the map even when a map has
   * been loaded
   * @param [in] scale scale for SLAM
   * @param [in] sequence_file file containing a sequence of commands that will
   * be added to execute queue
   * @param [in] station_mode whether the drone is in station mode or not
   * @return none
   */
  Tello(const std::string &drone_ip = "192.168.10.1",
        const std::string &local_drone_port = "8889",
        const std::string &local_video_port = "11111",
        const std::string &local_state_port = "8890",
        const std::string &camera_config_file = "../camera_config.yaml",
        const std::string &vocabulary_file = "../orb_vocab.dbow2",
        const int n_retries = 0, const int timeout = 5,
        const std::string &load_map_db_path = "",
        const std::string &save_map_db_path = "",
        const std::string &mask_img_path = "", bool load_map = false,
        bool continue_mapping = false, float scale = 1.0,
        const std::string &sequence_file = "",
        bool station_mode = false  // NOTE(vss): make const?
      );

  /**
   * @brief reads a sequence of commands from a file and adds them to the
   * execution queue
   * @param [in] file name of the file containing the sequence of commands
   * @return void
   */
  void readSequence(const std::string &file) const;

  /**
   * @brief Destructor
   * @return none
   */
  ~Tello();

  /**
   * @brief Check whether the drone is active
   * @return whether the rone is active
   */
  bool active() const;

  /**
   * @brief Command interface to call some command socket functions
   * @param [in] cmd_fx function to call
   * @param [in] cmd argument passed to the command function
   * @return none
   */
  void commandInterface(const std::string& cmd_fx, const std::string& cmd = "");

private:
  void jsToCommandThread();
  void jsToCommand(ButtonId update);
  void jsToCommand(AxisId update) const;

  bool run_ = true;
  bool station_mode_ = false;
  // NOTE: io_service_ needs to be destroyed after the sockets, not before
  asio::io_service io_service_;
  std::thread js_thread_;
  std::unique_ptr<Joystick> js_;
  std::unique_ptr<CommandSocket> cs;
  std::unique_ptr<VideoSocket> vs;
  std::unique_ptr<StateSocket> ss;

#ifdef USE_TERMINAL
  void terminalToCommandThread();
  void terminalToCommand(const std::string &cmd);
  std::thread term_thread_worker_, term_thread_fetch_;
  std::unique_ptr<Terminal> term_;
#endif // TERMINAL
};

#endif // TELLO_HPP
