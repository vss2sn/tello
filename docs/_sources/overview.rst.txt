.. _overview:

================================================================================
Overview
================================================================================

This repository contains a C++ library that enables interaction with DJI/RYZE Tello and Tello Edu drones.

It features joystick control, autonomous command queue execution with joystick safety override, command line control input and real time SLAM (using OpenVSLAM)

This repository does not depend on ROS.

Quick start
================================================================================
.. code-block:: bash

  sudo apt install libasio-dev libopencv-dev
  git clone https://github.com/vss2sn/tello.git
  cd tello
  mkdir -p build && cd build
  cmake ..
  make -j4
  ./tello
  
