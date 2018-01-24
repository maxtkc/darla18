import serial

ser = serial.Serial('/dev/ttyACM0', 9600)

#ser.write('1000,0001,00,0')

while True:
	line = ser.readline()
	print(line)
