/*
1) Need to dynamically set dropdown for songs
3) Need to send 0xff for no_song (working on arduino side)
4) Need to fix problem with code not starting the first time
data is sent from website
5) Need to autolaunch python application on reboot
7) Need to test secondary motion
8) Need to add ability to control sound volume (done on Arduino)
10) Need to print serial feedback in python for debugging
purposes
11) Kill button for secondary motion
12) Kill button for all control
*/

#include <SoftwareSerial.h>
#include "Adafruit_Soundboard.h"
#include "Adafruit_MCP23017.h"
#include <Wire.h>
#include <avr/interrupt.h>

// Masking for relay NOTE: These must be sent inverted to relay control
#define	MASKS			0x0001
#define MAIN_LIGHTS		0x0002
#define DISCO_BALL		0x0004
#define	HOUSE_LIGHTS	0x0008
#define STROBES			0x0010
#define MONKEYS			0x0020
#define MARQUEE			0x0040
#define MAIN_MOTION		0x0080
#define COW_UP			0x8000

// Soundboard pins 
#define SFX_TX 		5
#define SFX_RX 		6
#define SFX_RST 	4

// Soundboard commands
#define STOP_MUSIC	0xFF
#define VOLUME		150

// Soundboard song title dimensions
#define ROWS		6
#define COLS		4

// Default address for relay controller
#define MCP_addr 	0

// System states
#define IDLE		0
#define PLAY		1
#define ERROR		2
#define READ		3
#define DRIVE		4
#define TEST		5
#define WAIT		6	

// Second identifiers for scroll
#define DURATION	0
#define RELAY		2
#define SONG		1
#define SLED		3 

// Secondary motion commands
#define STOP_MOTION	2
#define FORWARD		0
#define REVERSE		1

// Secondary motion pins
#define MOTOR_INA	7
#define MOTOR_INB	8
#define MOTOR_PWM	9

// Second motion limit switch pins
//IMPORTANT: 	outstop is N.O. (default HIGH) 	:ISR vector 0
//				instop is N.C. (default LOW)	:ISR vector 1
#define OUTSTOP		2	
#define INSTOP		3

// Secondary motion speed
//NOTE: The slowest possible speed is 75 and the fastest is 255
#define MOTOR_SPD	200

// Create software serial for soundboard
SoftwareSerial ss = SoftwareSerial(SFX_TX, SFX_RX);
// Create soundboard object
Adafruit_Soundboard sfx = Adafruit_Soundboard(&ss, NULL, SFX_RST);
// Create relay control object
Adafruit_MCP23017 mcp;

uint8_t state = IDLE;
uint32_t countNow; 
uint32_t countLast;
uint8_t row;
uint8_t col;
uint8_t lastRow;
String inString = "";
uint16_t scroll[ROWS][COLS]; 

void secondaryMotion(uint8_t);

char songs[12][12] = {	
						"LOONY   OGG",
						"BYARD   OGG", 
						"CRACK   OGG",
						"ICEC    OGG",
						"LONE    OGG",
						"MLING   OGG",
						"MTC     OGG",
						"NEWMC   OGG"
};

void stop_forward_motion(void) {
	detachInterrupt(0);
	secondaryMotion(STOP_MOTION);
}

void stop_reverse_motion(void) {
	detachInterrupt(1);
	secondaryMotion(STOP_MOTION);
}

void mcp_init() {

	// Initialize MCP23017
	mcp.begin(MCP_addr); 

	// Set all relay control pins to output and turn off all relays 
	for(uint8_t x = 0; x < 16; x++) {
		mcp.pinMode(x, OUTPUT);
		mcp.digitalWrite(x, HIGH);
	}
}

// Play song, number cooresponds to row of array
void playMusic(uint16_t song_number) {
		sfx.playTrack(songs[song_number]);
}

// Stop playing song
void playStop() {
	sfx.stop();
}

// Set volume for music player
void playVolume(uint8_t newVolume) {
	int oldVolume = sfx.volUp();	
	if (newVolume < oldVolume) {
		while (newVolume < oldVolume) oldVolume = sfx.volDown();
	}
	else {
		while (newVolume > oldVolume) oldVolume = sfx.volUp();
	}
}

// Drive secondary motion
void secondaryMotion(uint8_t direction) {
	if (direction == FORWARD) {
		analogWrite(MOTOR_PWM, MOTOR_SPD);
		digitalWrite(MOTOR_INA, HIGH);
		digitalWrite(MOTOR_INB, LOW);
		attachInterrupt(0, stop_forward_motion, LOW);
	}
	else if (direction == REVERSE) {
		analogWrite(MOTOR_PWM, MOTOR_SPD);
		digitalWrite(MOTOR_INA, LOW);
		digitalWrite(MOTOR_INB, HIGH);
		attachInterrupt(1, stop_reverse_motion, HIGH);
	}
	else if (direction == STOP_MOTION) {
		analogWrite(MOTOR_PWM, 0x00);
		digitalWrite(MOTOR_INA, LOW);
		digitalWrite(MOTOR_INB, LOW);
	}
}

// This is ONLY a test drive function
void primaryMotion(void) {
	mcp.writeGPIOAB(~0x0080);
	delay(300);
	mcp.writeGPIOAB(~0x0000);
}

