/* See notes on bottom */

#include <wiringPi.h>

int ioPin = 1; // GPIO18`

int main (void) {
        wiringPiSetup ();
        pinMode (ioPin, OUTPUT);

        digitalWrite (ioPin, LOW);
        delay(1000);

        return 0;
}

/* 
Install the wiringpi library on your Pi (it may already have a version)
# apt-get install wiringPi

And then compile this code
# gcc c-stop.c -o c-stop -lwiringPi
copy it to /usr/local/bin
You need to use systemd and install "powercontrol-coms.service"

WiringPi has it's own numbering convention. So I made this table to 
be a quick look up for pin numbers. You can also run "gpio readall" 
on the Pi for a table.

Special :    SD SC                   MO MI SK    | ID  
WiringPi: 3v 08 09 07 0v 00 02 03 3v 12 13 14 0v | 00 05 06 13 19 26 0v
Physical: 01 03 05 07 09 11 13 15 17 19 21 23 25 | 27 29 31 33 35 37 39

Special :          TX RX                   C0 C1 | IC
WiringPi: 5v 5v 0v 15 16 01 0v 04 05 0v 06 10 11 | 01 0v 12 0v 16 20 21
Physical: 02 04 06 08 10 12 14 16 18 20 22 24 26 | 28 30 32 34 36 38 40
*/