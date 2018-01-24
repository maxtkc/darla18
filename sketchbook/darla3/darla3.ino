#include <SoftwareSerial.h>
#include "Adafruit_Soundboard.h"
#include "Adafruit_MCP23017.h"
#include <Wire.h>

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

#define MAX_INTERVALS 	1
#define OUTPUTS			4

// Second identifiers for scroll
#define DURATION	0
#define RELAY		1
#define SONG		2
#define SLED		3
//#define GARAGE		4

#define STOP	0xFF

// Create objects for Soundboard
SoftwareSerial ss = SoftwareSerial(SFX_TX, SFX_RX);
Adafruit_Soundboard sfx = Adafruit_Soundboard(&ss, NULL, SFX_RST);

// Create object for relay control
Adafruit_MCP23017 mcp;

uint8_t state = IDLE;

uint32_t countNow 	= 0;
uint32_t countLast 	= 0;
uint8_t  index 		= 0;

uint16_t test_score[] = {0x0000,0x0001,0x0002,0x0003,0x0004,0x0005,0x0006,0x0007,0x0008,0x0009,0x000A,0x000B,0x000C,0x000D,0x000E,0x000F};
uint16_t test_interval[] = {1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000,1000};

uint16_t score[MAX_INTERVALS];
uint16_t interval[MAX_INTERVALS];

uint16_t scroll[MAX_INTERVALS][OUTPUTS] = {{1000,0x0001,STOP,0},{1000,0x0002,STOP,0},{1000,0x0003,STOP,0},{1000,0x000F,0,0},{1000,0x0000,STOP,0}};

uint8_t last_char	= '\n';

void mcp_init() {
	mcp.begin(MCP_addr); 

	// Set all relay control pins to output and turn off all relays 
	for(uint8_t x = 0; x < 16; x++) {
		mcp.pinMode(x, OUTPUT);
		mcp.digitalWrite(x, HIGH);
	}
}

void setup() {
  	ss.begin(9600); // Start software serial for soundboard
	Serial.begin(9600); // Hardware serial for rpi

	pinMode(13,OUTPUT); // Initialize indicator light
	digitalWrite(13, LOW);

	// Relay control init
	mcp_init(); 

	// Soundboard init
  	if (!sfx.reset()) {
		state = ERROR;
	}

	state = PLAY; // TODO:Wait to get call from bluetooth
}

// song number or STOP for no song
void playMusic(uint16_t song_number) {
	if(song_number != STOP)	sfx.playTrack(song_number);
}

void secondaryMotion(uint16_t sled) {
	// TODO:Work with motor controller
}

// garage is HIGH or LOW
/*void primaryMotion(uint8_t garage) {
	mcp.digitalWrite(16, garage);
}*/

// Takes a character hex digit dig and returns 
// the decimal or -1 if not in hex
uint8_t unHex(uint8_t dig) {
	if(dig - '0' < 10)
		return dig - '0';
	if(dig - 'a' <= 'f' - 'a')
		return dig - 'a' + 10;
	if(dig - 'A' <= 'F' - 'A')
		return dig - 'A' + 10;
	last_char = dig;
	return -1;
}

// Returns first hex number available over Serial
// and reads following character
uint16_t getNextNumber() {
	uint16_t num = 0;
	uint8_t dig;
	for(uint8_t i = 0; (dig = unHex(Serial.read())) != -1; i++) {
		num += dig;
	}
	return num;
}

void loop() {
	switch(state) {
		case IDLE:
			digitalWrite(13, HIGH);
			break;
		case PLAY:
			countNow = millis();
			if (countNow - countLast > scroll[index][DURATION]) {
//				Serial.println(scroll[index]);
				mcp.writeGPIOAB(~scroll[index][RELAY]);	
				playMusic(scroll[index][SONG]);
				secondaryMotion(scroll[index][SLED]);
				//primaryMotion(scroll[index][GARAGE]);
				countLast = countNow;
				if (++index >= MAX_INTERVALS) state = IDLE;
			}
			break;
		case READ:
			// Full command available
			if(Serial.available() > 16) { 
				for(uint8_t i = 0; i < OUTPUTS; i++) {
					scroll[index][i] = strtol(Serial.readStringUntil(','));
				}
				index++;
			}
			// last_char = EOF, therefore done reading
			if(index >= MAX_INTERVALS) {
				index = 0;
				state = PLAY;
			}
			break;
	}
}

/*void read_mssg(uint8_t mssg[][MSSG_LENGTH]) {
	uint8_t mssg_len;
	uint8_t mssg_index = 0;
	uint8_t incoming_byte;
	while(Serial.available() > 0) {
		incoming_byte = Serial.read();
		if(incoming_byte == 's') {
			uint8_t mssg[mssg_len = Serial.parseInt()][MSSG_LENGTH];
		}
		else if(incoming_byte == 'l') {
			if(Serial.parseInt() == mssg_index && Serial.read() == 't') { 
				mssg[mssg_index][0] = Serial.parseInt();
				for(uint8_t i = 0; i < MSS_LENGTH && Serial.read() == ','; i++) {
					mssg[mssg_index][i] = Serial.parseInt();
				}
			}
			mssg_index++;
		}
		else {
		}
		if(Serial.read() != '\n') {
		} 
	}
*/
/*
  delay(1000);

  mcp.digitalWrite(0, HIGH);

  delay(1000);

  mcp.digitalWrite(0, LOW);
*/

