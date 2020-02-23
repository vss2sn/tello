project(tello)

cmake_minimum_required(VERSION 3.5)

add_definitions(-std=c++17)

set(CMAKE_CXX_FLAGS "-w -g -Werror -Wall -O0")
option(SIMPLE "Use simple formatting for all output to terminal" OFF)
option(USE_BOOST "Use boost libraries for asio, thread, etc" OFF)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc
${CMAKE_CURRENT_SOURCE_DIR}/lib_h264decoder)

file( GLOB LIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file( GLOB LIB_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/inc/*.hpp)

list(REMOVE_ITEM LIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

if(SIMPLE)
  add_definitions(-DSIMPLE)
endif(SIMPLE)

if(USE_BOOST)
  find_package(Boost COMPONENTS system thread REQUIRED)
  add_definitions(-DUSE_BOOST)
endif(USE_BOOST)

find_package(Threads REQUIRED)
find_package(OpenCV REQUIRED)

add_library( lib_h264decoder OBJECT
            ${CMAKE_CURRENT_SOURCE_DIR}/lib_h264decoder/h264decoder.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/lib_h264decoder/h264decoder.hpp
            )

add_executable(${PROJECT_NAME}
              ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
              ${LIB_SOURCES}
              ${LIB_HEADERS}
              $<TARGET_OBJECTS:lib_h264decoder>
              )

target_link_libraries(${PROJECT_NAME}
                      ${Boost_SYSTEM_LIBRARIES}
                      ${Boost_THREAD_LIBRARY}
                      Threads::Threads
                      ${OpenCV_LIBRARIES}
                      avcodec
                      avutil
                      swscale
                    )
