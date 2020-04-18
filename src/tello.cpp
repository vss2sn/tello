#include "tello.hpp"

Tello::Tello(
#ifdef USE_BOOST
    boost::asio::io_service& io_service,
#else
    asio::io_service& io_service,
#endif
std::condition_variable& cv_run,
const std::string drone_ip,
const std::string local_drone_port,
const std::string local_video_port,
const std::string local_state_port,
const std::string camera_config_file,
const std::string vocabulary_file,
const int n_retries,
const int timeout,
const std::string load_map_db_path,
const std::string save_map_db_path,
const std::string mask_img_path,
bool load_map,
bool continue_mapping,
float scale
):
io_service_(io_service),
cv_run_(cv_run)
{
  cs = std::make_unique<CommandSocket>(io_service, drone_ip, "8889", local_drone_port, n_retries, timeout);
  vs = std::make_unique<VideoSocket>(io_service,  "0.0.0.0", "11111", local_video_port,
    run_, camera_config_file, vocabulary_file, load_map_db_path, save_map_db_path,
    mask_img_path, load_map, continue_mapping, scale);
  ss = std::make_unique<StateSocket>(io_service, "0.0.0.0", "8890", local_state_port);

#ifdef USE_JOYSTICK
  js_ = std::make_unique<Joystick>();
  js_thread_ = std::thread([&]{jsToCommandThread();});
  js_thread_.detach();
#endif // USE_JOYSTICK

#ifdef USE_TERMINAL
  term_ = std::make_unique<Terminal>(run_);
  term_thread_worker_ = std::thread([&]{term_->terminalWorker();});
  term_thread_fetch_ = std::thread([&]{terminalToCommandThread();});
  term_thread_worker_.detach();
  term_thread_fetch_.detach();
#endif // TERMINAL
}

#ifdef USE_TERMINAL
void Tello::terminalToCommandThread(){
  while(run_)
  {
    usleep(1000);
    if(term_->hasCommnad()){
      terminalToCommand(term_->getCommand());
    }
  }
  std::cout << "----------- Terminal to command thread exits -----------" << std::endl;
}

void Tello::terminalToCommand(const std::string& cmd){

  // TODO: check split string and queue execution logic
  std::string parse_cmd = "";
  std::vector<std::string> cmd_v;
  for (auto& x : cmd)
  {
     if (x == ' ')
     {
         cmd_v.push_back(parse_cmd);
         parse_cmd = "";
     }
     else
     {
         parse_cmd = parse_cmd + x;
     }
  }
  cmd_v.push_back(parse_cmd);

  if(cmd_v[0] == "queue"){
    switch(term_->convertToEnum(cmd_v[1])){
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
      case UNKNOWN:
        utils_log::LogErr() << "Unknown queue command from terminal CLI";
        break;
      default:
        utils_log::LogErr() << "Unknown queue command from terminal CLI";
        break;
    }
  }
  else{
    bool check = cs->isExecutingQueue();
    if(check) cs->stopQueueExecution();
    std::mutex m;
    std::lock_guard<std::mutex> lk(m);
    cs->sendCommand(cmd);
  }
}
#endif // TERMINAL


// Refactor this with a nonbloacking read?
void Tello::jsToCommandThread(){
  while(run_)
  {
      usleep(1000);
      js_->update();
      if(!run_) break; // Is this necessary?
      if (js_->hasButtonUpdate())
      {
          jsToCommand(js_->getUpdatedButton());
      }
      if (js_->hasAxisUpdate())
      {
          jsToCommand(js_->getUpdatedAxis());
      }
  }
}


