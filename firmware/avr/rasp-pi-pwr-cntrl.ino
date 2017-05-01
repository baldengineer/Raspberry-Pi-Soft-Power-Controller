// Designed with a Custom 32u4 based board
// which is based on Arduino Leonardo
// but any 8-bit AVR would work

// I named pins wrong on my custom controller, so comments
// were to help re-map what went where. Pin assignments
// should follow Arduino Leonardo Board Pinouts
const byte offButton = 3;       //PD0 is SCL
const byte switchSense = A2;    //PF5
const byte offSignalPath = A3;  //PF4 Verify: no additional Z, adc 4 on 32u4
const byte heartBeatLED = 6;  
const byte frontLED = A4;       // PF1 /
const byte extraA0 = A0;        // PF7, Mine: A5 - Actually A0
const byte extraA1 = A1;        // PF6, Mine: A4 - Actually A1
const byte loadEnable = A1;     // The Pi's Supply's Enable!
const byte switchLED = A0;      // PD6

#define SWON true
#define SWOFF false

// Original prototype used a LDO with an inverted enable. 
// Kept these to make it easy to change active signal
#define enableON HIGH
#define enableOFF LOW

unsigned long previousOFFSignalCount = millis();
unsigned long previousOFFSignalInterval = 1000;

// Serial monitor throttle (debugging)
unsigned long previousStatePrintMillis = millis();
unsigned long previousStataPrintInterval = 1000;

// Blinking LED. Helps debug state machine
unsigned long heartBeatPreviousMillis = millis();
unsigned long heartBeatInterval = 500;
bool heartBeatState = true;

// Time to keep Pi on, after receiving "shutdown" signal
unsigned long timerOffPreviousMillis = millis();
unsigned long timerOffInterval = 15000; 

// Time to keep LDO on, after Pi is off. (Needed? I dunno)
unsigned long capDrainPreviousMillis = millis();
unsigned long capDrainInterval = 1000; 

bool previousButtonState = SWOFF;
bool previousPiSignalState = HIGH;

enum powerStates {
	POWER_OFF,		   // turn off all regulators
	SWITCH_ON,         // switch moved to ON position
	SWITCH_OFF,        // switch moved to OFF position
	TELL_PI_GO_OFF,	   // communicate to Pi is time to shutdown
	WAIT_FOR_PI,	   // ready for Pi to let us shut down
	SWITCH_CHANGE,     // switch changed state
	PISIGNAL_CHANGE,   // the Pi changed its mind
	START_OFF_TIMER,   // start countdown to off
	COUNTING_TO_OFF,    // start counting
	SHUTDOWN 			// drain the cap 
};

enum powerStates powerState = POWER_OFF;
enum powerStates previousState = powerState;

void setup() {
	Serial.begin(9600);

	pinMode(heartBeatLED, OUTPUT);
	pinMode(switchSense, INPUT);
	
	// keep off path in High-Z until hold-up
	// cap is fully charged
	pinMode(offSignalPath, INPUT);

	// use external pull-down, unless AVR design is 3v3
	pinMode(offButton, INPUT);
	pinMode(loadEnable, OUTPUT);
	digitalWrite(loadEnable, enableOFF);
	powerState = POWER_OFF; 
}

// Typical millis() detection for flashing LED
void heartBeat() {
	unsigned long millisTime = millis();
	if (millisTime - heartBeatPreviousMillis >= heartBeatInterval) {
		heartBeatPreviousMillis = millisTime;	
		heartBeatState = !heartBeatState;
	}
	digitalWrite(heartBeatLED, heartBeatState);
}

// For printing debug
void stateDebug() {
	if (previousState != powerState) {
		// used flashing LED more than serial, so never
		// created a switch() statement for these
		Serial.println(powerState);
		previousState = powerState;
	}
}

