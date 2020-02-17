#include "tello.hpp"

int main(){
  boost::asio::io_service io_service;
  Tello tello(io_service, "192.168.10.1", "8889", "8889", 3,5);
  tello.sendCommand("command");
  tello.sendCommand("takeoff");
  tello.sendCommand("land");
  usleep(20000000); // Ensure this is greater than timeout to prevent seg faults
  io_service.stop();
  return 0;
}
