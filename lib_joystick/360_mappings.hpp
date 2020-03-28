#ifndef 360MAPPINGS_HPP
#define 360MAPPINGS_HPP

#define EVENT_BUTTON         0x01    // Button state update
#define EVENT_AXIS           0x02    // Axis state update
#define EVENT_INIT           0x80    // Non-input event, to be ignored

#define MAX_BUTTON_COUNT       16
#define MAX_AXIS_COUNT          8    // 8 for 360, 27 for PS3

#define BUTTON_DOWN             1
#define BUTTON_UP               0

// Added deadband to make the axes less sensitive
#define DEADBAND 20

// "Axis" definitions.
// Down (trigger pulled in, stick down) +ve values
// Right +ve values
// Up (trigger not pulled in, stick up) -ve values
// Left -ve values
enum AxisId
{
    AXIS_LEFT_STICK_HORIZONTAL,
    AXIS_LEFT_STICK_VERTICAL,
    AXIS_LEFT_TRIGGER,
    AXIS_RIGHT_STICK_HORIZONTAL,
    AXIS_RIGHT_STICK_VERTICAL,
    AXIS_RIGHT_TRIGGER,
};

#define AXIS_COUNT 6

//  Button definitions. Down is 1, up is 0
enum ButtonId
{
    BUTTON_A,
    BUTTON_B,
    BUTTON_X,
    BUTTON_Y,
    BUTTON_LEFT_BUMPER,
    BUTTON_RIGHT_BUMPER,
    BUTTON_BACK,
    BUTTON_START,
    BUTTON_XBOX,
    BUTTON_LEFT_CLICK,
    BUTTON_RIGHT_CLICK,
};

#define BUTTON_COUNT 11

#endif // 360MAPPINGS_HPP
