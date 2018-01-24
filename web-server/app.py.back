import serial
from flask import Flask, render_template, request
app = Flask(__name__)

global rows
rows = 1 
relays=["CenterLights","FaceMasks","DiscoBall","MainMotion","MarqueLights","Monkeys","LightsMain","StrobeLights","CowUp","Blank","Blank","Blank","Blank","Blank","Blank","Blank"]

@app.route('/')
def main():
	return render_template('main.html', n=rows, time="Time", relays=relays)
	print(relays)

@app.route("/set", methods=["POST"])
def set():
	times = request.form.getlist("time")
	if request.form.get("submit") == "set":
		Ser = ""
		relaySer = 0
		for index, item in enumerate(relays):
			if request.form.get(item):
				print(str(index) + "," + item)
				relaySer += 2**(index)
		print(str(relaySer))
		#ser = serial.Serial('/dev/ttyACM0', 9600)
		#ser.write('S3000,1,10,100,3000,2,20,200,3000,3,30,300,3000,4,40,400,T')
		return render_template('main.html', n=rows)
	elif request.form.get("submit") == "addRow":
		global rows
		rows =  rows + 1
		return render_template('main.html', n=rows, time=times[len(times)-1], relays=relays)

if __name__ == '__main__':
	app.run(host='0.0.0.0', port=80, debug=True)
