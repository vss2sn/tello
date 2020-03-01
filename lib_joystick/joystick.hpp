#ifndef JOYSTICK_HPP
#define JOYSTICK_HPP

#include <iostream>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <climits>

#ifdef USE_BOOST

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#else

#include <thread>
#include "asio.hpp"

#endif

#include "gameSirT1s_mappings.hpp"
// #include "360_mappings.hpp"
// #include "ps3_mappings.hpp"

struct js_event
{
    uint32_t time;     /* event timestamp in milliseconds */
    int16_t value;     /* value */
    uint8_t type;      /* event type */
    uint8_t id;        /* axis/button number */
};

const size_t JS_EVENT_SIZE = sizeof(js_event);

class Joystick
{
public:
    Joystick(std::string port="/dev/input/js0");
    ~Joystick();

    void update();

    bool hasButtonUpdate();
    bool hasAxisUpdate();

    ButtonId getUpdatedButton();
    AxisId getUpdatedAxis();

    uint8_t getButtonState(ButtonId button_id);
    int16_t getAxisState(AxisId axis_id);

    void update_buttons();
    void update_axes();

    static short map(long x, long in_min, long in_max, long out_min, long out_max);
    static short mapConstLimits(long x);

    void worker();

    uint8_t getValueButton(int i);
    int16_t getValueAxis(int i);

private:
    int port_;
    js_event event_;

    ButtonId updated_button_;
    AxisId updated_axis_;

    bool is_axis_update_;
    bool is_button_update_;

    uint8_t button_values_[MAX_BUTTON_COUNT] = {0}; // Using max to allow for buttons not defined in header enum Button_Id
    int16_t axis_values_[MAX_AXIS_COUNT] = {0}; // Using max to allow for axes not defined in header enum Axis_Id

    bool accept_js_input_ = true;

#ifdef USE_BOOST
    boost::thread js_thread_;
#else
    std::thread js_thread_;
#endif
};

#endif JOYSTICK_HPP
