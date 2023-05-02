import os
import subprocess
import sys
from subprocess import CalledProcessError, check_output
from flask import Flask, render_template, request, jsonify

app = Flask(__name__, template_folder='templates')

@app.route('/wifi-change', methods=['GET', 'POST'])
def index():
    if request.method == 'POST':
        ssid = request.json['ssid']
        password = request.json['password']
        response = jsonify({'message': 'Processing request...'})
        response.status_code = 202
        try:
            output = check_output(['sudo','/home/milky/projects/weatherhub/change-wifi/scripts/configure.sh', '-a', 'WeatherHub', 'password', '-c', ssid, password])
            print(output.decode('UTF-8'))
            response = jsonify({'message': output.decode('UTF-8')})
            response.status_code = 200

            os.system("sudo reboot")
        except CalledProcessError as err:
            response = jsonify({'message': err.stderr.decode("UTF-8")})
            response.status_code = 500
        return response
    else:
        return render_template('index.html')

if __name__ == '__main__':
    app.run(debug=True, host='127.0.1.2', port=5000)