void setup() {
	// Serial init
  	ss.begin(9600); // Start software serial for soundboard
	Serial.begin(9600); // Hardware serial for RPI

	pinMode(13, OUTPUT); // Initialize indicator light
	digitalWrite(13, LOW);

	// Relay control init
	mcp_init(); 

	state = IDLE;

	// Soundboard init
  	if (!sfx.reset()) {
		state = ERROR;
		Serial.println("ERROR");
	}
	//Set initial volume
	playVolume(VOLUME);

	// Motor init
	digitalWrite(MOTOR_INA, LOW);
	digitalWrite(MOTOR_INB, LOW);
	pinMode(MOTOR_PWM, OUTPUT);
	pinMode(MOTOR_INA, OUTPUT);
	pinMode(MOTOR_INB, OUTPUT);
	analogWrite(MOTOR_PWM, MOTOR_SPD);

	// Sensor init
	pinMode(OUTSTOP, INPUT_PULLUP);
	pinMode(INSTOP, INPUT_PULLUP);

	Serial.println("READY");

//TESTING SCRATCH PAD
//playMusic(0);
//	delay(2000);
//	playStop();
//	playMusic(1);
//	delay(2000);
//	playStop();
//	playMusic(1);

//	mcp.writeGPIOAB(~0x0000);
//	primaryMotion();
//	secondaryMotion(REVERSE);
//	while(1);

}

void loop() {
	switch(state) {
		case IDLE:
			if (Serial.available()) { 
				char inChar = Serial.read(); 
				//All live control sequence commands must start with S
				if (inChar == 'S') {
					state = READ;
					Serial.println("READ");
				}
				//This is just for controlling the song USB drive
				else if (inChar == 'D') {
					state = DRIVE;
					Serial.println("DRIVE");
				}
				//This is just for sending simple test commands
				else if (inChar == 'X') {
					state = TEST;
					Serial.println("TEST");
				}
			}
			break;
			//Play one row of commands
		case PLAY:
				//Check to see if MAIN_MOTION bit is set
				//If it is set, write relay value wait and then clear
				//just the main motion bit and then write relay value again
				//The delay is required for triggering the main motion
				if (scroll[row][RELAY] & MAIN_MOTION) {
					mcp.writeGPIOAB(~scroll[row][RELAY]);	
					delay(300);
					scroll[row][RELAY] &= ~MAIN_MOTION;
					mcp.writeGPIOAB(~scroll[row][RELAY]);	
				}
				else
				//If main motion bit not set, just write relay value
					mcp.writeGPIOAB(~scroll[row][RELAY]);	

				//Play music if song number is NOT set to 0xff
				if (scroll[row][SONG] != 0xFF)
					playMusic(scroll[row][SONG]);
			
				//Control secondary motion
				secondaryMotion(scroll[row][SLED]);

				//Move to WAIT to wait for next row of commands
				state = WAIT;
			break;
			//Wait for count to elapse between each row of commands
		case WAIT:
			countNow = millis();
			if (countNow - countLast > scroll[row][DURATION]) {
				countLast = countNow;
				row++;
				//Stop playing song after each row of commands are complete.
				playStop();	
				state = PLAY;
				if (row >= lastRow) {
					row = 0;
					state = IDLE;
					Serial.println("IDLE");
				}
			}
			break;
			//Read the entire serial string before playing
			//The string format is:
			//S[duration],[song],[relay],[secondary motion],...T
		case READ:
			if(Serial.available()) { 
				int inChar = Serial.read();

				if (isDigit(inChar)) {
					inString += (char)inChar;
				}
				else if (inChar == ',') {
					scroll[row][col] = inString.toInt();
					col++;
					inString = "";
				}
				else if (inChar == 'T'){
					lastRow = row;
					row = 0;
					countLast = millis();	
					state = PLAY;
					Serial.println("PLAY");
				}
				else {
					state = ERROR;
					Serial.println("ERROR");
				}
				if (col >= COLS){
					row++;
					col = 0;
				}
			}
			break;
		case DRIVE:
			//This is the recommended way to initialize
			//digitalWrite(reset_pin, LOW);
			//pinMode(reset_pin, OUTPUT);
			//delay(10);
			//pinMode(reset_pin, INPUT);
			//delay(1000); // give a bit of time to 'boot up'

			//Initialize FX board for use as drive
			digitalWrite(SFX_RST, LOW);
			digitalWrite(SFX_RST, HIGH);
			state = IDLE;
			Serial.println("IDLE");
		case TEST:
			if(Serial.available()) { 
				int inChar = Serial.read();
				if (inChar == 'F') {
					secondaryMotion(FORWARD);
					secondaryMotion(FORWARD);
					Serial.println("FOR");
					state = IDLE;
				}
				else if (inChar == 'R') {
					secondaryMotion(REVERSE);
					secondaryMotion(REVERSE);
					Serial.println("REV");
					state = IDLE;
				}
				else if (inChar == 'V') {
					Serial.println(sfx.volUp());
					Serial.println("VUP");
					state = IDLE;
				}
				else if (inChar == 'D') {
					mcp.writeGPIOAB(~0x0004);
					state = IDLE;
				}
				else if (inChar == 'O'){
					mcp.writeGPIOAB(~0x0000);
					state = IDLE;
				}
			}
				
		case ERROR:
			Serial.println('$');
			digitalWrite(13, HIGH);
			break;
	}
}

