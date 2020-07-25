#!/bin/bash
echo "----------------------------------------"
echo "This script installs OpenVSLAM and its dependencies"
echo ""
echo "The default installation creates the following structure: "
echo ""
echo "INSTALL_SCRIPT__DIR"
echo "     |- lib_openvslam"
echo "     |-     |- external_dependencies"
echo "     |-     |-     |- <dependecy>"
echo "     |-     |-     |-     |- build"
echo "     |-     |-     |-     |- <other folders within dependency>"
echo "     |-     |-     |- install"
echo "     |-     |-     |-     |- <dependecy>"
echo "     |-     |-     |-     |- lib"
echo "     |-     |-     |-     |- include"
echo "     |-     |-     |-     |- <other folders installed, example: bin>"
echo "     |-     |- openvslam"
echo "     |-     |-     |- install"
echo "     |-     |-     |-     |- lib"
echo "     |-     |-     |-     |- include"
echo "     |-     |-     |- <other folders in openvslam>"
echo ""
echo "The install locations for all the dependencies are appended to a variable"
echo "$(tput bold)CMAKE_PREFIX_PATH $(tput sgr0) and added to ~/.bashrc."
echo "The runtime locations for all the dependencies are appended to a variable"
echo "$(tput bold)LD_LIBRARY_PATH $(tput sgr0) and added to ~/.bashrc."
echo "This ensures that removing the build and install directory and the added"
echo "line in the .bashrc file effectively uninstalls the dependencies."
echo "It also makes managing different versions of these libraries easier."
echo "----------------------------------------"
echo ""

export MAIN_DIR=$(pwd)

#Begin openvslam dependencies
mkdir -p lib/openvslam
cd lib/openvslam

git clone https://github.com/xdspacelab/openvslam.git

mkdir external_dependencies
cd external_dependencies
export EXT_DEP=$(pwd)

