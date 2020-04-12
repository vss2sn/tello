#include <mutex>
#include <condition_variable>

#include "utils.hpp"
#include "tello.hpp"

#ifdef USE_TERMINAL
#include "command_terminal.hpp"
#endif

#ifdef USE_CONFIG
#include "config_handler.hpp"
#endif

int main(){
  asio::io_service io_service;
  asio::io_service::work work(io_service);

  std::condition_variable cv_run;

#ifdef USE_CONFIG

  std::map<std::string, std::unique_ptr<Tello>>  m = handleConfig("../config.yaml", io_service, cv_run);

  if(m.count("0.prime.0") > 0){
    Tello& t = *m["0.prime.0"];
    t.cs->addCommandToQueue("command");
    t.cs->addCommandToQueue("sdk?");
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
  }
  else{
    utils_log::LogErr() << "The requested drone does not exist.";
  }

#else

  Tello t(io_service, cv_run, "192.168.10.1", "8889", "11111", "8890", "../orb_vocab.dbow2", "../camera_config.yaml");

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

#endif

  {
    std::mutex mtx;
    std::unique_lock<std::mutex> lck(mtx);
    cv_run.wait_for(lck,std::chrono::seconds(300));
  }

  utils_log::LogWarn() << "----------- Done -----------";
  utils_log::LogWarn() << "----------- Landing -----------";
  // t.cs->exitAllThreads();
  io_service.stop();
  usleep(1000000); // Ensure this is greater than timeout to prevent seg faults
  utils_log::LogDebug() << "----------- Main thread returns -----------";
  return 0;
}
