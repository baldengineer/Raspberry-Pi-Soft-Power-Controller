# RaspberryPi Code
There are two types of files in this directory. One is C-based programs. The other is systemd services. You need them all installed. Here's some detailed notes.

## C-Programs
This project doesn't use Python, it uses C. So you'll need to compile the code on your Pi. These utlities require the "[Wiring Pi](http://wiringpi.com/)" library installed. 

### Installing Wiring Pi
My copy of Jessie had Wiring Pi installed. You can tell by running this command:
```
$ gpio -v
```
If you get something back, it is installed. If you'd like the latest version (or you don't have it installed at all) follow these directions on [installing the latest wiringPi](http://wiringpi.com/download-and-install/).

### C-Start.c
This utility runs at startup. It enables a specific GPIO pin as output and sets it HIGH. I'm using GPIO18 (which is pin 1 in WiringPi's numbering convention. See below for details.) It only needs user-level privilege. 

Compile this code and "install" using:
```
$ gcc c-start.c -o c-start -lwiringPi
$ cp c-start /usr/local/bin/
```
### C-Stop.c
This utility runs during shutdown (and reboot.) It enables a specific GPIO pin as output and sets it LOW. I'm using GPIO18 (which is pin 1 in WiringPi's numbering convention. See below for details.) It only needs user-level privilege. 

Compile this code and "install" using:
```
$ gcc c-stop.c -o c-stop -lwiringPi
$ cp c-stop /usr/local/bin/
```

## systemd services
A big difference between "wheezy" and "jessie" is the init process. wheezy uses SystemInitV and jessie uses systemd. I found a *ton* of questions on how to run a script on startup or shutdown on the raspberry Pi. Everything was SystemInitV based, which won't work with the latest versions of Raspbian. (e.g. if you find putting things in /etc/rc.d isn't working, your linux is probably using systemd).

These could probably be combined into the same service, but I didn't try yet. Also, I can't figure out how to get the shutdown signal to work only on shutdown (and not reboot.)

### powercontrol-comms.service
This service will run c-start during boot and c-stop during shutdown. This is the signal our AVR-based power controller is monitoring. To install, put this file into `/etc/systemd/system`. (Note /lib/systemd is meant for *system* files. Not *user* files.) Once copied, do this:

```
$ cd /etc/systemd/system
$ ls powercontrol-coms.service #verify it is there
$ sudo systemctl enable powercontrol-coms.service
# reboot (or install the next one)
```
Make sure you copy `c-start` and `c-stop` to `/usr/local/bin/`.

### shutdown-gpio.service
This service will run c-watch during boot. This program checks once every second whether the power controller has told us to shutdown. To install, put this file into `/etc/systemd/system`. (Note /lib/systemd is meant for *system* files. Not *user* files.) Once copied, do this:

```
$ cd /etc/systemd/system
$ ls shutdown-gpio.service #verify it is there
$ sudo systemctl enable shutdown-gpio.service
# reboot (or install the previous)
```
Make sure you copy `c-start` and `c-stop` to `/usr/local/sbin/` (different from other file).


## Wiring Pi Pin Numbers
WiringPi has it's own numbering convention. So I made this table to be a quick look up for pin numbers. You can also run "gpio readall" on the Pi for a table.
```
Special :    SD SC                   MO MI SK    | ID  
WiringPi: 3v 08 09 07 0v 00 02 03 3v 12 13 14 0v | 00 05 06 13 19 26 0v
Physical: 01 03 05 07 09 11 13 15 17 19 21 23 25 | 27 29 31 33 35 37 39

Special :          TX RX                   C0 C1 | IC
WiringPi: 5v 5v 0v 15 16 01 0v 04 05 0v 06 10 11 | 01 0v 12 0v 16 20 21
Physical: 02 04 06 08 10 12 14 16 18 20 22 24 26 | 28 30 32 34 36 38 40
```

Special are special functions, so you probably don't want to use those for the power controller. Physical are the actual pin numbers. And then WiringPi is the number to use in the C-code to correspond with the Physical pin number. (It's confusing, but that's why I made the chart.)