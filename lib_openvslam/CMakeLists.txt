project(openvslam_with_api)

cmake_minimum_required(VERSION 3.5)

add_definitions(-std=c++17)
set(LIB_OPENVSLAM_WITH_API_HEADERS ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
add_library(${PROJECT_NAME} SHARED ${CMAKE_CURRENT_SOURCE_DIR}/openvslam_api.hpp ${CMAKE_CURRENT_SOURCE_DIR}/openvslam_api.cpp)

# ==============================================================================
# If you prefer to use CMake to get and build all the dependencies thus leaving
# the bashrc file untouched, set the varaiable USE_CMAKE_NOT_SCRIPT to ON
# ==============================================================================

message(STATUS "USE_CMAKE_NOT_SCRIPT ${USE_CMAKE_NOT_SCRIPT}")

if(NOT USE_CMAKE_NOT_SCRIPT)

option(USE_DBOW2 "" ON)
add_definitions(-DUSE_DBOW2)
find_package(openvslam REQUIRED)
find_package(Pangolin REQUIRED)
set(openvslam_libs "openvslam::openvslam;openvslam::pangolin_viewer;pangolin")
target_link_libraries(${PROJECT_NAME} ${openvslam_libs})

else(NOT USE_CMAKE_NOT_SCRIPT)

message("# ====================================================================")
message("# USE_CMAKE_NOT_SCRIPT is set to ON")
message("# This will fetch and install openvslam and all its dependencies.")
message("# If you have already run the install script:")
message("# 1. set `USE_CMAKE_NOT_SCRIPT` to `OFF`")
message("# 2. In the build folder run:")
message ("      rm -rf *")
message("# And rebuild")
message("# ====================================================================")

include(ExternalProject)

set(MAIN_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external_dependencies/install")
set(MAIN_PREFIX_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external_dependencies/src")
set(OPENVSLAM_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/openvslam/install")
set(OPENVSLAM_PREFIX_DIR "${CMAKE_CURRENT_SOURCE_DIR}/openvslam/src")

if(NOT WIN32)
  string(ASCII 27 Esc)
  set(ColourReset "${Esc}[m")
  set(BoldBlue    "${Esc}[1;34m")
endif()

# Eigen
set(install_dir ${MAIN_INSTALL_DIR}/eigen-eigen-5a0156e40feb)
find_package(Eigen3 PATHS ${install_dir} NO_DEFAULT_PATH)
if(NOT Eigen3_FOUND)
  set(EXTP_EIGEN "EXTP_EIGEN")
  message(STATUS "${BoldBlue}Eigen not found. Building.${ColourReset}")
  ExternalProject_Add(
    EXTP_EIGEN
    PREFIX ${MAIN_PREFIX_DIR}/eigen-eigen-5a0156e40feb
    URL http://bitbucket.org/eigen/eigen/get/3.3.4.tar.bz2
    INSTALL_DIR ${install_dir}
    CMAKE_CACHE_ARGS
      -DCMAKE_CXX_FLAGS="-w"
      -DCMAKE_BUILD_TYPE:STRING=Release
      -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    BUILD_COMMAND make -j
    INSTALL_COMMAND make install
  )
  ExternalProject_Get_Property(EXTP_EIGEN install_dir)
  add_dependencies(${PROJECT_NAME} EXTP_EIGEN)
else()
    message(STATUS "${BoldBlue}Eigen found. ${ColourReset}")
endif()
include_directories(${install_dir}/include/eigen3)
list(APPEND CMAKE_PREFIX_PATH "${install_dir}")

# OpenCV
find_package(OpenCV 3.3
  QUIET
  COMPONENTS core imgcodecs videoio features2d calib3d highgui
)
if(NOT OpenCV_FOUND)
  find_package(OpenCV 4
    QUIET
    COMPONENTS core imgcodecs videoio features2d calib3d highgui
  )
  if(NOT OpenCV_FOUND)
    set(install_dir ${MAIN_INSTALL_DIR}/opencv-3.4.0)
    find_package(OpenCV 3.4
      QUIET
      COMPONENTS core imgcodecs videoio features2d calib3d highgui
      PATHS ${install_dir}
    )
    if(NOT OpenCV_FOUND)
      message(STATUS "${BoldBlue}OpenCV not found. Building.${ColourReset}")
      ExternalProject_Add(
        EXTP_OPENCV
        PREFIX ${install_dir}
        URL https://github.com/opencv/opencv/archive/3.4.0.zip
        INSTALL_DIR ${install_dir}
        CMAKE_CACHE_ARGS
          -DCMAKE_CXX_FLAGS="-w"
          -DCMAKE_BUILD_TYPE:STRING=Release
          -DENABLE_CXX11:BOOL=ON
          -DBUILD_DOCS:BOOL=OFF
          -DBUILD_EXAMPLES:BOOL=OFF
          -DBUILD_JASPER:BOOL=OFF
          -DBUILD_OPENEXR:BOOL=OFF
          -DBUILD_PERF_TESTS:BOOL=OFF
          -DBUILD_TESTS:BOOL=OFF
          -DWITH_EIGEN:BOOL=ON
          -DWITH_FFMPEG:BOOL=ON
          -DWITH_OPENMP:BOOL=ON
          -DBUILD_opencv_cudacodec:BOOL=OFF
          -DBUILD_opencv_apps:BOOL=ON
          -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
        BUILD_COMMAND make -j
        INSTALL_COMMAND make install
      )
      ExternalProject_Get_Property(EXTP_OPENCV install_dir)
    else()
      message(STATUS "OpenCV ${OpenCV_VERSION} found.")
    endif()
    include_directories(${install_dir}/include)
    list(APPEND CMAKE_PREFIX_PATH "${install_dir}")
  else(OpenCV_FOUND)
    message(STATUS "OpenCV Version ${OpenCV_VERSION} found")
  endif()
else(OpenCV_FOUND)
  message(STATUS "OpenCV Version ${OpenCV_VERSION} found")
endif()

# DBoW2
option(USE_DBOW2 "" ON)
add_definitions(-DUSE_DBOW2)
set(install_dir ${MAIN_INSTALL_DIR}/DBoW2)
find_package(DBoW2 PATHS ${install_dir})
if(NOT DBoW2_FOUND)
  set(EXTP_DBOW2 "EXTP_DBOW2")
  message(STATUS "${BoldBlue}DBoW2 not found. Building.${ColourReset}")
  ExternalProject_Add(
    EXTP_DBOW2
    PREFIX ${MAIN_PREFIX_DIR}/DBoW2
    GIT_REPOSITORY https://github.com/shinsumicco/DBoW2.git
    GIT_TAG e8cc74d24705d0ad61b68df4e1086d1deff67136
    INSTALL_DIR ${install_dir}
    CMAKE_CACHE_ARGS
      -DCMAKE_CXX_FLAGS="-w"
      -DCMAKE_BUILD_TYPE:STRING=Release
      -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    BUILD_COMMAND make -j
    INSTALL_COMMAND make install
  )
  ExternalProject_Get_Property(EXTP_DBOW2 install_dir)
else()
    message(STATUS "${BoldBlue}DBoW2 found. ${ColourReset}")
endif()
include_directories(${install_dir}/include)
list(APPEND CMAKE_PREFIX_PATH "${install_dir}")
# target_link_libraries( ${PROJECT_NAME} ${DBoW2_LIBRARIES})

# g2o
set(install_dir ${MAIN_INSTALL_DIR}/g2o)
find_package(g2o PATHS ${install_dir})
if(NOT g2o_FOUND)
  set(EXTP_G2O "EXTP_G2O")
  message(STATUS "${BoldBlue}g2o not found. Building.${ColourReset}")

  ExternalProject_Add(
    EXTP_G2O
    PREFIX ${MAIN_PREFIX_DIR}/g2o
    GIT_REPOSITORY https://github.com/RainerKuemmerle/g2o.git
    GIT_TAG 9b41a4ea5ade8e1250b9c1b279f3a9c098811b5a
    DEPENDS ${EXTP_EIGEN}
    INSTALL_DIR ${MAIN_INSTALL_DIR}/g2o
    CMAKE_ARGS
      -DCMAKE_CXX_FLAGS="-std=c++11"
    CMAKE_CACHE_ARGS
      -DCMAKE_CXX_FLAGS="-w"
      -DCMAKE_BUILD_TYPE:STRING=Release
      -DBUILD_SHARED_LIBS:BOOL=ON
      -DBUILD_UNITTESTS:BOOL=OFF
      -DBUILD_WITH_MARCH_NATIVE:BOOL=ON
      -DG2O_USE_CHOLMOD:BOOL=OFF
      -DG2O_USE_CSPARSE:BOOL=ON
      -DG2O_USE_OPENGL:BOOL=OFF
      -DG2O_USE_OPENMP:BOOL=ON
      -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    BUILD_COMMAND make -j
    INSTALL_COMMAND make install
  )
  ExternalProject_Get_Property(EXTP_G2O install_dir)
else()
    message(STATUS "${BoldBlue}g2o found. ${ColourReset}")
endif()
include_directories(${install_dir}/include)
list(APPEND CMAKE_PREFIX_PATH "${install_dir}")

# Pangolin
set(install_dir ${MAIN_INSTALL_DIR}/Pangolin)
find_package(Pangolin PATHS ${install_dir})
if(NOT Pangolin_FOUND)
  message(STATUS "${BoldBlue}Pangolin not found. Building.${ColourReset}")
  set(EXTP_PANGOLIN "EXTP_PANGOLIN")
  ExternalProject_Add(
    EXTP_PANGOLIN
    PREFIX ${MAIN_PREFIX_DIR}/Pangolin
    GIT_REPOSITORY https://github.com/stevenlovegrove/Pangolin.git
    GIT_TAG b6cecf4e5faaa9cf6039f98c8d263cbc169f0b5d
    INSTALL_DIR ${install_dir}
    CMAKE_CACHE_ARGS
      -DCMAKE_CXX_FLAGS="-w"
      -DCMAKE_BUILD_TYPE:STRING=Release
      -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    BUILD_COMMAND make -j
    INSTALL_COMMAND make install
  )
  ExternalProject_Get_Property(EXTP_PANGOLIN install_dir)
endif()
include_directories(${install_dir}/include)
list(APPEND CMAKE_PREFIX_PATH "${install_dir}")

# sioclient/socket.io-client-cpp
set(install_dir ${MAIN_INSTALL_DIR}/socket.io-client-cpp)
find_package(sioclient PATHS ${install_dir})
if(NOT sioclient_FOUND)
  set(EXTP_SOCKET "EXTP_SOCKET")
  message(STATUS "${BoldBlue}sioclient not found. Building.${ColourReset}")
  ExternalProject_Add(
    EXTP_SOCKET
    PREFIX ${MAIN_PREFIX_DIR}/socket.io-client-cpp
    GIT_REPOSITORY https://github.com/shinsumicco/socket.io-client-cpp.git
    GIT_TAG ff6ef08e45c594e33aa6bc19ebdd07954914efe0
    GIT_SUBMODULES
    INSTALL_DIR ${install_dir}
    CMAKE_CACHE_ARGS
      -DCMAKE_CXX_FLAGS="-w"
      -DCMAKE_BUILD_TYPE:STRING=Release
      -DBUILD_UNIT_TESTS:BOOL=OFF
      -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    BUILD_COMMAND make -j
    INSTALL_COMMAND make install
  )
  ExternalProject_Get_Property(EXTP_SOCKET install_dir)
else()
    message(STATUS "${BoldBlue}sioclient found. ${ColourReset}")
endif()
include_directories(${install_dir}/include)
list(APPEND CMAKE_PREFIX_PATH "${install_dir}")

if(NOT "${EXTP_EIGEN}" STREQUAL "" OR NOT "${EXTP_DBOW2}" STREQUAL "" OR NOT "${EXTP_G2O}" STREQUAL "" OR NOT "${EXTP_PANGOLIN}" STREQUAL "" OR NOT "${EXTEXTP_SOCKET}" STREQUAL "" OR NOT "${EXT_OPENCV}" STREQUAL "" OR ${REBUILD_OPENVSLAM})
  message(STATUS "${BoldBlue}At least one dependency missing or `REBUILD_OPENVSLAM` set to `ON`. `make install` will be run for OpenVSLAM. Building.${ColourReset}")
  string(REPLACE ";" "|" CMAKE_PREFIX_PATH_ALT_SEP "${CMAKE_PREFIX_PATH}")
  ExternalProject_Add(
    EXTP_OPENVSLAM
    PREFIX ${OPENVSLAM_PREFIX_DIR}
    GIT_REPOSITORY https://github.com/xdspacelab/openvslam.git
    GIT_TAG 9674a321f99d3c7675f6d52d6c2d27a5f9e949d7
    PATCH_COMMAND git apply ${CMAKE_CURRENT_SOURCE_DIR}/../patches/pangolin_viewer.diff.extp
    INSTALL_DIR ${OPENVSLAM_INSTALL_DIR}
    DEPENDS ${EXTP_G2O} ${EXTP_EIGEN} ${EXTP_PANGOLIN} ${EXTP_DBOW2} ${EXTP_OPENCV}
    LIST_SEPARATOR |
    CMAKE_ARGS
      -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH_ALT_SEP}
      -DCMAKE_INSTALL_RPATH_USE_LINK_PATH=TRUE
      -DCMAKE_CXX_FLAGS="-w"
    CMAKE_CACHE_ARGS
      -DBUILD_WITH_MARCH_NATIVE:BOOL=ON
      -DUSE_PANGOLIN_VIEWER:BOOL=ON
      -DINSTALL_PANGOLIN_VIEWER:BOOL=ON
      -DINSTALL_SOCKET_PUBLISHER:BOOL=OFF
      -DUSE_SOCKET_PUBLISHER:BOOL=OFF
      -DUSE_STACK_TRACE_LOGGER:BOOL=ON
      -DBOW_FRAMEWORK:STRING=DBoW2
      -DBUILD_TESTS:BOOL=ON
      -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
    BUILD_COMMAND make -j
    INSTALL_COMMAND make install
  )
  add_dependencies(${PROJECT_NAME} EXTP_OPENVSLAM)
else()
  message(STATUS "${BoldBlue}OpenVSLAM found.${ColourReset}")
endif()

include_directories(${OPENVSLAM_INSTALL_DIR}/include/openvslam/3rd/json/include)
include_directories(${OPENVSLAM_INSTALL_DIR}/include/openvslam/3rd/spdlog/include)
include_directories(${PROJECT_NAME} ${OPENVSLAM_INSTALL_DIR}/include)

target_link_libraries( ${PROJECT_NAME}
                        # Use the build directory here if you do not want to
                        # ensure that the rpath is removed fro the installed
                        # files.
                       ${OPENVSLAM_INSTALL_DIR}/lib/libopenvslam.so
                       ${OPENVSLAM_INSTALL_DIR}/lib/libpangolin_viewer.so
                       # ${OPENVSLAM_INSTALL_DIR}/lib/libsocket_publisher.so
                       ${OpenCV_LIBRARIES}
                     )
endif(NOT USE_CMAKE_NOT_SCRIPT)
