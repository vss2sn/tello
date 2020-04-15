#include <mutex>
#include <condition_variable>

#include "utils.hpp"
#include "tello.hpp"

#ifdef USE_TERMINAL
#include "command_terminal.hpp"
#endif

int main(){
  asio::io_service io_service;
  asio::io_service::work work(io_service);

  std::condition_variable cv_run;
  Tello t(io_service, cv_run);

  t.cs->addCommandToQueue("command");
  t.cs->addCommandToQueue("sdk?");
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

  {
    std::mutex mtx;
    std::unique_lock<std::mutex> lck(mtx);
    cv_run.wait_for(lck,std::chrono::seconds(300000000));
  }

  // use cv wait for here

  utils_log::LogWarn() << "----------- Done -----------";
  utils_log::LogWarn() << "Landing.";
  // t.cs->exitAllThreads();
  io_service.stop();
  usleep(1000000); // Ensure this is greater than timeout to prevent seg faults
  utils_log::LogDebug() << "----------- Main thread returns -----------";
  return 0;
}
