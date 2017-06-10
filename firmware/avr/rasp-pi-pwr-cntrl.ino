//retropie-pwr-cntrl-v4.ino
// This code expects a SPST slide switch
// For more information, visit baldengineer.com and search "SNES"
// Code is still a work in progress

const byte signalFromPi = 3;  //PD0 is SCL
//const byte switchSense = 7;
const byte switchSense = A2;    //PF5
const byte offSignalPath = A3; //PF4 Verify: no additional Z, adc 4 on 32u4
const byte heartBeatLED = 6;  
const byte debugLED = 12;
const byte frontLED = A4;       // PF1 
const byte signalToPi = A5;// PF0 (is HeartBeat on Pi)
const byte extraA1 = A1;  // PF6, Mine: A4 - Actually A1
//const byte extraA0 = A0;  // PF7, Mine: A5 - Actually A0
const byte loadEnable = A1; // The Pi!
const byte switchLED = A0; // PD6

#define enableON HIGH
#define enableOFF LOW

unsigned long previousOFFSignalCount = millis();
unsigned long previousOFFSignalInterval = 1000;

unsigned long heartBeatPreviousMillis = millis();
unsigned long heartBeatInterval = 500;
bool heartBeatState = true;

unsigned long timerOffPreviousMillis = millis();
unsigned long timerOffInterval = 15000; 

unsigned long capDrainPreviousMillis = millis();
unsigned long capDrainInterval = 1000; 

unsigned long forcePowerOff = millis();
unsigned long forcePowerOffInterval = 45000UL; 
bool forcePowerOffState = false;

bool currentButtonState;
bool previousButtonState;

bool currentPiSignalState;
bool previousPiSignalState;

enum controllerStates {
	POWER_UP,
	BOOTING_PI,
	BOOTED,
	SHUT_DOWN_PI,
	POWER_DOWN
};
enum controllerStates controllerState = POWER_UP;
enum controllerStates previousControllerState = controllerState;

/*enum piPowerStates {
	OFF,
	BOOTING,
	BOOTED,
	SHUTTING_DOWN,
	KILLED
};
enum piPowerStates piPowerState = OFF;
enum piPowerStates previousPiPowerState = OFF; */

/*enum pwrButtonStates {
	UNKNOWN,
	SW_OFF,
	SW_ON,
	CHANGED
};
enum pwrButtonStates pwrButtonState = UNKNOWN;
enum pwrButtonStates previousPwrButtonState = UNKNOWN;*/


void setup() {
	Serial.begin(9600);	// debugging 
	pinMode(debugLED, OUTPUT);

	pinMode(offSignalPath, INPUT);

	pinMode(heartBeatLED, OUTPUT);
	pinMode(switchSense, INPUT);
	pinMode(signalFromPi, INPUT);

	pinMode(signalToPi, OUTPUT);
	digitalWrite(signalToPi, LOW);
	
	pinMode(loadEnable, OUTPUT);
	digitalWrite(loadEnable, enableOFF);
}

void heartBeat(unsigned long millisTime) {
	if (millisTime - heartBeatPreviousMillis >= heartBeatInterval) {
		heartBeatPreviousMillis = millisTime;	
		heartBeatState = !heartBeatState;
		digitalWrite(heartBeatLED, heartBeatState);
	}
}


void stateDebug() {
	if (previousControllerState != controllerState) {
		Serial.println(controllerState);
		previousControllerState = controllerState;
	}
}

/*int processPiSignalChange(bool piState) {
	// save it, for some reason
	previousPiPowerState = piPowerState;
	if (piState)
		return BOOTED;
	else
		return SHUTTING_DOWN;
}

int processPwrButtonChange(bool buttonState) {
	pwrButtonState = previousPwrButtonState;
	if (buttonState)
		return CHANGED_TO_ON;
	else
		return CHANGED_TO_OFF;
}*/

