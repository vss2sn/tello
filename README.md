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
5. SLAM integration has been provided using the OpenVSLAM library.

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
3. `USE_JOYSTICK`
    - Default `ON`
    - When set to `ON` allows the use of a joystick to control the drone and exits if a joystick is not found at startup.
4. `RECORD`
    - Default `OFF`
    - When set to `ON` records the video
5. `RUN_SLAM`
    - Default `OFF`
    - When set to `ON` runs OpenVSLAM, creating a map of the area and localizing the drone
    - Requires a configuration file for the camera as well as  ORB vocabulary file
    - Please run the install_openvslam using `bash -i install_openvslam` to install OpenVSLAM and its dependencies
    - Please download a sample vocabulary file [here](https://drive.google.com/open?id=1wUPb328th8bUqhOk-i8xllt5mgRW4n84) and store it in the main directory
    - Please copy the sample config.yaml file into the build directory

#### To build and run ####
    git clone https://github.com/vss2sn/tello.git  
    cd tello  
    sudo apt install libasio-dev libopencv-dev
    mkdir -p build && cd build  
    cmake ..
    make -j4
    ./tello

#### To build and run with SLAM ####
    git clone https://github.com/vss2sn/tello.git  
    cd tello  
    sudo apt install libasio-dev libopencv-dev
    bash -i install_openvslam
    # OR skip the above step and
    # run cmake with the option -DUSE_CMAKE_NOT_SCRIPT=ON
    mkdir -p build && cd build  
    cmake -DRUN_SLAM=ON ..
    make -j4
    ./tello


#### Required Dependencies ####
1. Asio and std::threads or Boost Asio and Boost threads (CMake option provided)
2. OpenCV (for video socket)

#### Optional Dependencies for SLAM ####
1. OpenVSLAM
2. Eigen3
3. g2o
4. DBoW2
5. OpenCV >= 3.4
6. Pangolin

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
| R1 | Control command `stop` |
| R2 | Control command `emergency` |
| L1 | Toggle autoland |
| L2 | Shift (To be used in conjunction with other buttons/axes) |
| A  | Prevent automatic landing and control command `takeoff` |
| B  | Allow automatic landing and control command `land` |
| X  | Control command `streamon` |
| Y  | Control command `streamoff` |
| A  + L2 | Take snapshot |
| X  + L2 | Control command `mon` |
| Y  + L2 | Control command `moff` |
| START | Control command `command` |
| SELECT | Start queue execution |
| START + L2 | Control command `land` and exit |
| SELECT + L2 | Stop queue execution |

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
| Right Button + L2 | Read command `speed?` |
| Left Button + L2 | Read command: `battery?` |
| Back Button + L2 | Read command: `time?` |
| Forward Button + L2 | Read command: `wifi?` |

#### Notes ####
1. Due to the asynchronous nature of the communication, the responses printed to the command might not be to the command state in the statement (for example in case the joystick was moved after a land command was sent, the statement would read `received response ok to command rc a b c d` instead of `received response ok to command land`)
2. More detailed documentation on the queue execution and its related functions, preventing auto land, threads, etc will be added in upcoming commits
3. The SLAM integration is a WIP; the API is being refactored and expanded to allow more options. There are 2 options to install and use OpenVSLAM and its dependencies with this code.
  - Running the install script will get, build and install the required packages into the lib_openvslam directory, and add the necessary paths to ~/.bashrc for compile time and runtime linking
  - Run cmake with the option `USE_CMAKE_NOT_SCRIPT` set to `ON` which will get, build and install the required packages into the lib_openvslam directory without modifying the ~/.bashrc file

#### References ####
1. Joystick library - https://github.com/Notgnoshi/joystick
2. h264decoder library - https://github.com/DaWelter/h264decoder
3. OpenVSLAM - https://github.com/xdspacelab/openvslam

#### Project paths (current & projected) ####
1. Refactor OpenVSLAM API
2. Create documentation and site
3. Add in other SLAM options
4. Clean up includes
5. Add script for camera configuration from video recorded on Tello
6. Add watchdog to check for state data
7. Integrate SLAM data into command structure to allow waypoints
