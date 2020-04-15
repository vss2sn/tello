#ifdef USE_TERMINAL

#ifndef TERMINAL_HPP
#define TERMINAL_HPP

#include <mutex>
#include <atomic>

std::string timedRead(int timeout_s = 1);

class Terminal{

public:

  static std::mutex terminal_mutex_;
  static std::mutex& getMutex();
  void terminalWorker();
  std::string getCommand();
  bool hasCommnad();
  Terminal(bool& on);
  void runTerminal();
  ~Terminal();

private:

  bool& on_;
  int pt_, xterm_fd_, saved_stdout_;
  char * ptname_;
  std::string s;
  std::atomic<bool> received_cmd_;

};

#endif // TERMINAL_HPP
#endif // TERMINAL
