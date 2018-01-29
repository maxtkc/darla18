#include <SoftwareSerial.h>
#include "Adafruit_Soundboard.h"
#include "Adafruit_MCP23017.h"
#include <Wire.h>
#include <avr/interrupt.h>

// Set pins for soundboard
#define SFX_TX 		5
#define SFX_RX 		6
#define SFX_RST 	4

// Default address for relay controller
#define MCP_addr 	0

#define IDLE		0
#define PLAY		1
#define ERROR		2
#define READ		3
#define DRIVE		4

#define ROWS		6
#define COLS		4

// Second identifiers for scroll
#define DURATION	0
#define RELAY		2
#define SONG		1
#define SLED		3

#define STOP		0xFF

#define MOTOR_INA	7
#define MOTOR_INB	8
#define MOTOR_PWM	9
#define MOTOR_SPD	200
#define IN		1
#define OUT		2

#define OUTSTOP		2
#define INSTOP		3

/* Pinmap
   2	OUTSTOP
   3	INSTOP
   4	SFX_RST
   5	SFX_TX
   6	SFX_RX
   7	MOTOR_INA
   8	MOTOR_INB
   9	MOTOR_PWM
*/

// Create objects for Soundboard
SoftwareSerial ss = SoftwareSerial(SFX_TX, SFX_RX);
Adafruit_Soundboard sfx = Adafruit_Soundboard(&ss, NULL, SFX_RST);

// Create object for relay control
Adafruit_MCP23017 mcp;

uint8_t state = IDLE;
uint32_t countNow; 
uint32_t countLast;
uint8_t row;
uint8_t col;
uint8_t lastRow;
String inString = "";
uint16_t scroll[ROWS][COLS]; 

char songs[12][12] = {	"LOONY   OGG",
						"BYARD   OGG", 
						"CRACK   OGG",
						"ICEC    OGG",
						"LONE    OGG",
						"MLING   OGG",
						"MTC     OGG",
						"NEWMC   OGG"
};

void mcp_init() {
	mcp.begin(MCP_addr); 

	// Set all relay control pins to output and turn off all relays 
	for(uint8_t x = 0; x < 16; x++) {
		mcp.pinMode(x, OUTPUT);
		mcp.digitalWrite(x, HIGH);
	}
}

void instop() {
	analogWrite(MOTOR_PWM, 0);
}

void outstop() {
	analogWrite(MOTOR_PWM, 0);
}

// song number or STOP for no song
void playMusic(uint16_t song_number) {
	if(song_number != STOP)	sfx.playTrack(songs[song_number]);
}

// sled must be OUT or IN or STOP -- thx max for ternerary ops
void secondaryMotion(uint16_t sled) {
	digitalWrite(MOTOR_INA, (sled == OUT)?HIGH:LOW);
	digitalWrite(MOTOR_INB, (sled == IN)?HIGH:LOW);
	analogWrite(MOTOR_PWM, MOTOR_SPD);
}

void setup() {
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

	// Motor init
	pinMode(MOTOR_PWM, OUTPUT);
	pinMode(MOTOR_INA, OUTPUT);
	pinMode(MOTOR_INB, OUTPUT);

	// Sled interupt
//SHOULD FIX THIS  DOES NOT COMPILE
//	attachInterrupt(digitalPinToInterrupt(OUTSTOP), outstop, LOW);
//	attachInterrupt(digitalPinToInterrupt(INSTOP), instop, LOW);

	Serial.println(state);

//TEST CODE
//	sfx.playTrack(songs[3]);
//	mcp.writeGPIOAB(~0x0000);
	secondaryMotion(STOP);
	while(1);
}

void loop() {
	switch(state) {
		case IDLE:
			if (Serial.available()) { 
				char inChar = Serial.read(); 
				if (inChar == 'S') {
					state = READ;
					Serial.println("READ");
				}
				else if (inChar == 'D') {
					state = DRIVE;
					Serial.println("DRIVE");
				}
			}
			break;
		case PLAY:
			countNow = millis();
			if (countNow - countLast > scroll[row][DURATION]) {
				mcp.writeGPIOAB(~scroll[row][RELAY]);	
				playMusic(scroll[row][SONG]);
				secondaryMotion(scroll[row][SLED]);
				countLast = countNow;
				if (++row >= lastRow) {
					row = 0;
					state = IDLE;
					Serial.println("IDLE");
				}
			}
			break;
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
		case ERROR:
			//Serial.println('$');
			digitalWrite(13, HIGH);
			break;
	}
}

