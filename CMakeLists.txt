project(tello)

cmake_minimum_required(VERSION 3.5)

add_definitions(-std=c++17)

set(CMAKE_CXX_FLAGS "-w -g -Werror -Wall -O0")
option(SIMPLE "Use simple formatting for all output to terminal" ON)
option(USE_BOOST "Use boost libraries for asio, thread, etc" OFF)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc
                    ${CMAKE_CURRENT_SOURCE_DIR}/lib_h264decoder
                    ${CMAKE_CURRENT_SOURCE_DIR}/lib_joystick
                    ${CMAKE_CURRENT_SOURCE_DIR}/lib_utils
                   )

file( GLOB LIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file( GLOB LIB_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/inc/*.hpp)

file( GLOB LIB_SOURCES_JOYSTICK ${CMAKE_CURRENT_SOURCE_DIR}/lib_joystick/*.cpp)
file( GLOB LIB_HEADERS_JOYSTICK ${CMAKE_CURRENT_SOURCE_DIR}/lib_joystick/*.h)

if(SIMPLE)
  add_definitions(-DSIMPLE)
endif(SIMPLE)

if(USE_BOOST)
  find_package(Boost COMPONENTS system thread REQUIRED)
  add_definitions(-DUSE_BOOST)
endif(USE_BOOST)

find_package(Threads REQUIRED)
find_package(OpenCV REQUIRED)

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

# target_link_libraries(joystick utils)

add_executable( ${PROJECT_NAME}
                ${CMAKE_CURRENT_SOURCE_DIR}/main/main.cpp
                ${LIB_SOURCES}
                ${LIB_HEADERS}
              )

target_link_libraries( ${PROJECT_NAME}
                       ${Boost_SYSTEM_LIBRARIES} # Will be empty string if not using boost
                       ${Boost_THREAD_LIBRARY} # Will be empty string if not using boost
                       Threads::Threads
                       ${OpenCV_LIBRARIES}
                       h264decoder
                       joystick
                       utils
                     )
