#include "joystick.hpp"

Joystick::Joystick(std::string port)
{
    int fd = open(port.c_str(), O_RDONLY);
    if (fd < 0)
    {
        std::cout << "Failed to open [" << port << "]." << std::endl;
        exit(0);
    }
    this->port_ = fd;
}

Joystick::~Joystick()
{
    accept_js_input_ = false;
    close(this->port_);
}

void Joystick::update()
{
    // zero out the previous event
    memset(&event_, 0, JS_EVENT_SIZE);
    size_t bytes_read = 0;
    ssize_t tmp = 0;

    // Blocking read
    while (bytes_read < JS_EVENT_SIZE)
    {
        tmp = read(port_, &event_ + bytes_read, JS_EVENT_SIZE - bytes_read);
        if (tmp > 0)
        {
            bytes_read += tmp;
        }
    }

    event_.type &= ~EVENT_INIT;

    if (event_.type == EVENT_AXIS)
    {
        updated_axis_ = static_cast<AxisId>(event_.id);
        is_axis_update_ = true;
        is_button_update_ = false;
        axis_values_[event_.id] = event_.value;
    }

    if (event_.type == EVENT_BUTTON)
    {
        updated_button_ = static_cast<ButtonId>(event_.id);
        is_axis_update_ = false;
        is_button_update_ = true;
        button_values_[event_.id] = event_.value;
    }
}

bool Joystick::hasButtonUpdate()
{
    return is_button_update_;
}

bool Joystick::hasAxisUpdate()
{
    return is_axis_update_;
}

uint8_t Joystick::getButtonState(ButtonId button_id)
{
    return button_values_[(int) button_id];
}

int16_t Joystick::getAxisState(AxisId axis_id)
{
    return axis_values_[(int) axis_id];
}

ButtonId Joystick::getUpdatedButton()
{
    return updated_button_;
}

AxisId Joystick::getUpdatedAxis()
{
    return updated_axis_;
}

short Joystick::map(long x, long in_min, long in_max, long out_min, long out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

#define OUT_MAX 100
#define OUT_MIN -100
short Joystick::mapConstLimits(long x)
{
    return (x - SHRT_MIN) * (OUT_MAX - OUT_MIN) / (SHRT_MAX - SHRT_MIN) + OUT_MIN;
}

void Joystick::update_buttons()
{
    ButtonId update = getUpdatedButton();
    unsigned int value = getButtonState(update);

    switch (update)
    {
    BUTTON_A:
    BUTTON_B:
    BUTTON_X:
    BUTTON_Y:
    BUTTON_LEFT_BUMPER_1:
    BUTTON_RIGHT_BUMPER_1:
    BUTTON_LEFT_BUMPER_2:
    BUTTON_RIGHT_BUMPER_2:
    BUTTON_START:
    BUTTON_TURBO:
    BUTTON_SELECT:
    BUTTON_CLEAR:
    BUTTON_LEFT_CLICK:
    BUTTON_RIGHT_CLICK:
    BUTTON_FRONT_CLICK:
    BUTTON_BACK_CLICK:
    default:
        std::cout << "Button: [" << update << "] Value: [" << value <<"]" << std::endl;
        break;
    }
}

void Joystick::update_axes()
{
    AxisId update = getUpdatedAxis();
    int16_t value = getAxisState(update);

    switch (update)
    {
    AXIS_LEFT_STICK_HORIZONTAL:
    AXIS_LEFT_STICK_VERTICAL:
    AXIS_LEFT_TRIGGER:
    AXIS_RIGHT_STICK_HORIZONTAL:
    AXIS_RIGHT_STICK_VERTICAL:
    AXIS_RIGHT_TRIGGER:
    default:
        std::cout << "Axis: [" << update << "] Value: [" << value << "]" << std::endl;
        break;
    }
}

void Joystick::worker(){
  while(accept_js_input_)
  {
      usleep(1000);
      update();
      if (hasButtonUpdate())
      {
          update_buttons();
      }
      if (hasAxisUpdate())
      {
          update_axes();
      }
  }
}

uint8_t Joystick::getValueButton(int i){
  return axis_values_[i];
}

int16_t Joystick::getValueAxis(int i){
  return axis_values_[i];
}
