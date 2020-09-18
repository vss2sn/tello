#include "tello/tello.hpp"

#include <fstream>

#include "utils/utils.hpp"

namespace constants {
constexpr auto joystick_timeout = std::chrono::milliseconds(1);
constexpr auto wait_for_land = std::chrono::seconds(5);
}  // namespace constants

Tello::Tello(const std::string &drone_ip, const std::string &local_drone_port,
             const std::string &local_video_port,
             const std::string &local_state_port,
             const std::string &camera_config_file,
             const std::string &vocabulary_file, const int n_retries,
             const int timeout, const std::string &load_map_db_path,
             const std::string &save_map_db_path,
             const std::string &mask_img_path, bool load_map,
             bool continue_mapping, float scale,
             const std::string &sequence_file, bool station_mode)
    : station_mode_{station_mode} {
  cs = std::make_unique<CommandSocket>(io_service_, drone_ip, "8889",
                                       local_drone_port, n_retries, timeout);
  if (!station_mode_) {
    vs = std::make_unique<VideoSocket>(
        io_service_, "0.0.0.0", "11111", local_video_port, run_,
        camera_config_file, vocabulary_file, load_map_db_path, save_map_db_path,
        mask_img_path, load_map, continue_mapping, scale);
  }
  ss = std::make_unique<StateSocket>(io_service_, "0.0.0.0", "8890",
                                     local_state_port);

#ifdef USE_JOYSTICK
  js_ = std::make_unique<Joystick>();
  js_thread_ = std::thread([&, this] { this->jsToCommandThread(); });
  // js_thread_ = std::thread(&Tello::jsToCommandThread, this);
#endif  // USE_JOYSTICK

#ifdef USE_TERMINAL
  term_ = std::make_unique<Terminal>(run_);
  // term_thread_worker_ = std::thread(&Tello::terminalWorker, this);
  // term_thread_fetch_ = std::thread(&Tello::terminalToCommandThread, this);
  term_thread_worker_ =
      std::thread([&, this] { this->term_->terminalWorker(); });
  term_thread_fetch_ =
      std::thread([&, this] { this->terminalToCommandThread(); });
#endif  // TERMINAL

  // If using joystick, joystick should be initailized before adding to command
  // queue; safety.
  readSequence(sequence_file);
}

#ifdef USE_TERMINAL
void Tello::terminalToCommandThread() {
  while (run_) {
    std::this_thread::sleep_for(joystick_timeout);
    if (term_->hasCommnad()) {
      terminalToCommand(term_->getCommand());
    }
  }
  utils_log::LogDebug()
      << "----------- Terminal to command thread exits -----------";
}

void Tello::terminalToCommand(const std::string &cmd) {
  // TODO: check split string and queue execution logic
  std::string parse_cmd = "";
  std::vector<std::string> cmd_v;
  for (auto &x : cmd) {
    if (x == ' ') {
      cmd_v.push_back(parse_cmd);
      parse_cmd = "";
    } else {
      parse_cmd = parse_cmd + x;
    }
  }
  cmd_v.push_back(parse_cmd);

  if (cmd_v[0] == "queue") {
    switch (term_->convertToEnum(cmd_v[1])) {
      case ADD:
        cs->addCommandToQueue(cmd_v[2]);
        break;
      case START:
        cs->executeQueue();
        break;
      case STOP:
        cs->stopQueueExecution();
        break;
      case ADD_FRONT:
        cs->addCommandToFrontOfQueue(cmd_v[2]);
        break;
      case CLEAR:
        cs->clearQueue();
        break;
      case REMOVE_NEXT:
        cs->removeNextFromQueue();
        break;
      case ALLOW_AUTO_LAND:
        cs->doNotAutoLand();
        break;
      case DO_NOT_AUTO_LAND:
        cs->allowAutoLand();
        break;
      case READ_SEQUENCE:
        readSequence(cmd_v[2]);
      case UNKNOWN:
        utils_log::LogErr() << "Unknown queue command from terminal CLI";
        break;
      default:
        utils_log::LogErr() << "Unknown queue command from terminal CLI";
        break;
    }
  } else {
    bool check = cs->isExecutingQueue();
    if (check) cs->stopQueueExecution();
    std::mutex m;
    std::lock_guard<std::mutex> lk(m);
    cs->sendCommand(cmd);
  }
}
#endif  // TERMINAL

