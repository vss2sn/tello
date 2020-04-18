.. commandcontrolmethods:

================================================================================
Command/control methods
================================================================================
  * Using a joystick
  * Autonomously using a command queue
  * Command line interface

Joystick mode
^^^^^^^^^^^^^
  * The joystick is always active (though it's use can be safely commented out)
  * Commands from the joystick are continuously sent to the drone without waiting for a response
  * Based on the controller used, the mappings might require modification
  * The default controller is gameSirT1s; key/axis to value mappings for PS3 and 360 controller have been provided, though the mappings between the values to the commands sent to the drone might need modification

Autonomous mode
^^^^^^^^^^^^^^^
  * This queue is always overridden by the joystick as a safety measure
  * Using the joystick while the queue is executing pauses the queue, which will need to be restarted even when there is no longer additional input from the joystick
  * Queue execution can be safely paused and resumed
  * Commands can be dynamically added
  * This mode enables (optional) command retries when a command does not receive any response from the drone

Command line interface
^^^^^^^^^^^^^^^^^^^^^^
  * A command line interface can be brought up by setting the CMake option ``USE_TERMINAL`` to ``ON``
  * Like the joystick, the CLI will pause command queue execution

SLAM integration has been provided using the OpenVSLAM library.
