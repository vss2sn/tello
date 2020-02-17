project(tello)

cmake_minimum_required(VERSION 3.5)

add_definitions(-std=c++11 -DSIMPLE)

set(CMAKE_CXX_FLAGS "-w -g -Werror -Wall -O0")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)

file( GLOB LIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file( GLOB LIB_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/inc/*.hpp)

list(REMOVE_ITEM LIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

find_package(Boost COMPONENTS system thread REQUIRED)
find_package(Threads REQUIRED)
add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
               ${LIB_SOURCES} ${LIB_HEADERS})

target_link_libraries(${PROJECT_NAME} ${Boost_SYSTEM_LIBRARIES} ${Boost_THREAD_LIBRARY} Threads::Threads)
