.. cmakeoptions:

================================================================================
CMake Options
================================================================================

1. ``SIMPLE``

  - Default ``OFF``
  - When set to ``OFF`` the output to the screen will also print the line number and file number of the print statement as well as the log level (STATUS, DEBUG, INFO, WARN, ERR).

2. ``USE_JOYSTICK``

  - Default ``ON``
  - When set to ``ON`` allows the use of a joystick to control the drone and exits if a joystick is not found at startup.

3. ``RECORD``

  - Default ``OFF``
  - When set to ``ON`` records the video

4. ``RUN_SLAM``

  - Default ``OFF``
  - When set to ``ON`` runs OpenVSLAM, creating a map of the area and localizing the drone

5. ``USE_TERMINAL``

  - DEFAULT ``OFF``
  - When set to ``ON`` opens up an xterm that takes in lines and sends them as commands to the Tello (example: ``command`` , ``takeoff``, etc)

6. ``USE_CONFIG``

  - Default ``OFF``
  - When set to ``ON`` uses the config manager to create the Tello from the config file ``config.yaml``
