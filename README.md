# flask를 이용하여 간단한 IoT 웹 서버 구현

`개발환경 및 툴`  

라즈베리파이4, cds sensor, dht22 sensor, led, usb led, OLED(ssd1306), NodeMCU(ESP8266),  
Arduino IDE, Python3, Chorme  

`소개`  

Python flask 패키지를 이용하여 간단한 IoT용 웹 서버를 구현.  
구현한 웹 페이지에서 MQTT 프로토콜을 이용하여 센서노드의 데이터를 읽어오고 기기를 제어.  

### 주요 기능

`웹 서버`  
LED on, off button으로 led 제어  
USB LED on, off button으로 USB led 제어  
Get temperature, humidity 버튼으로 온습도 가져와서 웹에 출력  
Get light intensity 버튼으로 조도 정보 가져와서 웹에 출력  

URL을 이용하여 led, usb led 제어 (toggle됨)  

`NodeMCU`  
OLED에 온,습도 정보 1초마다 출력  
조도 센서 이용, 밝기 정보 출력  
led 제어  
usb led 제어 (relay 이용)  

`MQTT for NodeMCU`  
온습도, 조도 정보 publish  
led, usbled 에 관련된 on,off topic을 subscribe하여 반응하도록 구현  

`MQTT for flask`  
flask_mqtt 패키지 이용  
온,습도, 조도 정보 subscribe하여 출력  
웹 버튼을 통해 led, usbled control topic 및 msg publish  