void loop() {
	unsigned long currentMillis = millis();
	bool currentButtonState = digitalRead(switchSense);
	bool currentPiSignalState = digitalRead(offButton);

	heartBeat();

	// what does the sense pin look like
	digitalWrite(switchLED, currentButtonState);

	// was having trouble with Pi signal until I realized, it is floating
	/* if (previousPiSignalState != currentPiSignalState) {
		delay(10);
		currentPiSignalState = digitalRead(offButton);
		powerState = PISIGNAL_CHANGE;
		previousPiSignalState = currentPiSignalState;
	} */

	// bouncing really isn't an issue with this signal
	currentPiSignalState = digitalRead(offButton);
	if (currentPiSignalState)
		powerState = SWITCH_CHANGE;
	else
		powerState = SWITCH_OFF;

	// monitor state of "Power" Button
	// TODO: Add timer to detect how long it has been pressed or not-pressed
	if (previousButtonState != currentButtonState) {
		delay(20); // crude debounce
		currentButtonState = digitalRead(switchSense);
		powerState = SWITCH_CHANGE;
		previousButtonState = currentButtonState;
	}

	switch (powerState) {
		case POWER_OFF:
			stateDebug();
			// if 1s has passed and we're back at power-off, then
			// controller is trying to shutdown. 
			if (millis() > 1000) {
				pinMode(offSignalPath, OUTPUT);
				digitalWrite(offSignalPath, LOW);
			}
			digitalWrite(loadEnable, enableOFF);

			// keep heartbeat off
			heartBeatPreviousMillis = currentMillis;
			heartBeatState = false;
		break;

		case SWITCH_ON:
			stateDebug();
			heartBeatInterval = 1000;
			heartBeatState = true;
			
			// go High Z while switch is in ON position
			// testing has been done with a momentary pushbutton
			// final design uses a SPST. code should handle both			
			pinMode(offSignalPath, INPUT);
			// delay(20); // make sure cap is charged before enabling big supply
			digitalWrite(loadEnable, enableON);
		break;

		case SWITCH_OFF:
			stateDebug();
			heartBeatInterval = 500;

			// turn on hold-up signal
			pinMode(offSignalPath, OUTPUT);
			// TODO: measure Cap Charge Time
			digitalWrite(offSignalPath, HIGH); 
			powerState = TELL_PI_GO_OFF;
		break;

		case TELL_PI_GO_OFF:
			// eventually need to send signal to Pi to initiate shutdown.
			// that code would go here....
			// digitalWrite(somepin, HIGH);

			// for now, just tell pi to shutdown manually
			powerState = WAIT_FOR_PI;
			
			// TODO: Emergency timer, shutdown after 2 minutes.
		break;

		case WAIT_FOR_PI:
			stateDebug();
			heartBeatInterval = 250;
			
			// simulate pi siganl with serial
			if (Serial.read() == '.') 
				powerState = START_OFF_TIMER;

			// may need something more complicated than this
			if (digitalRead(offButton) == HIGH)
				powerState = START_OFF_TIMER;

			// could also add a pushbutton, for "off" or
			// setup a timer for holding the ON button
		break;

		/* case PISIGNAL_CHANGE:
			if (currentPiSignalState)
				powerState = SWITCH_CHANGE;
			else
				powerState = SWITCH_OFF;
		break; */
		
		case SWITCH_CHANGE:
			stateDebug();
			if (digitalRead(switchSense))
				powerState = SWITCH_ON;
			else
				powerState = SWITCH_OFF;
		break;

		case START_OFF_TIMER:
			stateDebug();
			heartBeatInterval = 100;
			
			timerOffPreviousMillis = millis();
			powerState = COUNTING_TO_OFF;
		    // control signal to HighZ
			//pinMode(offSignalPath, INPUT);
		break;

		case COUNTING_TO_OFF:
			heartBeatInterval = 50;
			stateDebug();
			
			// have we sat for 15 seconds yet?
			currentMillis = millis();
			if (currentMillis - timerOffPreviousMillis >= timerOffInterval) {
				powerState = SHUTDOWN;
				
				timerOffPreviousMillis = currentMillis;
				capDrainPreviousMillis = currentMillis;
			}
		break;

		case SHUTDOWN:
			stateDebug();
			heartBeatInterval = 25;
			
			pinMode(offSignalPath, OUTPUT);
			digitalWrite(loadEnable, enableOFF);
			digitalWrite(offSignalPath, LOW);
		/*	if (currentMillis - capDrainPreviousMillis >= capDrainInterval) {
				powerState = POWER_OFF;
				capDrainPreviousMillis = currentMillis;
			}*/
		break;

		default:
			//Serial.println(F("uhhh, non-existant state."));
		break;
	}

}