void loop() {
	// blink the LED
	heartBeat(millis());

	// check where the Pi is at
	currentPiSignalState = digitalRead(signalFromPi);
	if (previousPiSignalState != currentPiSignalState) {
		// in case has some noise wait and re-sample
		delay(10);
		currentPiSignalState = digitalRead(signalFromPi);
		previousPiSignalState = currentPiSignalState;

		//piPowerState = processPiSignalChange(currentPiSignalState);
	} 

	// how is the button doing?
	currentButtonState = digitalRead(switchSense);
	if (previousButtonState != currentButtonState) {
		// in case of bounce, wait and re-sample
		delay(20);
		currentButtonState = digitalRead(switchSense);
		previousButtonState = currentButtonState;

		// let the Pi know if it should be on or off
		digitalWrite(signalToPi, currentButtonState);

		if (currentButtonState == LOW) {
			// user is forcing shutdown, so start emergency timer
			//timerOffPreviousMillis = millis(); 
			pinMode(offSignalPath, OUTPUT);
			digitalWrite(offSignalPath, HIGH);
			forcePowerOff = millis();
		}

		//pwrButtonState = processPwrButtonChange(currentButtonState);
		//pwrButtonState = CHANGED;
	}

	switch (controllerState) {
		case POWER_UP:
			stateDebug();
			// disable heart beat 
			heartBeatPreviousMillis = millis();
			heartBeatState = false;

			// should be a no brainer.
			if (currentButtonState) {
				//pwrButtonState = SW_ON;
				controllerState = BOOTING_PI;
			}	
		break;

		case BOOTING_PI:
			//stateDebug();
			heartBeatInterval = 1000;

			// Turn on the Pi
			digitalWrite(loadEnable, enableON);

			// wait for Pi to boot
			if (currentPiSignalState) {
				controllerState = BOOTED;
			}

			// kill the power if the Pi never boot (or we never)
			// get our PiAlive signal.
			if (currentButtonState == LOW) {
				capDrainPreviousMillis = millis();
				digitalWrite(signalToPi, LOW);
				controllerState = POWER_DOWN;
			}
		break;

		case BOOTED:
			//stateDebug();
			heartBeatInterval = 500;

			// wait for Pi signal that it is shutting down
			if (currentPiSignalState == LOW) {
				// Pi Initiated Shutdown
				timerOffPreviousMillis = millis();
				controllerState = SHUT_DOWN_PI;
			}

			if (currentButtonState == LOW) {
				// User is telling Pi to shut down.
				timerOffPreviousMillis = millis(); // moved to switch change
				//forcePowerOff = millis();
				controllerState = SHUT_DOWN_PI;
			} else {
				// let the swtich keep the cap up.
				pinMode(offSignalPath, INPUT);
			}


		break;

		case SHUT_DOWN_PI:
			//stateDebug();
			heartBeatInterval = 250;
			// give the Pi some time to finish up its power-down
			if (currentPiSignalState == LOW) {
				if (millis() - timerOffPreviousMillis >= timerOffInterval) {
					controllerState = POWER_DOWN;
					capDrainPreviousMillis = millis();
				}
			}

			// or a reboot is occured
			if (currentPiSignalState) {
				controllerState = BOOTED;
			}

			// Shutdown took way too long
			if (millis() - forcePowerOff >= forcePowerOffInterval) {
				controllerState = POWER_DOWN;
				capDrainPreviousMillis = millis();
			}
		break;

		case POWER_DOWN:
			//stateDebug();
			heartBeatInterval = 100;

			// let the cap drain once the switch goes off.
			if (currentButtonState == LOW) {
				// turn off the Pi
				digitalWrite(loadEnable, enableOFF);
				if (millis() - capDrainPreviousMillis >= capDrainInterval) {
					// drain the cap
					pinMode(offSignalPath, OUTPUT);
					digitalWrite(offSignalPath, LOW);
				}				
			}
		break;
	}
}
