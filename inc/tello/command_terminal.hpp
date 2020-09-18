#ifdef USE_TERMINAL

#ifndef TERMINAL_HPP
#define TERMINAL_HPP

#include <atomic>
#include <mutex>

enum TERMINAL_CMD_TYPE {
  ADD,
  START,
  STOP,
  ADD_FRONT,
  CLEAR,
  REMOVE_NEXT,
  ALLOW_AUTO_LAND,
  DO_NOT_AUTO_LAND,
  UNKNOWN
};

/**
 * @class Terminal
 * @brief Creates an Xterm that sets STDIN to itself and accepts command line
 * input
 */
class Terminal {
public:

  /**
   * @brief Constructor
   * @param [in] run refernce to a bool variable that is set to false whn the
   * code is exiting
   * @return none
   */
  Terminal(bool &run, std::mutex &display_mutex);

  // TODO: add a constructor without a ref that sets the internal bool to true
  // and a destrcutor that sets it to false

  /**
   * @brief gets the latest command from the teminal (CLI)
   * @return the command from the terminal (CLI)
   */
  std::string getCommand();

  /**
   * @brief checks whether there is a command input from the terminal (CLI)
   * @return bool whether there is a command to be executed from the termonal
   * (CLI)
   */
  bool hasCommnad();

  /**
   * @brief terminal worker finction that constantly runs and reads CLI input
   * when it exists.
   * @return void
   */
  void terminalWorker();

  /**
   * @brief Converts the string command into an enum
   * @param [in] cmd_type
   * @return strong typed enum of the string command type
   */
  TERMINAL_CMD_TYPE convertToEnum(const std::string &cmd_type);
  /**
   * @brief Destructor
   * @return none
   */
  ~Terminal();

  /**
   * @brief Get the mutex used by the terminal for controlling input/output
   * @return reference to the mutex used for display 
   */
  std::mutex &getMutex();

private:
  std::string timedRead(int timeout_s = 1, int timeout_ms = 0);
  bool &run_;
  int pt_, xterm_fd_, saved_stdout_;
  char *ptname_;
  std::string s;
  std::atomic<bool> received_cmd_;
  std::mutex& terminal_mutex_;
};

#endif // TERMINAL_HPP
#endif // TERMINAL
