.. code_overview:

#### Code overview ####
1. `BaseSocket` is an abstract class providing the framework for the other sockets
2. `VideoSocket` is a class that connects to the video streaming port of the tello
3. `StateSocket` is a class that connects to the port where the state of the tello is continuously published and displays the state
4. `CommandSocket` is the class that creates and stores and manages the command queue and its execution, sends commands to the drone, waits for its response (if timeout set) and retries sending commands (if retries enabled)
5. `Tello` class is a wrapper class that instantiates a command, video and state socket as well as a joystick. As all commands are always sent via the command socket, `Tello` class implements functions to convert the joystick inputs to commands and calls the function to send commands, ensuring that the joystick library is isolated
6. To ensure that the tello does not automatically land after 15 seconds (this is in the Tello firmware) a command of `rc 0 0 0 0` is sent if no other commands have been sent. The timeout of this command can be set as required, and this feature can be activated/deactivated
7. The tello does not wait receive a response when an `rc` command is sent, regardless of whether the command is sent via joystick or autonomously
8. `Terminal` is a class that opens up an xterm (install xterm before using) and allows command line input that sends the commands to the drone. 

##### Notes #####
1. Due to the asynchronous nature of the communication, the responses printed to the command might not be to the command state in the statement (for example in case the joystick was moved after a land command was sent, the statement would read `received response ok to command rc a b c d` instead of `received response ok to command land`)
