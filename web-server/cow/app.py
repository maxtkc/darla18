from threading import Timer
import serial
import time
from flask import Flask, render_template, request
app = Flask(__name__)

# names of relay options
relays=["Masks","MainLights","DiscoBall","HouseLights","Strobe","Monkeys","Marque","MainMotion","Blank","Blank","Blank","Blank","Blank","Blank","Blank","Blank"]

# names of files storing sequences
sequenceFiles=["sequence0.txt","sequence1.txt","sequence2.txt","sequence3.txt","sequence4.txt"]

ser = serial.Serial('/dev/ttyACM0', 9600)

@app.route('/')
def main():
	# store visible data, update after any submission
	global visible

	# default values to display
	visible = [[1000,1,[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]],[1000,1,[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]]]
	return render_template('main.html', n=len(visible), values=visible, relays=relays)

#	# open port
#	global ser
#	ser = serial.Serial('/dev/ttyACM0', 9600)

@app.route("/set", methods=["POST"])
def set():

	# update "visible" array to represent values selected
	global visible
	global ser
	# parse request and store it to be dealt with
	Ser = toCSV(request)
	# save request
	visible = toArray(Ser)
	if request.form.get("submit") == "play":	
		# send it down and return the same template back
		ser.write(Ser.encode())
		print("Writing to serial: " + Ser.encode())
		return render_template('main.html', n=len(visible), values=visible, relays=relays)
	elif request.form.get("submit") == "addRow":
		# add new row, identical to last row visible
		newRow = visible[len(visible) - 1]
		visible.append(newRow)	
		return render_template('main.html', n=len(visible), values=visible, relays=relays)
	elif request.form.get("submit") == "save":

		# determine which file to save to
		index = int(request.form.get("saveAs"))

		# overwrite corresponding file with new csv data
		file = open(sequenceFiles[index - 1], "w")
		file.seek(0)
		file.truncate()
		file.write(Ser)
		file.close()
#print(visible[0][1])
		return render_template('main.html', n=len(visible), values=visible, relays=relays)
	elif request.form.get("submit") == "open":
	
		# determine which file to open
		index = int(request.form.get("openAs"))

		# open corresponding file
		file = open(sequenceFiles[index - 1], "r")
		New = file.read()
		file.close()
		
		# update "visible" to data from file
		visible = toArray(New)
		return render_template('main.html', n=len(visible), values=visible, relays=relays)

# convert form elements to CSV string to be passed to arduino
def toCSV(request):

	#begin string with "S"
	Ser = "S"

	# iterate over rows of table
	for i in range(0,len(visible)):

		# append user-inputted time to string, followed by comma
		Ser += (request.form.get("time" + str(i)) + ",")

		# append user-inputed song number to string, followed by comma
		Ser += (request.form.get("song" + str(i)) + ",")

		# append relay value to string, followed by comma
		relaySer = 0
# print(enumerate(relays))
		for index, item in enumerate(relays):
#			print(item , index)
			if request.form.get(item + str(i)):
#				print(index , item)
				relaySer += 2**(index)
		Ser += (str(relaySer) + ",")

		# append "dummy" value
		Ser += "0,"

	# end string with "T"
	Ser += "T"
#print(Ser)
	print "CSV form:", Ser
	return Ser
	
# convert CSV string to array to be displayed in html table
def toArray(Ser):

	# remove "S" and "T" from ends of string
	Ser = Ser[1:len(Ser)-2]

	# split string by commas
	split = Ser.split(',')

	# convert to integers
	split = [int(x) for x in split]
	# split = map(int, split)
	data = []
	for i, value in enumerate(split):
		# time
		if i % 4 == 0:
			# new row
			data.append([])
			data[i/4] = [value]
		# song
		elif i % 4 == 1:
			data[i/4].append(value)
		# relays
		elif i % 4 == 2:
			relaylist = list(reversed([int(x) for x in format(value, '016b')]))
			data[i/4].append(relaylist)
	print "Array form:", data
	return data

def update_ser(interval):
	Timer(interval, update_ser, [interval]).start()
	while ser.in_waiting:  # Or: while ser.inWaiting():
		print "Ard:", ser.readline()

update_ser(1)
		
if __name__ == '__main__':
	app.run(host='0.0.0.0', port=100, debug=True)
