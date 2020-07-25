#ifndef GAMESIRT1SMAPPINGS_HPP
#define GAMESIRT1SMAPPINGS_HPP

#define EVENT_BUTTON         0x01    // Button state update
#define EVENT_AXIS           0x02    // Axis state update
#define EVENT_INIT           0x80    // Non-input event, to be ignored

#define MAX_BUTTON_COUNT       15    // Kept randomly high
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
    AXIS_LEFT_STICK_HORIZONTAL,        // 0
    AXIS_LEFT_STICK_VERTICAL,          // 1
    AXIS_RIGHT_STICK_HORIZONTAL,       // 2
    AXIS_RIGHT_STICK_VERTICAL,         // 3
    AXIS_RIGHT_BUMPER_2,               // 4
    AXIS_LEFT_BUMPER_2,                // 5
    AXIS_BUTTONS_HORIZONTAL,           // 6
    AXIS_BUTTONS_VERTICAL              // 7
};

#define AXIS_COUNT 8
// Update max axis count

//  Button definitions. Down is 1, up is 0
enum ButtonId
{
    BUTTON_A,                          // 0
    BUTTON_B,                          // 1
    BUTTON_UNKNOWN_1,                  // 2
    BUTTON_X,                          // 3
    BUTTON_Y,                          // 4
    BUTTON_UNKNOWN_2,                  // 5
    BUTTON_LEFT_BUMPER_1,              // 6
    BUTTON_RIGHT_BUMPER_1,             // 7
    BUTTON_LEFT_BUMPER_2,              // 8
    BUTTON_RIGHT_BUMPER_2,             // 9
    BUTTON_SELECT,                     // 10
    BUTTON_START,                      // 11
    BUTTON_UNKNOWN_12,                 // 12
    BUTTON_LEFT_STICK,                 // 13
    BUTTON_RIGHT_STICK                 // 14

};

#define BUTTON_COUNT 15
// Update max button count

#endif // GAMESIRT1SMAPPINGS_HPP
