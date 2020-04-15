.. SLAMnotes:

================================================================================
Notes on running with SLAM
================================================================================

Requirements
================================================================================
- To run with SLAM please install OpenVLAM as mentioned in the :ref:`build instructions <building>`
- Requires a configuration file for the camera as well as an ORB vocabulary file
- A camera configuration file (``config.yaml``)
- Please download a sample vocabulary file `here <https://drive.google.com/open?id=1wUPb328th8bUqhOk-i8xllt5mgRW4n84>`_ and store it in the main directory
- Please copy the sample config.yaml file into the build directory

Calibrating the tello camera
================================================================================
- To get an accurate camera calibration file, please run the main executable (``tello``), built with the CMake Option ``RECORD`` set to ON
- Move a chessboard pattern (similar to the one used in OpenCV calibration examples) as the video is being recorded
- Run the python script util_scripts (``calibrate_camera_from_video.py``) with the requisite options to provide this video as input
- Replace the constants (k1, k2, k3, p1 and p2) with the ones obtained from the script