// Refactor this with a nonbloacking read?
void Tello::jsToCommandThread() {
  while (run_) {
    std::this_thread::sleep_for(constants::joystick_timeout);
    js_->update();
    if (js_->hasButtonUpdate()) {
      jsToCommand(js_->getUpdatedButton());
    }
    if (js_->hasAxisUpdate()) {
      jsToCommand(js_->getUpdatedAxis());
    }
  }
}

// TODO(vss): Consider setting wait for response to false when using joysticlk?
void Tello::jsToCommand(ButtonId update) {
  int value = (int)js_->getButtonState(update);
  if (value != 0) {
    bool check = cs->isExecutingQueue();
    if (check) {
      cs->stopQueueExecution();
    }
    switch (update) {
      case BUTTON_A:
        if (js_->getButtonState(BUTTON_LEFT_BUMPER_2) != 0u) {
          if (station_mode_) {
            vs->setSnapshot();
          }
        } else {
          cs->doNotAutoLand();
          cs->sendCommand("takeoff");
        }
        utils_log::LogDebug()
            << "Button [A]: [" << update << "] Value: [" << value << "]";
        break;
      case BUTTON_B:
        if (js_->getButtonState(BUTTON_LEFT_BUMPER_2) != 0u) {
          if (!station_mode_) {
            vs->toggleRecordVideo();
          }
        } else {
          cs->allowAutoLand();
          cs->sendCommand("land");
        }
        utils_log::LogDebug() << "Button [B]: [" << update << "] Value: [" << value << "]";
        break;
      case BUTTON_X:
        if (js_->getButtonState(BUTTON_LEFT_BUMPER_2) != 0u) {
          cs->sendCommand("mon");
        } else {
          cs->sendCommand("streamon");
        }
        utils_log::LogDebug()
            << "Button [X]: [" << update << "] Value: [" << value << "]";
        break;
      case BUTTON_Y:
        if (js_->getButtonState(BUTTON_LEFT_BUMPER_2) != 0u) {
          cs->sendCommand("moff");
        } else {
          cs->sendCommand("streamoff");
        }
        utils_log::LogDebug()
            << "Button [Y]: [" << update << "] Value: [" << value << "]";
        break;
      case BUTTON_RIGHT_BUMPER_1:
        cs->stop();
        utils_log::LogDebug() << "Button [RIGHT_BUMPER_1]: [" << update
                              << "] Value: [" << value << "]";
        break;
      case BUTTON_RIGHT_BUMPER_2:
        cs->emergency();
        utils_log::LogDebug() << "Button [RIGHT_BUMPER_2]: [" << update
                              << "] Value: [" << value << "]";
        break;
      case BUTTON_LEFT_BUMPER_1:
        if (cs->dnal()) {
          cs->allowAutoLand();
        } else {
          cs->doNotAutoLand();
        }
        utils_log::LogDebug() << "Button [LEFT_BUMPER_1]: [" << update
                              << "] Value: [" << value << "]";
        break;
      case BUTTON_LEFT_BUMPER_2:
        utils_log::LogDebug() << "Shift cmd_fx assigned.";
        utils_log::LogDebug() << "Button [LEFT_BUMPER_2]: [" << update
                              << "] Value: [" << value << "]";
        break;
      case BUTTON_START:
        if (js_->getButtonState(BUTTON_LEFT_BUMPER_2) != 0u) {
          // std::this_thread::sleep_for(constants::wait_for_land);
          run_ = false;
          utils_log::LogWarn() << "Exit called from joystick";
        } else {
          cs->sendCommand("command");
        }
        utils_log::LogDebug()
            << "Button [START]: [" << update << "] Value: [" << value << "]";
        break;
      case BUTTON_SELECT:
        utils_log::LogDebug()
            << "Button [SELECT]: [" << update << "] Value: [" << value << "]";
        // NOTE: as joystick commands would immediately stop execution just
        // pressing L2 (shift) would stop execution of the queue, but for HRI
        // reasons using L2+Select will be used to call stop execution.
        if (js_->getButtonState(BUTTON_LEFT_BUMPER_2) != 0u) {
          utils_log::LogDebug() << "Button select stop execute";
          cs->stopQueueExecution();
        } else {
          utils_log::LogDebug() << "Button select execute";
          cs->executeQueue();
        }
        break;
      default:
        utils_log::LogDebug() << "Unknown button command";
        utils_log::LogDebug()
            << "Button: [" << update << "] Value: [" << value << "]";
        break;
    }
  }
}

