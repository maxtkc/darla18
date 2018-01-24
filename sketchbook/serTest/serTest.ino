#include <SoftwareSerial.h>

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
}

void loop() { // run over and over
  Serial.print('p');
  delay(1000);
}

