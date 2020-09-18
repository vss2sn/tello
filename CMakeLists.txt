project(tello)

cmake_minimum_required(VERSION 3.5)

# if(${CMAKE_VERSION} VERSION_LESS 3.5)
#     cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
# else()
#     cmake_policy(VERSION 3.5)
# endif()

# These should be the only options the user requires

option(SIMPLE "Use simple formatting for all output to terminal" OFF)
option(USE_JOYSTICK "Use a joystick/controller to control the drone manually" ON)
option(RUN_SLAM "Run SLAM in real time" OFF)
option(USE_TERMINAL "Run with terminal for CLI" OFF)
option(USE_CONFIG "Use configuration file to set up tello(s)" OFF)

message(STATUS "CMake Option `SIMPLE` - Use simple formatting for all output to terminal ${SIMPLE}")
message(STATUS "CMake Option `USE_JOYSTICK` - Use a joystick/controller to control the drone manually ${USE_JOYSTICK}")
message(STATUS "CMake Option `RUN_SLAM` - Run SLAM in real time ${RUN_SLAM}")
message(STATUS "CMake Option `USE_TERMINAL` - `Run with terminal for CLI` ${USE_TERMINAL}")
message(STATUS "CMake_Option `USE_CONFIG` - Use configuration file to set up tello(s) ${USE_CONFIG}")

# End These should be the only options the user requires

option(USE_CMAKE_NOT_SCRIPT "Use Cmake ExternalProject to get and build OpenVSLAM and its dependencies" OFF)
option(REBUILD_OPENVSLAM "Rebuild OpenVSLAM" OFF)

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

if(USE_TERMINAL)
  add_definitions(-DUSE_TERMINAL)
endif(USE_TERMINAL)

if(USE_CONFIG)
  find_package(yaml-cpp REQUIRED)
  add_definitions(-DUSE_CONFIG)
endif(USE_CONFIG)

if(RUN_SLAM)
  add_definitions(-DRUN_SLAM)
endif(RUN_SLAM)

add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/main/main.cpp)

set_target_properties(${PROJECT_NAME}
  PROPERTIES
    CXX_STANDARD "17"
    CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O3 -Wall -Werror"
)

find_package(OpenCV REQUIRED COMPONENTS core imgcodecs videoio features2d calib3d highgui)
find_package(Threads REQUIRED)
add_subdirectory(lib)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc/)

target_link_libraries( ${PROJECT_NAME} PUBLIC
                       ${OpenCV_LIBRARIES}
                       Threads::Threads
                       h264decoder
                       joystick
                       utils
                     )
if(RUN_SLAM)
  target_link_libraries( ${PROJECT_NAME} PUBLIC slam_api )
endif(RUN_SLAM)

if(USE_CONFIG)
  target_link_libraries( ${PROJECT_NAME} PUBLIC yaml-cpp)
endif(USE_CONFIG)

file( GLOB LIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
target_sources(${PROJECT_NAME} PRIVATE ${LIB_SOURCES})
