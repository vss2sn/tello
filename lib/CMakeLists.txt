project(lib)

cmake_minimum_required(VERSION 3.5)

add_subdirectory(h264decoder)
add_subdirectory(utils)
add_subdirectory(joystick)
if(RUN_SLAM)
  add_definitions(-DRUN_SLAM)
  add_subdirectory(slam_api)
endif(RUN_SLAM)