void Tello::jsToCommand(AxisId update) const {
  int16_t value = js_->getAxisState(update);
  utils_log::LogDebug() << "Axis: [" << update << "] Value: ["
                        << js_->mapConstLimits(value) << "]";
  std::string cmd =
      "rc " + std::to_string(js_->mapConstLimits(js_->getValueAxis(2))) + " " +
      std::to_string(js_->mapConstLimits(js_->getValueAxis(3)) * -1) + " " +
      std::to_string(js_->mapConstLimits(js_->getValueAxis(1)) * -1) + " " +
      std::to_string(js_->mapConstLimits(js_->getValueAxis(0)));
  bool check = cs->isExecutingQueue();
  if (check) {
    cs->stopQueueExecution();
  }
  switch (update) {
    case AXIS_LEFT_STICK_HORIZONTAL:
      cs->sendCommand(cmd);
      break;
    case AXIS_LEFT_STICK_VERTICAL:
      cs->sendCommand(cmd);
      break;
    case AXIS_RIGHT_STICK_HORIZONTAL:
      cs->sendCommand(cmd);
      break;
    case AXIS_RIGHT_STICK_VERTICAL:
      cs->sendCommand(cmd);
      break;
    case AXIS_RIGHT_BUMPER_2:
      break;
    case AXIS_LEFT_BUMPER_2:
      break;
    case AXIS_BUTTONS_HORIZONTAL:
      if (js_->getButtonState(BUTTON_LEFT_BUMPER_2) > 0) {
        if (value > 0) {
          cs->sendCommand("speed?");
        } else if (value < 0) {
          cs->sendCommand("battery?");
        }
      } else {
        if (value > 0) {
          cs->sendCommand("flip r");
        } else if (value < 0) {
          cs->sendCommand("flip l");
        }
      }
      break;
    case AXIS_BUTTONS_VERTICAL:
      if (js_->getButtonState(BUTTON_LEFT_BUMPER_2) != 0u) {
        if (value > 0) {
          cs->sendCommand("time?");
        } else if (value < 0) {
          cs->sendCommand("wifi?");
        }
      } else {
        if (value > 0) {
          cs->sendCommand("flip b");
        } else if (value < 0) {
          cs->sendCommand("flip f");
        }
      }
      break;
    default:
      utils_log::LogDebug() << "Axis: [" << update << "] Value: ["
                            << js_->mapConstLimits(value) << "]";
      break;
  }
}

void Tello::readSequence(const std::string &file) const {
  if (!file.empty()) {
    std::ifstream ifile(file);
    if (!ifile.is_open()) {
      utils_log::LogErr() << "File does not exist";
      return;
    }
    std::string line;
    while (std::getline(ifile, line)) {
      //  NOTE: add check here?
      utils_log::LogDebug() << "Adding to queue from file: [" << line << "]";
      cs->addCommandToQueue(line);
    }
  }
}

Tello::~Tello() {
  utils_log::LogDebug() << "in tello destructor";

#ifdef USE_TERMINAL
  term_thread_worker_.join();
  term_thread_fetch_.join();
  utils_log::LogDebug() << "After terminal joined";
#endif  // TERMINAL

#ifdef USE_JOYSTICK
  if (js_thread_.joinable()) {
    js_thread_.join();
  }
  utils_log::LogDebug() << "After joystick joined";
#endif  // JOYSTICK

  cs->prepareToExit();
  io_service_.stop();
  utils_log::LogDebug() << "After io service stopped";
}

bool Tello::active() const { return run_; }

void Tello::commandInterface(const std::string &cmd_fx,
                             const std::string &cmd) {
  if (cmd_fx == "execute_queue") {
    cs->executeQueue();
  } else if (cmd_fx == "add_command_to_queue") {
    cs->addCommandToQueue(cmd);
  } else if (cmd_fx == "add_command_to_front_of_queue") {
    cs->addCommandToFrontOfQueue(cmd);
  } else if (cmd_fx == "clear_queue") {
    cs->clearQueue();
  } else if (cmd_fx == "stop_queue_execution") {
    cs->stopQueueExecution();
  } else if (cmd_fx == "remove_next_from_queue") {
    cs->removeNextFromQueue();
  } else if (cmd_fx == "do_not_auto_land") {
    cs->doNotAutoLand();
  } else if (cmd_fx == "alow_auto_land") {
    cs->allowAutoLand();
  } else if (cmd_fx == "emergency") {
    cs->emergency();
  } else if (cmd_fx == "stop") {
    cs->stop();
  } else if (cmd_fx == "is_executing_queue") {
    cs->isExecutingQueue();
  } else if (cmd_fx == "land") {
    cs->land();
  } else {
    utils_log::LogErr() << "Unknown command sent to command interface";
  }
}
