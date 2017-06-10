# firmware

There are two pieces to this project. The code that runs on the AVR (power controller) and some code that runs on the Pi itself. The code on the AVR is somewhat generic, so it could probably power Beagle Bone or other Single-Board Computers.

The code for the RaspberryPi is compiled C code, not python. (You could use Python, but I didn't.) It makes use of the wiringPi library. NOOBS Jessie from April 2017 includes a version of that library in the stock install.