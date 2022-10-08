from flask import Flask, render_template
from flask_mqtt import Mqtt
import time

app = Flask(__name__)

app.config['MQTT_BROKER_URL'] = '***.***.***.***'
app.config['MQTT_BROKER_PORT'] = 1883
app.config['MQTT_USERNAME'] = 'iot'
app.config['MQTT_PASSWORD'] = '******'
# app.config['MQTT_KEEPALIVE'] = 5
# app.config['MQTT_TLS_ENABLED'] = False
topic = '/iot/21600057'

mqtt_client = Mqtt(app)

URL_HOME='/'
URL_LED_ON='/LED/ON'
URL_LED_OFF='/LED/OFF'
URL_USB_LED_ON='/USBLED/ON'
URL_USB_LED_OFF='/USBLED/OFF'
URL_LED='/iot/led'
URL_USB_LED='/iot/usbled'
URL_LIGHT='/light'
URL_DHT22='/dht22'

LIGHT_STR=''
DHT_STR=''
light_button = False
dht_button = False

@app.route(URL_HOME)
def homepage():
	return render_template('index.html')

@app.route(URL_LED_ON)
def led_on():
	mqtt_client.publish("iot/21600057", "light/ledon")
	return render_template('index.html')

@app.route(URL_LED_OFF)
def led_off():
	mqtt_client.publish("iot/21600057", "light/ledoff")
	return render_template('index.html')

@app.route(URL_LED)
def led():
	mqtt_client.publish("iot/21600057", "light/led")
	return render_template('index.html')

@app.route(URL_USB_LED_ON)
def usb_led_on():
	mqtt_client.publish("iot/21600057", "light/usbledon")
	return render_template('index.html')

@app.route(URL_USB_LED_OFF)
def usb_led_off():
	mqtt_client.publish("iot/21600057", "light/usbledoff")
	return render_template('index.html')

@app.route(URL_USB_LED)
def usb_led():
	mqtt_client.publish("iot/21600057", "light/usbled")
	return render_template('index.html')

@app.route(URL_LIGHT)
def cds_light():
	global LIGHT_STR
	global light_button
	mqtt_client.unsubscribe('iot/21600057/dht22')
	mqtt_client.subscribe('iot/21600057/sensor/cds')
	light_button=True
	time.sleep(1);
	return render_template('index.html', res=LIGHT_STR)

@app.route(URL_DHT22)
def dht22_info():
	global DHT_STR
	global dht_button
	mqtt_client.unsubscribe('iot/21600057/sensor/cds')
	mqtt_client.subscribe("iot/21600057/dht22")
	dht_button=True
	time.sleep(1);
	return render_template('index.html', res=DHT_STR)

@mqtt_client.on_message()
def handle_mqtt_message(client, userdata, message):
	global LIGHT_STR, DHT_STR
	global light_button, dht_button
	data = dict(
		topic=message.topic,
		payload=message.payload.decode()
	)
	if (data['topic'] == 'iot/21600057/dht22'):
		if (dht_button):
			DHT_STR=data['payload']
			dht_button=False

	elif (data['topic'] == 'iot/21600057/sensor/cds'):
		if (light_button):
			LIGHT_STR=data['payload']
			light_button=False

if __name__ == '__main__':
	app.run(host='0.0.0.0', port=80, debug=True)