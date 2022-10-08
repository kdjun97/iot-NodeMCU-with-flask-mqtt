#include "EspMQTTClient.h"
#include "DHTesp.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//GPIO Pin Mapping 부분이다.
#define D0 16 //LED
#define D1 5 //OLED SCL
#define D2 4 //OLED SDA
#define D3 0 //DHT22
#define D4 2 //Relay
#define D5 14
#define D6 12
#define D7 13
#define D8 15
#define LED_PIN D0
#define LED_ON HIGH // active-HIGH
#define LED_OFF LOW

// 과제를 위한 맵핑 부분
#define LIGHT_PIN A0 // Light 핀 맵핑
#define LED_PIN D0 // LED 핀 맵핑
#define DHTPIN D3 // DHT 핀 맵핑
#define RELAY1_PIN D4 // HW1을 위해 D4로 맵핑 변경
#define RELAY_OFF HIGH // HIGH 신호이면 릴레이 동작하지 않음
#define RELAY_ON LOW // LOW 신호이면 릴레이 동작함

// case를 정의 (subscribe case)
#define CASE_LIGHT_LED "light/led" // led toggle
#define CASE_LIGHT_LED_ON "light/ledon" // led on
#define CASE_LIGHT_LED_OFF "light/ledoff" // led off
#define CASE_LIGHT_USB_LED "light/usbled" // usb led toggle
#define CASE_LIGHT_USB_LED_ON "light/usbledon" // usb led on
#define CASE_LIGHT_USB_LED_OFF "light/usbledoff" // usb led off

//MQTT관련 선언
#define mqtt_broker "***.***.***.***"
#define mqtt_username "iot"
#define mqtt_password "******"
#define mqtt_clientname "jumy" // should be changed

//WiFi 관련 선언
const char *wifi_ssid = "*****"; // should be changed
const char *wifi_password = "*****"; // should be changed

EspMQTTClient client(
  wifi_ssid, //연결된 와이파이
  wifi_password, //연결된 와이파이 비밀 번호
  mqtt_broker, // MQTT Broker server ip
  mqtt_username, //broker에 접속하는 아이디
  mqtt_password, //broker에 접속하는 비밀번호
  mqtt_clientname, //유일한 기기를 사용하는 사용자
  1883 // The MQTT port, default to 1883. this line can be omitted
);

//Subscribe Topic for MQTT
const char *sub_topic = "iot/21600057";

// led, usbled의 상태를 위한 함수
int led_state = LED_OFF; //led_state는 led의 상태
int usb_led_state = RELAY_OFF; // 초기 상태는 불 꺼진 상태

// 각종 static var
String msgString;
int light_value; // cds 센서로 부터 받는 light value
float humidity, temperature; // 습도, 온도 변수
char str_temperature[40], str_humidity[40];

// 시간에 대한 변수
unsigned long time_snapshot1; 

// led, usbled 변화 bool 값
bool led_change=false;
bool usb_led_change=false;

DHTesp dht; // dht 

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(D0, OUTPUT);
  pinMode(LIGHT_PIN, INPUT);
  pinMode (LED_PIN, OUTPUT);
  pinMode (RELAY1_PIN, OUTPUT); // 릴레이 signal 단자
  digitalWrite(RELAY1_PIN, RELAY_OFF);
  dht.setup(DHTPIN, DHTesp::DHT22); // hw1에서 요구했던 DHTPIN(D3)에 맵핑 

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  Serial.println(F("SSD1306 allocation successed"));

  time_snapshot1 = millis();
}

void onConnectionEstablished() { //client가 서버와 연결되면 loop를 돌면서 이 함수가 호출됨
  Serial.println("Connection Established!");
  
  client.subscribe(sub_topic, [](const String & topic, const String & payload)
  {
    Serial.println(topic + ": " + payload);
    msgString = payload;

    // light에 관련된 subscribe
    if (msgString.equals(CASE_LIGHT_LED)) {led_control(1); led_change=true;}
    else if (msgString.equals(CASE_LIGHT_LED_ON)) {led_control(2); led_change=true;}
    else if (msgString.equals(CASE_LIGHT_LED_OFF)) {led_control(3); led_change=true;}
    else if (msgString.equals(CASE_LIGHT_USB_LED)) {usb_led_control(1); usb_led_change=true;}
    else if (msgString.equals(CASE_LIGHT_USB_LED_ON)) {usb_led_control(2); usb_led_change=true;}
    else if (msgString.equals(CASE_LIGHT_USB_LED_OFF)) {usb_led_control(3); usb_led_change=true;}

    if (led_change) {
      digitalWrite(D0, led_state);
      led_change=false;
    }
    else if (usb_led_change) {
      digitalWrite(RELAY1_PIN, usb_led_state);
      usb_led_change=false;
    }
    
  });
}

void loop() {
  client.loop();
  if (time_snapshot1 + 1000 <= millis()) {
    publish_cds();    
    fetch_dht22_info();
    write_dht22_info_to_display();
    publish_dht22();
    time_snapshot1 = millis();
  }
}

// led 제어 함수 (temp 값, 1:토글, 2:on, 3:off)
void led_control(int temp) {
  if (temp==1) led_state = !led_state;
  else if (temp==2) led_state = LED_ON;
  else if (temp==3) led_state = LED_OFF;
}

// usb led 제어 함수 (temp 값, 1:토글, 2:on, 3:off)
void usb_led_control(int temp) {
  if (temp==1) usb_led_state = !usb_led_state;
  else if (temp==2) usb_led_state = RELAY_ON;
  else if (temp==3) usb_led_state = RELAY_OFF;
}

// cds 센서 정보 publish 함수
void publish_cds() {
  String cds_topic = "iot/21600057/sensor/cds"; // cds topic
  char msg[50];
  light_value = analogRead(LIGHT_PIN);
  
  sprintf(msg, "Light amount is %d", light_value);
  client.publish(cds_topic, msg);
}

// dht22 정보 publish
void publish_dht22() {
  String dht_all_topic = "iot/21600057/dht22"; // 온습도 topic
  String t_topic = "iot/21600057/dht22_t";// 온도 topic
  String h_topic = "iot/21600057/dht22_h";// 습도 topic

  char msg[50];

  sprintf(msg, "{Humidity : %.2f, Temperature : %.2f}", humidity, temperature);
  client.publish(dht_all_topic, msg);  
  sprintf(msg, "{Temperature : %.2f}", temperature);
  client.publish(t_topic, msg);  
  sprintf(msg, "{Humidity : %.2f}", humidity);
  client.publish(h_topic, msg);    
}

// 온습도 정보 읽기
void fetch_dht22_info() {
  humidity = dht.getHumidity();
  temperature = dht.getTemperature();

  if (!isnan(humidity) && !isnan(temperature)) { // !(not a number), humidity와 temperature가 숫자 값이라면
    Serial.print("Temp:");
    Serial.print(temperature);
    Serial.print("\tHumidity:");
    Serial.println(humidity);
  }
  else
    Serial.println("Fetch DHT22 Data Error!");
}

// OLED에 온습도 정보 출력
void write_dht22_info_to_display() {
  display.clearDisplay();

  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  
  display.setCursor(0,0);             // Start at top-left corner
  display.print(F("T:"));
  display.print(temperature);
  display.println(F("'C"));
  
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
  display.print(F("H:"));
  display.print(humidity);
  display.println(F("%"));
  
  display.display();
}
