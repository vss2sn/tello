#include "command_socket.hpp"
#include "video_socket.hpp"
#include "state_socket.hpp"
#include "utils.hpp"
#include "joystick.hpp"
#include "tello.hpp"

int main(){
#ifdef USE_BOOST
  boost::asio::io_service io_service;
#else
  asio::io_service io_service;
#endif

  // Joystick joy;
  Tello t(io_service);

  // CommandSocket c_tello(io_service, "192.168.10.1", "8889", "8889", 0,5);
  t.cs->addCommandToQueue("command");
  t.cs->addCommandToQueue("streamon");
  t.cs->addCommandToQueue("takeoff");
  t.cs->executeQueue();
  t.cs->addCommandToQueue("forward 20");
  t.cs->addCommandToQueue("back 20");
  t.cs->addCommandToQueue("delay 5");
  t.cs->addCommandToFrontOfQueue("stop");
  // usleep(2000000);
  t.cs->stopQueueExecution();
  t.cs->doNotAutoLand();
  t.cs->addCommandToQueue("land");
  usleep(120000000);

  // VideoSocket v_tello(io_service, "0.0.0.0", "11111", "11111");
  //
  // StateSocket s_tello(io_service, "0.0.0.0", "8890", "8890");

  LogWarn() << "-------------------Done--------------------";
  io_service.stop();
  usleep(1000000); // Ensure this is greater than timeout to prevent seg faults
  return 0;
}
