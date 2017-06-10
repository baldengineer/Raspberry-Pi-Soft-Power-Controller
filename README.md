# Raspberry-Pi-Soft-Power-Controller

For now, you can find more information at:
https://hackaday.io/project/21625-raspberry-pi-soft-power-controller

## Requirements
* Raspberry Pi running Jessie
* 2 GPIO Pins on Raspberry Pi
* AVR (Arduino Uno, Leonardo, etc) w/ 2 GPIO
* Slide Switch (or Pushbutton)

### Optional
* Bald Engineer's RetroPie Controller

## Notes
* Currently the AVR code is designed for a SPST switch and not a push button. I'm working on a push button version, but that isn't what I needed in my project. 
* Please don't fab the PCBs as-is. There are design issues for them. I am open to collaboration on a "hat" version of these boards. I have some ideas. (Key, can we mount a barrel jack upside down??)
