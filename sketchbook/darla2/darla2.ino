#include <SoftwareSerial.h>
#include "Adafruit_Soundboard.h"
#include "Adafruit_MCP23017.h"
#include <Wire.h>

// Choose any two pins that can be used with SoftwareSerial to RX & TX
#define SFX_TX 5
#define SFX_RX 6

// Connect to the RST pin on the Sound Board
#define SFX_RST 4

#define MCP_addr 0

SoftwareSerial ss = SoftwareSerial(SFX_TX, SFX_RX);
Adafruit_Soundboard sfx = Adafruit_Soundboard(&ss, NULL, SFX_RST);

Adafruit_MCP23017 mcp;

uint8_t state;

// sets 0-16 i2c pins as outputs
void mcp_init() {
	mcp.begin(MCP_addr); //default adress 0
	for(uint8_t x = 0; x < 16; x++) {
		mcp.pinMode(x, OUTPUT);
	}
}

void setup() {
  	ss.begin(9600);
	Serial.begin(9600);

	mcp_init();

  	if (!sfx.reset()) {
		state = 1;
	}
	//sfx.playTrack(1);
}

void blink(uint16_t period) {
	mcp.digitalWrite(0, HIGH);
	delay(period/2);
	mcp.digitalWrite(0, LOW);
	delay(period/2);
}

void loop() {

	if (	
	read_mssg();
	for(uint8_t i = 0; i < 5; i++) {
		blink(1000);
	}
}

void read_mssg(uint8_t mssg[][MSSG_LENGTH]) {
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
/*
  delay(1000);

  mcp.digitalWrite(0, HIGH);

  delay(1000);

  mcp.digitalWrite(0, LOW);
*/
}
