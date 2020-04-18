.. _building:

================================================================================
Building the code
================================================================================

Dependencies
^^^^^^^^^^^^

Required dependencies:
  - ASIO
  - OpenCV

Optional Dependencies for SLAM:
  - OpenVSLAM
  - Eigen3
  - g2o
  - DBoW2
  - OpenCV >= 3.4
  - Pangolin

Other Optional Dependencies:
  - yaml-cpp # For a creating drone from a config file
  - xterm # For opening an xterm for command line input

Installing dependencies
^^^^^^^^^^^^^^^^^^^^^^^
.. code-block:: bash

  sudo apt install libasio-dev libopencv-dev # Required dependencies
  bash -i install_openvslam.sh # optional dependencies for OpenVSLAM, which can also be installed using CMake option as well
  sudo apt install yaml-cpp xterm # other optional dependencies

Installing OpenVSLAM and its dependencies (optional)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

SLAM integration has been provided using the OpenVSLAM library

There are 2 options provided to build and install OpenVSLAM and its dependencies.

Option 1: Install script
  - Running the install script will build and install OpenVLAM and its dependencies in a directory (``lib_openvslam``) and add lines in ``~/.bashrc`` to append the appropriate paths to ``CMAKE_PREFIX_PATH`` and ``LD_LIBRARY_PATH``
  - Please run the script using ``bash -i install_openvslam`` as it needs to be run interactively for corerctly sourcing the updated ``.bashrc`` file
  - To build the code with SLAM functionality enabled, run ``cmake`` with the option ``RUN_SLAM`` set to ON (``cmake -DRUN_SLAM=ON ..``)

.. important::
  If the default install location of the script is changed, please ensure that the patch file is edited so OpenVSLAM is patched correctly

.. note::
  This option will make OpenVSLAM and its dependencies available to all other projects, which can be prevented by commenting out the lines between ``# Begin paths added for openvslam`` and ``# End paths added for openvslam`` when compiling other code

Option 2: CMake option

  - Install dependencies
  .. code-block:: bash

    sudo apt update -y
    sudo apt install -y build-essential pkg-config cmake git wget curl unzip
    sudo apt install -y libatlas-base-dev libsuitesparse-dev
    sudo apt install -y libgtk-3-dev
    sudo apt install -y ffmpeg
    sudo apt install -y libavcodec-dev libavformat-dev libavutil-dev libswscale-dev libavresample-dev
    sudo apt install -y gfortran
    sudo apt install -y libyaml-cpp-dev libgoogle-glog-dev libgflags-dev
    sudo apt install -y libglew-dev

  - Run ``cmake`` with the options ``RUN_SLAM`` and ``USE_CMAKE_NOT_SCRIPT`` set to ``ON`` (``cmake -DRUN_SLAM=ON -DUSE_CMAKE_NOT_SCRIPT=ON ..``)
  - This will fetch and install all the relevant packages
  - Please make sure you install all the apt packages that are dependencies of OpenVSLAM (refer to L51-L64 of ``install_openvslam.sh``)

.. note::
  This will only allow the libraries build to be accessible by this project and by no other. This can be changed by adding the appropriate paths to ~/.bashrc


Building the code
^^^^^^^^^^^^^^^^^

To build and run the code:

.. code-block:: bash

  git clone https://github.com/vss2sn/tello.git
  cd tello
  mkdir -p build && cd build
  cmake ..
  make -j4
  ./tello

To build and run with SLAM:

.. note::
  Please download a sample vocabulary file `here <https://drive.google.com/open?id=1wUPb328th8bUqhOk-i8xllt5mgRW4n84>`_ and store it in the main directory

.. code-block:: bash

  git clone https://github.com/vss2sn/tello.git
  cd tello
  # Install SLAM here using ``bash -i install_openvslam`` OR run the cmake command below with -DUSE_CMAKE_NOT_SCRIPT
  mkdir -p build && cd build
  cmake -DRUN_SLAM ..
  make -j4
  ./tello
