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

  Tello t(io_service);

  t.cs->addCommandToQueue("command");
  t.cs->addCommandToQueue("streamon");
  t.cs->addCommandToQueue("takeoff");
  t.cs->executeQueue();
  t.cs->addCommandToQueue("forward 20");
  t.cs->addCommandToQueue("back 20");
  t.cs->addCommandToQueue("delay 5");
  t.cs->addCommandToFrontOfQueue("stop");
  // t.cs->stopQueueExecution();
  t.cs->doNotAutoLand();
  t.cs->addCommandToQueue("land");
  usleep(300000000); // 5 minute timeout

  LogWarn() << "-------------------Done--------------------";
  LogWarn() << "Landing.";
  io_service.stop();
  usleep(1000000); // Ensure this is greater than timeout to prevent seg faults
  return 0;
}