# echo "External dependecies install path: ${EXT_DEP}"
# echo ""
#
# sudo echo "# Paths added for openvslam" >> ~/.bashrc
#
# sudo apt update -y
# sudo apt upgrade -y --no-install-recommends
# # basic dependencies
# sudo apt install -y build-essential pkg-config cmake git wget curl unzip
# # g2o dependencies
# sudo apt install -y libatlas-base-dev libsuitesparse-dev
# # OpenCV dependencies
# sudo apt install -y libgtk-3-dev
# sudo apt install -y ffmpeg
# sudo apt install -y libavcodec-dev libavformat-dev libavutil-dev libswscale-dev libavresample-dev
# # eigen dependencies
# sudo apt install -y gfortran
# # other dependencies
# sudo apt install -y libyaml-cpp-dev libgoogle-glog-dev libgflags-dev
#
# # (if you plan on using PangolinViewer)
# # Pangolin dependencies
# sudo apt install -y libglew-dev
#
# # (if you plan on using SocketViewer)
# # Protobuf dependencies
# sudo apt install -y autogen autoconf libtool
# curl -sL https://deb.nodesource.com/setup_6.x | sudo -E bash -
# sudo apt install -y nodejs
#
# echo "----------------------------------------"
# echo "Installing Eigen 3.3.4"
# echo "----------------------------------------"
#
# cd ${EXT_DEP}
# wget -q https://gitlab.com/libeigen/eigen/-/archive/3.3.4/eigen-3.3.4.tar.bz2
# tar xf eigen-3.3.4.tar.bz2
# rm -rf eigen-3.3.4.tar.bz2
# cd eigen-3.3.4
# mkdir -p build && cd build
# cmake \
#     -DCMAKE_BUILD_TYPE=Release \
#     -DCMAKE_INSTALL_PREFIX=${EXT_DEP}/install/eigen-3.3.4/ \
#     ..
# make -j2 --silent
# make install
# cd ${EXT_DEP}/install/eigen-3.3.4/
# sudo echo "export CMAKE_PREFIX_PATH=\$CMAKE_PREFIX_PATH:$(pwd)" >> ~/.bashrc
# sudo echo "export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:$(pwd)/lib" >> ~/.bashrc
#
# echo "----------------------------------------"
# echo "Sourcing bashrc"
# echo "----------------------------------------"
# source ~/.bashrc
#
# echo "----------------------------------------"
# echo "Installing OpenCV 3.4"
# echo "----------------------------------------"
# cd ${EXT_DEP}
# wget -q https://github.com/opencv/opencv/archive/3.4.0.zip
# unzip -q 3.4.0.zip
# rm -rf 3.4.0.zip
# cd opencv-3.4.0
# mkdir -p build && cd build
# cmake \
#     -DCMAKE_BUILD_TYPE=Release \
#     -DCMAKE_INSTALL_PREFIX=${EXT_DEP}/install/opencv-3.4.0/ \
#     -DENABLE_CXX11=ON \
#     -DBUILD_DOCS=OFF \
#     -DBUILD_EXAMPLES=OFF \
#     -DBUILD_JASPER=OFF \
#     -DBUILD_OPENEXR=OFF \
#     -DBUILD_PERF_TESTS=OFF \
#     -DBUILD_TESTS=OFF \
#     -DWITH_EIGEN=ON \
#     -DWITH_FFMPEG=ON \
#     -DWITH_OPENMP=ON \
#     -DBUILD_opencv_cudacodec=OFF \
#     -DWITH_CUDA=OFF \
#     -DWITH_CUFFT=OFF \
#     -DWITH_CUBLAS=OFF \
#     -DBUILD_CUDA_STUBS=OFF \
#     -DCMAKE_CXX_FLAGS="-w" \
#     ..
# make -j2 --silent
# make install
# cd ${EXT_DEP}/install/opencv-3.4.0
# sudo echo "export CMAKE_PREFIX_PATH=\$CMAKE_PREFIX_PATH:$(pwd)" >> ~/.bashrc
# sudo echo "export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:$(pwd)/lib" >> ~/.bashrc
#
# echo "----------------------------------------"
# echo "Sourcing bashrc"
# echo "----------------------------------------"
# source ~/.bashrc
#
# echo "----------------------------------------"
# echo "Installing DBoW2"
# echo "----------------------------------------"
# cd ${EXT_DEP}
# git clone https://github.com/shinsumicco/DBoW2.git
# cd DBoW2
# mkdir build
# cd build
# cmake \
#     -DCMAKE_BUILD_TYPE=Release \
#     -DCMAKE_INSTALL_PREFIX=${EXT_DEP}/install/DBoW2/ \
#     -DCMAKE_CXX_FLAGS="-w" \
#     ..
# make -j2 --silent
# make install
# cd ${EXT_DEP}/install/DBoW2/
# sudo echo "export CMAKE_PREFIX_PATH=\$CMAKE_PREFIX_PATH:$(pwd)" >> ~/.bashrc
# sudo echo "export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:$(pwd)/lib" >> ~/.bashrc
#
# echo "----------------------------------------"
# echo "Sourcing bashrc"
# echo "----------------------------------------"
# source ~/.bashrc
#
# echo "----------------------------------------"
# echo "Installing g2o"
# echo "----------------------------------------"
# cd ${EXT_DEP}
# git clone https://github.com/RainerKuemmerle/g2o.git
# cd g2o
# git checkout 9b41a4ea5ade8e1250b9c1b279f3a9c098811b5a
# mkdir build
# cd build
# cmake \
#     -DCMAKE_BUILD_TYPE=Release \
#     -DCMAKE_INSTALL_PREFIX=${EXT_DEP}/install/g2o/ \
#     -DCMAKE_CXX_FLAGS=-std=c++11 \
#     -DBUILD_SHARED_LIBS=ON \
#     -DBUILD_UNITTESTS=OFF \
#     -DBUILD_WITH_MARCH_NATIVE=ON \
#     -DG2O_USE_CHOLMOD=OFF \
#     -DG2O_USE_CSPARSE=ON \
#     -DG2O_USE_OPENGL=OFF \
#     -DG2O_USE_OPENMP=ON \
#     -DBUILD_opencv_apps=ON \
#     -DCMAKE_CXX_FLAGS="-w" \
#     ..
# make -j2 --silent
# make install
# cd ${EXT_DEP}/install/g2o/
# sudo echo "export CMAKE_PREFIX_PATH=\$CMAKE_PREFIX_PATH:$(pwd)" >> ~/.bashrc
# sudo echo "export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:$(pwd)/lib" >> ~/.bashrc
#
# echo "----------------------------------------"
# echo "Sourcing bashrc"
# echo "----------------------------------------"
# source ~/.bashrc
#
# echo "----------------------------------------"
# echo "Installing Pangolin"
# echo "----------------------------------------"
# cd ${EXT_DEP}
# git clone https://github.com/stevenlovegrove/Pangolin.git
# cd Pangolin
# git checkout ad8b5f83222291c51b4800d5a5873b0e90a0cf81
# mkdir build
# cd build
# cmake \
#     -DCMAKE_BUILD_TYPE=Release \
#     -DCMAKE_INSTALL_PREFIX=${EXT_DEP}/install/Pangolin/ \
#     -DCMAKE_CXX_FLAGS="-w" \
#     ..
# make -j2 --silent
# make install
# cd ${EXT_DEP}/install/Pangolin/
# sudo echo "export CMAKE_PREFIX_PATH=\$CMAKE_PREFIX_PATH:$(pwd)" >> ~/.bashrc
# sudo echo "export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:$(pwd)/lib" >> ~/.bashrc
#
# echo "----------------------------------------"
# echo "Sourcing bashrc"
# echo "----------------------------------------"
# source ~/.bashrc
#
# echo "----------------------------------------"
# echo "Installing socket.io-client-cpp"
# echo "----------------------------------------"
# cd ${EXT_DEP}
# git clone https://github.com/shinsumicco/socket.io-client-cpp
# cd socket.io-client-cpp
# git submodule init
# git submodule update
# mkdir build
# cd build
# cmake \
#     -DCMAKE_BUILD_TYPE=Release \
#     -DCMAKE_INSTALL_PREFIX=${EXT_DEP}/install/socket.io-client-cpp/ \
#     -DBUILD_UNIT_TESTS=OFF \
#     -DCMAKE_CXX_FLAGS="-w" \
#     ..
# make -j2 --silent
# make install
# cd ${EXT_DEP}/install/socket.io-client-cpp/
# sudo echo "export CMAKE_PREFIX_PATH=\$CMAKE_PREFIX_PATH:$(pwd)" >> ~/.bashrc
# sudo echo "export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:$(pwd)/lib" >> ~/.bashrc
#
# sudo apt install -y libprotobuf-dev protobuf-compiler

