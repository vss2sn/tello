#!/bin/bash

#Begin openvslam dependencies
mkdir lib_openvslam
cd lib_openvslam
mkdir external_dependencies
cd external_dependencies
export EXT_DEP=$(pwd)

echo "External dependacies install path: ${EXT_DEP}"

sudo echo "# Paths added for openvslam" >> ~/.bashrc

sudo apt update -y
sudo apt upgrade -y --no-install-recommends
# basic dependencies
sudo apt install -y build-essential pkg-config cmake git wget curl unzip
# g2o dependencies
sudo apt install -y libatlas-base-dev libsuitesparse-dev
# OpenCV dependencies
sudo apt install -y libgtk-3-dev
sudo apt install -y ffmpeg
sudo apt install -y libavcodec-dev libavformat-dev libavutil-dev libswscale-dev libavresample-dev
# eigen dependencies
sudo apt install -y gfortran
# other dependencies
sudo apt install -y libyaml-cpp-dev libgoogle-glog-dev libgflags-dev

# (if you plan on using PangolinViewer)
# Pangolin dependencies
sudo apt install -y libglew-dev

# (if you plan on using SocketViewer)
# Protobuf dependencies
sudo apt install -y autogen autoconf libtool
curl -sL https://deb.nodesource.com/setup_6.x | sudo -E bash -
sudo apt install -y nodejs


echo "----------------------------------------"
echo "Installing Eigen 3.3.4"
echo "----------------------------------------"

cd ${EXT_DEP}
wget -q http://bitbucket.org/eigen/eigen/get/3.3.4.tar.bz2
tar xf 3.3.4.tar.bz2
rm -rf 3.3.4.tar.bz2
cd eigen-eigen-5a0156e40feb
mkdir -p build && cd build
cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=$(pwd)/../install/ \
    ..
make -j4
make install
cd ../install
sudo echo "export CMAKE_PREFIX_PATH=\$CMAKE_PREFIX_PATH:$(pwd)" >> ~/.bashrc

echo "----------------------------------------"
echo "Installing OpenCV 3.4"
echo "----------------------------------------"
cd ${EXT_DEP}
wget -q https://github.com/opencv/opencv/archive/3.4.0.zip
unzip -q 3.4.0.zip
rm -rf 3.4.0.zip
cd opencv-3.4.0
mkdir -p build && cd build
cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=$(pwd)/../install/ \
    -DENABLE_CXX11=ON \
    -DBUILD_DOCS=OFF \
    -DBUILD_EXAMPLES=OFF \
    -DBUILD_JASPER=OFF \
    -DBUILD_OPENEXR=OFF \
    -DBUILD_PERF_TESTS=OFF \
    -DBUILD_TESTS=OFF \
    -DWITH_EIGEN=ON \
    -DWITH_FFMPEG=ON \
    -DWITH_OPENMP=ON \
    ..
make -j4
make install
cd ../install
sudo echo "export CMAKE_PREFIX_PATH=\$CMAKE_PREFIX_PATH:$(pwd)" >> ~/.bashrc

echo "----------------------------------------"
echo "Installing DBoW2"
echo "----------------------------------------"
cd ${EXT_DEP}
git clone https://github.com/shinsumicco/DBoW2.git
cd DBoW2
mkdir build && cd build
cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=$(pwd)/../install/ \
    ..
make -j4
make install
cd ../install
sudo echo "export CMAKE_PREFIX_PATH=\$CMAKE_PREFIX_PATH:$(pwd)" >> ~/.bashrc

echo "----------------------------------------"
echo "Installing g2o"
echo "----------------------------------------"
cd ${EXT_DEP}
git clone https://github.com/RainerKuemmerle/g2o.git
cd g2o
git checkout 9b41a4ea5ade8e1250b9c1b279f3a9c098811b5a
mkdir build && cd build
cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=$(pwd)/../install/ \
    -DCMAKE_CXX_FLAGS=-std=c++11 \
    -DBUILD_SHARED_LIBS=ON \
    -DBUILD_UNITTESTS=OFF \
    -DBUILD_WITH_MARCH_NATIVE=ON \
    -DG2O_USE_CHOLMOD=OFF \
    -DG2O_USE_CSPARSE=ON \
    -DG2O_USE_OPENGL=OFF \
    -DG2O_USE_OPENMP=ON \
    ..
make -j4
make install
cd ../install
sudo echo "export CMAKE_PREFIX_PATH=\$CMAKE_PREFIX_PATH:$(pwd)" >> ~/.bashrc

echo "----------------------------------------"
echo "Installing Pangolin"
echo "----------------------------------------"
cd ${EXT_DEP}
git clone https://github.com/stevenlovegrove/Pangolin.git
cd Pangolin
git checkout ad8b5f83222291c51b4800d5a5873b0e90a0cf81
mkdir build && cd build
cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=$(pwd)/../install/ \
    ..
make -j4
make install
cd ../install
sudo echo "export CMAKE_PREFIX_PATH=\$CMAKE_PREFIX_PATH:$(pwd)" >> ~/.bashrc

echo "----------------------------------------"
echo "Installing socket.io-client-cpp"
echo "----------------------------------------"
cd ${EXT_DEP}
git clone https://github.com/shinsumicco/socket.io-client-cpp
cd socket.io-client-cpp
git submodule init
git submodule update
mkdir build && cd build
cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=$(pwd)/../install/ \
    -DBUILD_UNIT_TESTS=OFF \
    ..
make -j4
make install
cd ../install
sudo echo "export CMAKE_PREFIX_PATH=\$CMAKE_PREFIX_PATH:$(pwd)" >> ~/.bashrc

sudo echo "# End paths added for openvslam" >> ~/.bashrc

sudo apt install -y libprotobuf-dev protobuf-compiler

echo "----------------------------------------"
echo "Installing OpenVSLAM"
echo "----------------------------------------"
cd ${EXT_DEP}/../
git clone https://github.com/xdspacelab/openvslam.git
cd openvslam
mkdir build && cd build

source ~/.bashrc

cmake \
    -DBUILD_WITH_MARCH_NATIVE=ON \
    -DUSE_PANGOLIN_VIEWER=ON \
    -DUSE_SOCKET_PUBLISHER=OFF \
    -DUSE_STACK_TRACE_LOGGER=ON \
    -DBOW_FRAMEWORK=DBoW2 \
    -DBUILD_TESTS=ON \
    ..
make -j4
