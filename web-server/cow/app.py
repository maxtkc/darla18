import serial
from flask import Flask, render_template, request
app = Flask(__name__)

# names of relay options
relays=["CenterLights","FaceMasks","DiscoBall","MainMotion","MarqueLights","Monkeys","LightsMain","StrobeLights","CowUp","Blank","Blank","Blank","Blank","Blank","Blank","Blank"]

# names of files storing sequences
sequenceFiles=["sequence0.txt","sequence1.txt","sequence2.txt","sequence3.txt","sequence4.txt"]

@app.route('/')
def main():
	# store visible data, update after any submission
	global visible

	# default values to display
	visible = [[1000,1,[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]],[1000,1,[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]]]
	return render_template('main.html', n=len(visible), values=visible, relays=relays)

@app.route("/set", methods=["POST"])
def set():

	# update "visible" array to represent values selected
	global visible
	Ser = toCSV(request)
	visible = toArray(Ser)
	if request.form.get("submit") == "play":	

		# open port
		ser = serial.Serial('/dev/ttyACM1', 9600)
		ser.write(Ser.encode())
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
		for index, item in enumerate(relays):
			if request.form.get(item + str(i)):
				relaySer += 2**(index - 1)
		Ser += (str(relaySer) + ",")

		# append "dummy" value
		Ser += "1,"

	# end string with "T"
	Ser += "T"
	return Ser
	
# convert CSV string to array to be displayed in html table
def toArray(Ser):

	# remove "S" and "T" from ends of string
	Ser = Ser[1:len(Ser)-2]

	# split string by commas
	split = Ser.split(',')

	# convert to integers
	split = map(int, split)
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
	return data
		
if __name__ == '__main__':
	app.run(host='0.0.0.0', port=80, debug=True)
