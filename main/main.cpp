#include <condition_variable>
#include <mutex>

#include "tello/tello.hpp"
#include "utils/utils.hpp"

#ifdef USE_TERMINAL
#include "tello/command_terminal.hpp"
#endif

#ifdef USE_CONFIG
#include "tello/config_handler.hpp"
#endif

namespace constants {
constexpr auto max_flight_time = std::chrono::minutes(5);
}  // namespace constants

int main(){

// NOTE: This has been modified to consistently test the code with and without the config handler.

#ifdef USE_CONFIG
  std::map<std::string, std::unique_ptr<Tello>>  m = handleConfig("../config.yaml");
  if(m.count("0.prime.0") == 0){
    utils_log::LogErr() << "The requested drone does not exist.";
    return;
  }
  Tello* t = m["0.prime.0"].get();
#else
  std::unique_ptr<Tello> tello = std::make_unique<Tello>("192.168.10.1", "8889", "11111", "8890", "../camera_config.yaml", "../orb_vocab.dbow2");
  Tello* t = tello.get();
#endif

  t->commandInterface("add_command_to_queue", "command");
  t->commandInterface("add_command_to_queue", "sdk?");
  t->commandInterface("add_command_to_queue", "command");
  t->commandInterface("add_command_to_queue", "sdk?");
  t->commandInterface("add_command_to_queue", "streamon");
  t->commandInterface("add_command_to_queue", "takeoff");
  t->commandInterface("execute_queue");
  t->commandInterface("add_command_to_queue", "forward 20");
  t->commandInterface("add_command_to_queue", "back 20");
  t->commandInterface("add_command_to_queue", "delay 5");
  t->commandInterface("add_command_to_front_of_queue", "stop");
  t->commandInterface("stop_queue_execution");
  t->commandInterface("do_not_auto_land");
  t->commandInterface("add_command_to_queue", "land");

  while(t->active()) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  utils_log::LogWarn() << "----------- Done -----------";
  utils_log::LogDebug() << "----------- Main thread returns -----------";
  return 0;
}
