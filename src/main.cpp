#include "tello.hpp"

int main(){
  boost::asio::io_service io_service;
  Tello tello(io_service, "192.168.10.1", "8889", "8889");
  tello.sendCommand("command");
  usleep(3000000);
  tello.sendCommand("takeoff");
  usleep(9000000);
  tello.sendCommand("land");
  usleep(3000000);
  io_service.stop();
  return 0;
}
