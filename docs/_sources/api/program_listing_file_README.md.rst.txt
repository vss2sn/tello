
.. _program_listing_file_README.md:

Program Listing for File README.md
==================================

|exhale_lsh| :ref:`Return to documentation for file <file_README.md>` (``README.md``)

.. |exhale_lsh| unicode:: U+021B0 .. UPWARDS ARROW WITH TIP LEFTWARDS

.. code-block:: markdown

   # Tello #
   
   ### This repository contains a C++ library that enables interaction with DJI/RYZE Tello and Tello Edu drones ###
   
   [![Build Status](https://travis-ci.com/vss2sn/tello.svg?branch=master)](https://travis-ci.com/vss2sn/tello)
   
   #### This README contains a quick overview of the repository. Please refer to the [documentation](https://vss2sn.github.io/tello/) for details. ####
   
   <a name="toc"></a>
   #### Table of contents ####
   - [Table of contents](#toc)
   - [Overview](#overview)
   - [CMake Options](#cmake)
   - [Quickstart](#qs)
   - [Build and run with SLAM](#bslam)
   - [Default joystick mappings](#joy)
   - [Notes](#notes)
   - [References](#ref)
   - [Project paths (current & projected)](#pp)
   - [Troubleshooting](#ts)
   
   <a name="overview"></a>
   #### Overview ####
   
   Command/control methods
   1. Using a joystick
   2. Autonomously using a command queue
   3. Command line interface
   
   Joystick mode
   * The joystick is always active (though it's use can be safely removed by setting the CMake option `USE_JOYSTICK` to `OFF`)
   * Commands from the joystick are continuously sent to the drone without waiting for a response
   * Based on the controller used, the mappings might require modification
   * The default controller is gameSirT1s; key/axis to value mappings for PS3 and 360 controller have been provided, though the mappings between the values to the commands sent to the drone might need modification
   
   Autonomous mode
   * This queue is always overridden by the joystick as a safety measure
   * Using the joystick while the queue is executing pauses the queue, which will need to be restarted even when there is no longer additional input from the joystick
   * Queue execution can be safely paused and resumed
   * Commands can be dynamically added
   * This mode enables (optional) command retries when a command does not receive any response from the drone
   
   Command line interface
   * A command line interface can be brought up by setting the CMake option `USE_TERMINAL` to `ON`
   * Like the joystick, the CLI will pause command queue execution
   * To send a command to the drone, the command format is `SDK command` (eg: land)
   * To modify the queue via the terminal, the command format is `queue <function> <command>` where
     `function` : add, start, stop, addfront, clear, removenext, allowautoland, donotautoland
     `command`  : (required argument for some, not all, of the above) `SDK command`
     (eg: queue start, queue addfront takeoff)
   
   SLAM integration has been provided using the OpenVSLAM library.
   
   <a name="cmake"></a>
   #### CMake options ####
   1. `SIMPLE`
       - Default `OFF`
       - When `OFF` the output to the screen will also print the line number and file number of the print statement as well as the log level (STATUS, DEBUG, INFO, WARN, ERR).
   2. `USE_JOYSTICK`
       - Default `ON`
       - When set to `ON` allows the use of a joystick to control the drone and exits if a joystick is not found at startup.
   3. `RUN_SLAM`
       - Default `OFF`
       - When set to `ON` runs OpenVSLAM, creating a map of the area and localizing the drone
   4. `USE_TERMINAL`
       - Default `OFF`
       - When set to `ON` opens up an xterm that takes in lines and sends them as commands to the Tello (example: `command` , `takeoff`, etc)
   5. `USE_CONFIG`
       - Default `OFF`
       - When set to `ON` uses the config manager to create the Tello from the config file `config.yaml`
   
   <a name="qs"></a>
   #### Quickstart ####
       sudo apt install libasio-dev libopencv-dev
       git clone https://github.com/vss2sn/tello.git  
       cd tello  
       mkdir -p build && cd build  
       cmake ..
       make -j4
       ./tello
   
   <a name="bslam"></a>
   #### To build and run with SLAM ####
   
   Please refer to the [documentation](https://vss2sn.github.io/tello/building.html) for details
   
   <a name="joy"></a>
   #### Default joystick mappings ####
   
   Please note that mappings have been provided for PS3 and Xbox360 controllers as well, but they might require some tweaking. The default mapping is for the gameSirT1s.
   
   The mappings include querying `battery`, `wifi`, etc as well as `stop`, `emergency`, and `flip` commands.
   
   Please refer to the [documentation](https://vss2sn.github.io/tello/joystick_mapping.html) for details
   
   <a name="notes"></a>
   #### Notes ####
   1. Due to the asynchronous nature of the communication, the responses printed to the command might not be to the command state in the statement (for example in case the joystick was moved after a land command was sent, the statement would read `received response ok to command rc a b c d` instead of `received response ok to command land`)
   2. Travis CI is run with `cmake -DRUN_SLAM ..` with the `install_openvslam.sh` script to ensure proper builds
   
   <a name="ref"></a>
   #### References ####
   1. Joystick library - https://github.com/Notgnoshi/joystick
   2. h264decoder library - https://github.com/DaWelter/h264decoder
   3. OpenVSLAM - https://github.com/xdspacelab/openvslam
   
   <a name="pp"></a>
   #### Project paths (current & projected) ####
   Please refer to the [documentation](https://vss2sn.github.io/tello/ongoing.html) for details
   
   <a name="ts"></a>
   #### Troubleshooting ####
   Please refer to the [documentation](https://vss2sn.github.io/tello/troubleshooting.html) for details
