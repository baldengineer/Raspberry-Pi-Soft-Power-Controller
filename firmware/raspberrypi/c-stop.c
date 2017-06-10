// install the wiringpi library on your Pi
// And then compile this code
// gcc c-stop.c -o c-stop -lwiringPi
// copy it to /usr/local/bin

#include <wiringPi.h>

int ioPin = 1; // GPIO18`

int main (void) {
        wiringPiSetup ();
        pinMode (ioPin, OUTPUT);

        digitalWrite (ioPin, LOW);
        delay(1000);

        return 0;
}