// TODO: Consider setting wait for response to false when using joysticlk?
void Tello::jsToCommand(ButtonId update){
  int value = (int)js_->getButtonState(update);
  if(value!=0){
    bool check = cs->isExecutingQueue();
    if(check) cs->stopQueueExecution();
    switch (update)
    {
      case BUTTON_A:
        if(js_->getButtonState(BUTTON_LEFT_BUMPER_2)){
          vs->setSnapshot();
        }
        else{
          cs->doNotAutoLand();
          cs->sendCommand("takeoff");
        }
        utils_log::LogDebug() << "Button [A]: [" << update << "] Value: [" << value <<"]";
        break;
      case BUTTON_B:
        cs->allowAutoLand();
        utils_log::LogDebug() << "Button [B]: [" << update << "] Value: [" << value <<"]";
        cs->sendCommand("land");
        break;
      case BUTTON_X:
        if(js_->getButtonState(BUTTON_LEFT_BUMPER_2)){
          cs->sendCommand("mon");
        }
        else{
          cs->sendCommand("streamon");
        }
        utils_log::LogDebug() << "Button [X]: [" << update << "] Value: [" << value <<"]";
        break;
      case BUTTON_Y:
        if(js_->getButtonState(BUTTON_LEFT_BUMPER_2)){
          cs->sendCommand("moff");
        }
        else{
          cs->sendCommand("streamoff");
        }
        utils_log::LogDebug() << "Button [Y]: [" << update << "] Value: [" << value <<"]";
        break;
      case BUTTON_RIGHT_BUMPER_1:
        cs->stop();
        utils_log::LogDebug() << "Button [RIGHT_BUMPER_1]: [" << update << "] Value: [" << value <<"]";
        break;
      case BUTTON_RIGHT_BUMPER_2:
        cs->emergency();
        utils_log::LogDebug() << "Button [RIGHT_BUMPER_2]: [" << update << "] Value: [" << value <<"]";
        break;
      case BUTTON_LEFT_BUMPER_1:
        if(cs->dnal_) cs->allowAutoLand();
        else cs->doNotAutoLand();
        utils_log::LogDebug() << "Button [LEFT_BUMPER_1]: [" << update << "] Value: [" << value <<"]";
        break;
      case BUTTON_LEFT_BUMPER_2:
        utils_log::LogDebug() << "Shift function assigned.";
        utils_log::LogDebug() << "Button [LEFT_BUMPER_2]: [" << update << "] Value: [" << value <<"]";
        break;
      case BUTTON_START:
        if(js_->getButtonState(BUTTON_LEFT_BUMPER_2)){
          usleep(1000000);
          cs->land();
          usleep(5000000); // Block any other joystick input
          utils_log::LogWarn() << "Exit called from joystick";
          // NOTE: Notification of calling end of code kept here* to allow expansion
          // to swarm, where a single joystick might be used to command multiple
          // Tellos at which point this function will be commented using a #define
          // and a swarm joystick function would be run that sends caommands to
          // all the Tellos in the swarm. it would be rewuired that the above
          // function be called for all the tellos before the cv_run_.notify_all
          // is called.
          // *(instead of in command socket)
          {
            std::mutex mut;
            std::lock_guard<std::mutex> lk(mut);
            cv_run_.notify_all();
          }
        }
        else{
          cs->sendCommand("command");
        }
        utils_log::LogDebug() << "Button [START]: [" << update << "] Value: [" << value <<"]";
        break;
      case BUTTON_SELECT:
        utils_log::LogDebug() << "Button [SELECT]: [" << update << "] Value: [" << value <<"]";
          // NOTE: as joystick commands would immediately stop execution just
          // pressing L2 (shift) would stop execution of the queue, but for HRI
          // reasons using L2+Select will be used to call stop execution.
        if(js_->getButtonState(BUTTON_LEFT_BUMPER_2)){
          utils_log::LogDebug() << "Button select stop execute";
          cs->stopQueueExecution();
        }
        else{
          utils_log::LogDebug() << "Button select execute";
          cs->executeQueue();
        }
        break;
      default:
        utils_log::LogDebug() << "Unknown button command";
        utils_log::LogDebug() << "Button: [" << update << "] Value: [" << value <<"]";
        break;
    }
  }
}

void Tello::jsToCommand(AxisId update){
  int16_t value = js_->getAxisState(update);
  utils_log::LogDebug() << "Axis: [" << update << "] Value: [" << js_->mapConstLimits(value) <<"]";
  std::string cmd = "rc "
  + std::to_string(js_->mapConstLimits(js_->getValueAxis(2))) + " "
  + std::to_string(js_->mapConstLimits(js_->getValueAxis(3))*-1) + " "
  + std::to_string(js_->mapConstLimits(js_->getValueAxis(1))*-1) + " "
  + std::to_string(js_->mapConstLimits(js_->getValueAxis(0)));
  bool check = cs->isExecutingQueue();
  if(check) cs->stopQueueExecution();
  switch (update)
  {
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
      if(js_->getButtonState(BUTTON_LEFT_BUMPER_2) > 0){
        if(value > 0) cs->sendCommand("speed?");
        else if(value < 0) cs->sendCommand("battery?");
      }
      else{
        if(value > 0) cs->sendCommand("flip r");
        else if(value < 0) cs->sendCommand("flip l");
      }
      break;
    case AXIS_BUTTONS_VERTICAL:
      if(js_->getButtonState(BUTTON_LEFT_BUMPER_2)){
        if(value > 0) cs->sendCommand("time?");
        else if(value < 0) cs->sendCommand("wifi?");
      }
      else{
        if(value > 0) cs->sendCommand("flip b");
        else if(value < 0) cs->sendCommand("flip f");
      }
      break;
    default:
        utils_log::LogDebug() << "Axis: [" << update << "] Value: [" << js_->mapConstLimits(value) <<"]";
        break;
  }
}

Tello::~Tello(){
  run_ = false;
  usleep(1000000);
}
