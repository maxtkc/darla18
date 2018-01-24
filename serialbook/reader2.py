import serial
import time

ser = serial.Serial('/dev/ttyACM1', 9600)
#time.sleep(7)
#if ser.isOpen():
#	ser.flush()
#ser.write('S3000,1,10,100,3000,2,20,200,3000,3,30,300,3000,4,40,400,T')
ser.write('S3000,2,1,100,T')
#ser.close()
#line = " "
#while line[0] != '$':
while True :
	line = ser.readline()
	print(line)
#ser.close()
	
