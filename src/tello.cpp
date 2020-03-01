#include "tello.hpp"

Tello::Tello(
#ifdef USE_BOOST
    boost::asio::io_service& io_service
#else
    asio::io_service& io_service
#endif
):io_service_(io_service)
{
  cs = std::make_unique<CommandSocket>(io_service, "192.168.10.1", "8889", "8889", 0,5);
  vs = std::make_unique<VideoSocket>(io_service,  "0.0.0.0", "11111", "11111");
  ss = std::make_unique<StateSocket>(io_service, "0.0.0.0", "8890", "8890");
  js_ = std::make_unique<Joystick>();

  #ifdef USE_BOOST
      js_thread_ = boost::thread(boost::bind(&Tello::jsToCommandThread, this));
  #else
      js_thread_ = std::thread([&]{jsToCommandThread();});
      js_thread_.detach();
  #endif
}

void Tello::jsToCommandThread(){
  while(run_)
  {
      usleep(1000);
      js_->update();
      if(!run_) break;
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
        cs->doNotAutoLand();
        cs->sendCommand("takeoff");
        LogDebug() << "Button [A]: [" << update << "] Value: [" << value <<"]";
        break;
      case BUTTON_B:
        cs->allowAutoLand();
        LogDebug() << "Button [B]: [" << update << "] Value: [" << value <<"]";
        cs->sendCommand("land");
        break;
      case BUTTON_X:
        cs->sendCommand("streamon");
        LogDebug() << "Button [X]: [" << update << "] Value: [" << value <<"]";
        break;
      case BUTTON_Y:
        cs->sendCommand("streamoff");
        LogDebug() << "Button [Y]: [" << update << "] Value: [" << value <<"]";
        break;
      case BUTTON_RIGHT_BUMPER_1:
        cs->stop();
        LogDebug() << "Button [RIGHT_BUMPER_1]: [" << update << "] Value: [" << value <<"]";
        break;
      case BUTTON_RIGHT_BUMPER_2:
        cs->emergency();
        LogDebug() << "Button [RIGHT_BUMPER_2]: [" << update << "] Value: [" << value <<"]";
        break;
      case BUTTON_LEFT_BUMPER_1:
        if(cs->dnal_) cs->allowAutoLand();
        else cs->doNotAutoLand();
        LogDebug() << "Button [RIGHT_BUMPER_1]: [" << update << "] Value: [" << value <<"]";
        break;
      case BUTTON_LEFT_BUMPER_2:
        LogDebug() << "Shift function assigned.";
        LogDebug() << "Button [RIGHT_BUMPER_2]: [" << update << "] Value: [" << value <<"]";
        break;
      case BUTTON_START:
        cs->sendCommand("command");
        LogDebug() << "Button [START]: [" << update << "] Value: [" << value <<"]";
        break;
      case BUTTON_SELECT:
        LogDebug() << "Button [SELECT]: [" << update << "] Value: [" << value <<"]";
        if(check){
          LogDebug() << "Button select stop execute";
          cs->stopQueueExecution();
        }
        else{
          LogDebug() << "Button select execute";
          cs->executeQueue();
        }
        break;
      default:
        LogDebug() << "Unknown button command";
        LogDebug() << "Button: [" << update << "] Value: [" << value <<"]";
        break;
    }
  }
}

void Tello::jsToCommand(AxisId update){
  int16_t value = js_->getAxisState(update);
  LogDebug() << "Axis: [" << update << "] Value: [" << js_->mapConstLimits(value) <<"]";
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
        LogDebug() << "Axis: [" << update << "] Value: [" << js_->mapConstLimits(value) <<"]";
        break;
  }
}

Tello::~Tello(){
  run_ = false;
  usleep(10000);
  cs->sendCommand("land");
}
