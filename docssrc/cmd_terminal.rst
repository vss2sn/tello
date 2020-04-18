.. cmd_terminal:

================================================================================
Command Terminal
================================================================================

To use the command terminal

* To send a command to the drone, the command format is "<SDK command>" (eg: land)
* To modify the queue via the terminal, the command format is "queue <function> <command>" where:

  | <function> : add, start, stop, addfront, clear, removenext, allowautoland, donotautoland
  | <command>  : (required argument for some, not all, of the above) <SDK command>
  | eg: ``queue start``, ``queue addfront takeoff``
