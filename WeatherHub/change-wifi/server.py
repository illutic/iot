import os
import subprocess
from subprocess import CalledProcessError, check_output
from flask import Flask, render_template, request, jsonify

app = Flask(__name__, template_folder='templates')

@app.route('/', methods=['GET', 'POST'])
def index():
    if request.method == 'POST':
        ssid = request.json['ssid']
        password = request.json['password']
        response = jsonify({'message': 'Processing request...'})
        response.status_code = 202
        try:
            output = check_output([f'{os.getcwd()}/change-wifi/scripts/configure.sh', '-a', 'WeatherHub', 'password', '-c', ssid, password])
            print(output.decode('UTF-8'))
            response = jsonify({'message': output.decode('UTF-8')})
            response.status_code = 200
        except CalledProcessError:
            response = jsonify({'message': str(CalledProcessError.output)})
            response.status_code = 500
        return response
    else:
        return render_template('index.html')

if __name__ == '__main__':
    app.run(debug=True)
