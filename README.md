# Tello #

### This repository contains a C++ library that enables interaction with DJI/RYZE Tello and Tello Edu drones ###

#### Overview ####
1. This code does not depend on ROS/ROS2 for flying using a joystick/sending commands listed in the SDK
2. The code has 2 methods to control the drone.
    - Using a joystick
    - Autonomously using a queue in which commands can be stored and dynamically added and removed and whose execution can be paused and restarted
3. In Joystick mode:
    - The joystick is always active (though it's use can be safely commented out)
    - The commands are continuously sent from the drone without waiting for a response
    - Based on the controller used, the mappings might require modification
    - The default controller is gameSirT1s; key/axis to value mappings for PS3 and 360 controller have been provided, though the mappings between the values to the commands sent to the drone might need modification
4. In autonomous mode:
    - This queue is always overridden by the joystick as a safety measure
    - using the joystick while the queue is executing pauses the queue, which will need to be restarted even when there is no longer additional input from the joystick
    - Queue execution can be safely paused and resumed
    - This mode enables (optional) command retries when a command does not receive any response from the drone

#### File Structure ####

|Folder|Contains|
|-------------|-------------|
| inc  | header files for different sockets and tello classes |
| src  | source files for different sockets and tello classes |
| lib_utils | common functions such as functions for logging |
| lib_joystick | joystick library and controller mappings |
| lib_h264decoder | h264decoder library used to decode images from tello |
| main | main executable |

#### CMake options ####
1. `SIMPLE`
    - Default `OFF`
    - When `OFF` the output to the screen will also print the line number and file number of the print statement as well as the log level (STATUS, DEBUG, INFO, WARN, ERR).
2. `USE_BOOST`
    - Default `OFF`
    - For those more familiar with `BOOST`, the library can be made to use boost::threads and boost::asio. When set to `OFF` is uses `libasio` (written by the author of boost asio) and std::threads.

#### To build and run ####
    # begin dependancies
    sudo apt install libasio-dev # or sudo apt install libboost-dev
    sudo apt install ibopencv-dev
    # end dependancies
    git clone https://github.com/vss2sn/tello.git  
    cd tello  
    mkdir build  
    cd build  
    cmake .. && make -j4
    ./tello

#### Dependancies ####
1. Asio and std::threads or Boost Asio and Boost threads (CMake option provided)
2. OpenCV (for video socket)

#### Code overview #####
1. `BaseSocket` is an abstract class providing the framework for the other sockets
2. `VideoSocket` is a class that connects to the video streaming port of the tello
3. `StateSocket` is a class that connects to the port where the state of the tello is continuously published and displays the state
4. `CommandSocket` is the class that creates and stores and manages the command queue and its execution, sends commands to the drone, waits for its response (if timeout set) and retries sending commands (if retries enabled)
5. `Tello` class is a wrapper class that instantiates a command, video and state socket as well as a joystick. As all commands are always sent via the command socket, `Tello` class implements functions to convert the joystick inputs to commands and calls the function to send commands, ensuring that the joystick library is isolated
6. To ensure that the tello does not automatically land after 15 seconds (this is in the Tello firmware) a command of `rc 0 0 0 0` is sent if no other commands have been sent. The timeout of this command can be set as required, and this feature can be activated/deactivated
7. The tello does not wait receive a response when an `rc` command is sent, regardless of whether the command is sent via joystick or autonomously

#### Current joystick mappings ####

| Button | Function |
|-------------|-------------|
| A  | Prevent automatic landing and control command `takeoff` |
| B  | Allow automatic landing and control command `land` |
| X  | Control command `streamon` |
| Y  | Control command `streamoff` |
| R1 | Control command `stop` |
| R2 | Control command `emergency` |
| L1 | Toggle autoland |
| L2 | Shift (To be used in conjunction with other buttons/axes) |
| START | Control command `command` |
| SELECT | Start queue execution |
| START + L1 | Control command `land` and exit |
| SELECT + L1 | Stop queue execution |

| Axis | Function |
|-------------|-------------|
| Right Stick Horizontal | Set command: Leftward/Rightward velocity `a` in `rc a b c d` |
| Right Stick Vertical | Set command: Forward/Backward velocity `b` in `rc a b c d` |
| Left Stick Horizontal | Set command: Angular velocity (Yaw) `d` in `rc a b c d` |
| Left Stick Vertical | Set command: Vertical velocity `c` in `rc a b c d` |
| Right Button | Control command `flip r`  |
| Left Button | Control command `flip l` |
| Back Button | Control command `flip b` |
| Forward Button | Control command `flip f` |
| Right Button + L1 | Read command `speed?` |
| Left Button + L1 | Read command: `battery?` |
| Back Button + L1 | Read command: `time?` |
| Forward Button + L1 | Read command: `wifi?` |

#### Notes ####
1. Due to the asynchronous nature of the communication, the responses printed to the command might not be to the command state in the statement (for example in case the joystick was moved after a land command was sent, the statement would read `received response ok to command rc a b c d` instead of `received response ok to command land`)
2. More detailed documentation on the queue execution and its related functions, preventing auto land, threads, etc will be added in upcoming commits

#### References ####
1. Joystick library - https://github.com/Notgnoshi/joystick
2. h264decoder library - https://github.com/DaWelter/h264decoder
