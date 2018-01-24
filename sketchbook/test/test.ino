#define MOTOR_PWM	9
#define MOTOR_INA	7
#define MOTOR_INB	8

uint8_t x = 0;

void setup() {
	pinMode(MOTOR_PWM, OUTPUT);
	pinMode(MOTOR_INA, OUTPUT);
	pinMode(MOTOR_INB, OUTPUT);
	digitalWrite(MOTOR_INA, HIGH);
	digitalWrite(MOTOR_INB, LOW);
}

void loop() {
	analogWrite(MOTOR_PWM, x++);
	delay(100);
}
