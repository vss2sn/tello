# Tello #

### This repository contains a C++ library that enables interaction with DJI/RYZE Tello and Tello Edu drones ###

#### Notes ####
1. This code does not depend on ROS/ROS2 for flying using a joystick/sending commands listed in the SDK
2. The code has 2 methods to control the drone.
    - Using a joystick
    - Autonomously using a queue in which commands can be stored
3. In Joystick mode:
    - The joystick is always active (though it's use can be safely commented out)
    - Based on the controller used, the mappings might require modification
    - Does not wait for a response when ending commands
4. In autonomous mode:
    - This queue is always overridden by the joystick as a safety measure; using the joystick while the queue is executing pauses the queue, which will need to be restarted even when there is no longer additional input from the  joystick
    - Its execution can be safely paused and resumed
    - It enables (optional) command retries when a command does not receive any response from the drone

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
    git clone https://github.com/vss2sn/tello.git  
    cd tello  
    mkdir build  
    cd build  
    cmake .. && make -j
    ./main  

#### Dependancies ####
1. Asio or Boost Asio (CMake option provided)
2. OpenCV (for video socket)
3. ffmpeg (for video socket)

#### Code overview #####
1. `base_socket` is an abstract class providing the framework for the other sockets
2. `video_socket` is a class that connects to the video streaming port of the tello
3. `state_socket` is a class that connects to the port where the state of the tello is continuously published and displays the state
4. `command_socket` is the class that creates and stores and manages the command queue and its execution, sends commands to the tello, waits for its response (if timeout set) and retries sending commands (if retries enabled)
5. `tello` class is a wrapper class that instantiates a command, video and state socket as well as a joystick. As all commands are always sent via the command socket, `tello` class implements functions to convert the joystick inputs to commands and calls the function to send commands, ensuring that the joystick library is isolated
6. To ensure that the tello does not automatically land after 15 seconds (this is in the Tello firmware) a command of `rc 0 0 0 0` is sent if no other commands have been sent. The timeout of this command can be set as required, and this feature can be activated/deactivated.