echo "----------------------------------------"
echo "Sourcing bashrc"
echo "----------------------------------------"
source ~/.bashrc

echo "----------------------------------------"
echo "Installing OpenVSLAM"
echo "----------------------------------------"

cd ${MAIN_DIR}
git apply ${MAIN_DIR}/patches/pangolin_viewer.diff

cd ${EXT_DEP}/../openvslam

mkdir build
cd build

cmake \
    -DBUILD_WITH_MARCH_NATIVE=ON \
    -DUSE_PANGOLIN_VIEWER=ON \
    -DINSTALL_PANGOLIN_VIEWER=ON \
    -DUSE_SOCKET_PUBLISHER=OFF \
    -DUSE_STACK_TRACE_LOGGER=ON \
    -DBOW_FRAMEWORK=DBoW2 \
    -DBUILD_TESTS=ON \
    -DCMAKE_INSTALL_PREFIX=$(pwd)/../install/ \
    -DCMAKE_CXX_FLAGS="-w" \
    ..
make -j2 --silent
make install
cd ../install
sudo echo "export CMAKE_PREFIX_PATH=\$CMAKE_PREFIX_PATH:$(pwd)" >> ~/.bashrc
sudo echo "export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:$(pwd)/lib" >> ~/.bashrc

sudo echo "# End paths added for openvslam" >> ~/.bashrc
