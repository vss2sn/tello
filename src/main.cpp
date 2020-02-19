#include "command_socket.hpp"
#include "video_socket.hpp"

int main(){
  boost::asio::io_service io_service;
  CommandSocket c_tello(io_service, "192.168.10.1", "8889", "8889", 3,5);
  c_tello.addCommandToQueue("command");
  c_tello.addCommandToQueue("streamon");

  // tello.addCommandToQueue("takeoff");
  // tello.addCommandToQueue("land");
  c_tello.executeQueue();

  VideoSocket v_tello(io_service, "0.0.0.0", "11111", "11111");
  usleep(300000000); // Ensure this is greater than timeout to prevent seg faults
  io_service.stop();
  return 0;
}
