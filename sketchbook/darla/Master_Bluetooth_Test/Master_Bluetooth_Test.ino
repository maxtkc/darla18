#include <SoftwareSerial.h>  

int bluetoothTx = 5;  // TX-O pin of bluetooth mate, Arduino D2
int bluetoothRx = 6;  // RX-I pin of bluetooth mate, Arduino D3

SoftwareSerial bluetooth(bluetoothTx, bluetoothRx);
void setup() {
  Serial.begin(9600);  // Begin the serial monitor at 9600bps
  bluetooth.begin(115200);  // The Bluetooth Mate defaults to 115200bps
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available())  // If stuff was typed in the serial monitor
  {
      bluetooth.print('l');
  }
}
