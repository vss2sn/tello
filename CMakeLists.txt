project(tello)

cmake_minimum_required(VERSION 3.5)

add_definitions(-std=c++17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -DDEBUG")

# These should be the only options the user requires

option(SIMPLE "Use simple formatting for all output to terminal" OFF)
option(USE_JOYSTICK "Use a joystick/controller to control the drone manually" ON)
option(RECORD "Record tello video feed" ON)
option(RUN_SLAM "Run SLAM in real time" ON)
option(USE_TERMINAL "Run with terminal for CLI" ON)
option(USE_CONFIG "Use configuration file to set up tello(s)" ON)

message(STATUS "CMake Option `SIMPLE` - Use simple formatting for all output to terminal ${SIMPLE}")
message(STATUS "CMake Option `USE_JOYSTICK` - Use a joystick/controller to control the drone manually ${USE_JOYSTICK}")
message(STATUS "CMake Option `RECORD` - Record tello video feed ${RECORD}")
message(STATUS "CMake Option `RUN_SLAM` - Run SLAM in real time ${RUN_SLAM}")
message(STATUS "CMake Option `USE_TERMINAL` - `Run with terminal for CLI` ${USE_TERMINAL}")
message(STATUS "CMake_Option `USE_CONFIG` - Use configuration file to set up tello(s) ${USE_CONFIG}")

# End These should be the only options the user requires

option(USE_CMAKE_NOT_SCRIPT "Use Cmake ExternalProject to get and build OpenVSLAM and its dependencies" OFF)
option(REBUILD_OPENVSLAM "Rebuild OpenVSLAM" OFF)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc
                    ${CMAKE_CURRENT_SOURCE_DIR}/lib_h264decoder
                    ${CMAKE_CURRENT_SOURCE_DIR}/lib_joystick
                    ${CMAKE_CURRENT_SOURCE_DIR}/lib_utils
                    ${CMAKE_CURRENT_SOURCE_DIR}/lib_openvslam/openvslam/src
                   )

file( GLOB LIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file( GLOB LIB_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/inc/*.hpp)

file( GLOB LIB_SOURCES_JOYSTICK ${CMAKE_CURRENT_SOURCE_DIR}/lib_joystick/*.cpp)
file( GLOB LIB_HEADERS_JOYSTICK ${CMAKE_CURRENT_SOURCE_DIR}/lib_joystick/*.h)

if(SIMPLE)
  add_definitions(-DSIMPLE)
endif(SIMPLE)

add_definitions(-DASIO_STANDALONE)
# __cplusplus macro not being set corretly in Travis CI,
# If __cplusplus >  2011... asio shoould be able to compile without boost.
# Succeeds locally but fails in travis CI.
# TODO: Debug this.

if(USE_JOYSTICK)
  add_definitions(-DUSE_JOYSTICK)
endif(USE_JOYSTICK)

if(RECORD)
  add_definitions(-DRECORD)
endif(RECORD)

if(USE_TERMINAL)
  add_definitions(-DUSE_TERMINAL)
endif(USE_TERMINAL)

if(USE_CONFIG)
  find_package(yaml-cpp REQUIRED)
  add_definitions(-DUSE_CONFIG)
endif(USE_CONFIG)

find_package(Threads REQUIRED)

add_library( h264decoder SHARED
            ${CMAKE_CURRENT_SOURCE_DIR}/lib_h264decoder/h264decoder.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/lib_h264decoder/h264decoder.hpp
            )

target_link_libraries(h264decoder
                      avcodec
                      avutil
                      swscale)

add_library( utils SHARED
             ${CMAKE_CURRENT_SOURCE_DIR}/lib_utils/utils.cpp
             ${CMAKE_CURRENT_SOURCE_DIR}/lib_utils/utils.hpp
           )

add_library( joystick SHARED
             ${LIB_SOURCES_JOYSTICK}
             ${LIB_HEADERS_JOYSTICK}
           )

add_executable( ${PROJECT_NAME}
                ${CMAKE_CURRENT_SOURCE_DIR}/main/main.cpp
                ${LIB_SOURCES}
                ${LIB_HEADERS}
              )

if(RUN_SLAM)
  add_definitions(-DRUN_SLAM)
  add_subdirectory(lib_openvslam)
  add_dependencies( ${PROJECT_NAME} openvslam_with_api )
  include_directories(${LIB_OPENVSLAM_WITH_API_HEADERS})
  target_link_libraries( ${PROJECT_NAME} openvslam_with_api )
else(RUN_SLAM)
  find_package(OpenCV REQUIRED COMPONENTS core imgcodecs videoio features2d calib3d highgui)
  target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBRARIES})
endif(RUN_SLAM)

if(USE_CONFIG)
  target_link_libraries( ${PROJECT_NAME} yaml-cpp)
endif(USE_CONFIG)

target_link_libraries( ${PROJECT_NAME}
                       Threads::Threads
                       h264decoder
                       joystick
                       utils
                     )
