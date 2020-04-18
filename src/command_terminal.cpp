#ifdef USE_TERMINAL

#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <string.h>
#include <sys/time.h>
#include <stdio.h>

#include "command_terminal.hpp"

// #include <memory>
// #include <thread>

Terminal::Terminal(bool& on)
:
on_(on)
{
  pt_ = posix_openpt(O_RDWR);
  if(pt_ == -1){
    std::cerr << "Could not open pseudo terminal." << std::endl;
    exit(0);
  }
  ptname_ = ptsname(pt_);
  if(!ptname_){
    std::cerr << "Could not get pseudo terminal device name" << std::endl;
    close(pt_);
    exit(0);
  }
  if(unlockpt(pt_) == -1){
    std::cerr << "Could not get pseudo terminal device name." << std::endl;
    close(pt_);
    exit(0);
  }
  std::ostringstream oss;
  oss << "xterm -S" << (strrchr(ptname_, '/')+1) << "/" << pt_ << " &";
  system(oss.str().c_str());

  xterm_fd_ = open(ptname_,O_RDWR);
  saved_stdout_ = dup(1);
}

void Terminal::terminalWorker(){
  // for(int i=0; i<5; i++){
  //   usleep(1000000);
  //   std::unique_lock<std::mutex> lk(terminal_mutex_);
  //   std::cout << "Lock 1 obtained in runTerminal" << std::endl;
  //   // dup2(xterm_fd_, 0); // Set STDIN to this terminal
  //   dup2(xterm_fd_, 1); // Set STDOUT to this terminal
  //   // dup2(xterm_fd_, 2); // Set STDERR to this terminal
  //   std::cout << "This should appear on the xterm." << std::endl;
  //   dup2(saved_stdout_, 1);
  // }

  dup2(xterm_fd_, 0);
  while(on_){
    s = timedRead();
    if(!s.empty()){
      std::cout << "Received command from terminal: " << s << std::endl;
      {
        std::unique_lock<std::mutex> lk(terminal_mutex_);
        dup2(xterm_fd_, 1);
        printf(">> \n");
        dup2(saved_stdout_, 1);
        received_cmd_ = true;
       }
       usleep(1000);
     }
  }
  dup2(saved_stdout_, 0);
  std::cout << "----------- Terminal worker thread exits -----------" << std::endl;
}

bool Terminal::hasCommnad(){
  return received_cmd_;
}

std::string Terminal::getCommand(){
  received_cmd_ = false;
  return s;
}

Terminal::~Terminal(){
  close(pt_);
  std::cout << "----------- Termial exiting gracefully -----------" << std::endl;
}

std::mutex Terminal::terminal_mutex_;

std::mutex& Terminal::getMutex(){
  return terminal_mutex_;
}

std::string Terminal::timedRead(int timeout_s, int timeout_ms){

   fd_set fdset;
   struct timeval timeout;
   int  rc;
   std::string command;

   timeout.tv_sec = timeout_s;   /* wait for 6 seconds for data */
   timeout.tv_usec = timeout_ms;

   FD_ZERO(&fdset);

   FD_SET(0, &fdset);

   rc = select(1, &fdset, NULL, NULL, &timeout);
   if (rc == -1){
     /* Failed */
     std::cout << "Failed to read command" << std::endl;
   }
   else if (rc == 0){
     /* Timed out */
   }
   else
   {
      if (FD_ISSET(0, &fdset))
      {
         getline(std::cin, command);
      }
   }
   return command;
}

// TODO: replace by unordered_map?
TERMINAL_CMD_TYPE Terminal::convertToEnum(const std::string& cmd_type){
  if(cmd_type == "add"){
    return ADD;
  }
  else if(cmd_type == "start"){
    return START;
  }
  else if(cmd_type == "stop"){
    return STOP;
  }
  else if(cmd_type == "addfront"){
    return ADD_FRONT;
  }
  else if(cmd_type == "clear"){
    return CLEAR;
  }
  else if(cmd_type == "removenext"){
    return REMOVE_NEXT;
  }
  else if(cmd_type == "allowautoland"){
    return ALLOW_AUTO_LAND;
  }
  else if(cmd_type == "donotautoland"){
    return DO_NOT_AUTO_LAND;
  }
  else{
    return UNKNOWN;
  }
}

// int main()
// {
//   bool on = true;
//   std::unique_ptr<Terminal> upt_terminal = std::make_unique<Terminal>(on);
//   auto t = std::thread([&]{upt_terminal->runTerminal();});
//
//   // for(int i=0;i<10;i++){
//     usleep(10000000);
//     // std::cout << "Should be on primary screen>"<< std::endl;
//   // }
//   on = false;
//   t.join();
//
//   return 0;
// }

#endif //TERMINAL
