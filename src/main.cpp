#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <Adafruit_Sensor.h>
#include <ESP8266WebServer.h>
#include <max6675.h>
#include "creds.h"
#include "ota.h"
#include <WiFiUdp.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include "index_html.h"

// A total of six wires are needed for thermocouples
int thermo1S0 = 5; // s0 is yellow wire
int thermo1CS = 4;  // cs is blue wire
int thermo1CLK = 2; // clk is green wire
int thermo2S0 = 12; // s0 is yellow wire
int thermo2CS = 13;  // cs is blue wire
int thermo2CLK = 15; // clk is green wire

// LCD needs 2 pins
int lcdPin = 14;
SoftwareSerial LCD(10, lcdPin);

int cookhightemp = 180;
int cooklowtemp = 170;
int backoff_timer = 0; // used to slow how often the switch is thrown
int backoff_interval = 5000; 
int thermocouple_delay = 250;
int power_control_pin = 16;

MAX6675 thermocouple1(thermo1CLK, thermo1CS, thermo1S0); 
MAX6675 thermocouple2(thermo2CLK, thermo2CS, thermo2S0); 

ESP8266WebServer server(80);


void handle_OnConnect() {
  server.send(200, "text/html", indexPage); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}


void getdata(){
  int t1 = (int) thermocouple1.readFarenheit(); // Gets the values of the temperature
  int t2 = (int) thermocouple2.readFarenheit(); // Gets the values of the temperature
String data = " {";
data += "  maxtemp: "+ String(cookhightemp)+",";
data += "  mintemp: "+ String(cooklowtemp)+",";
data += "  interval: "+ String(backoff_interval)+",";
data += "  t1: "+ String(t1)+",";
data += "  t2: "+ String(t2)+",";
data += "  enabled: true";
data += "}";
      server.send(206, "text/json", data);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  delay(500);
  LCD.begin(9600);
  LCD.flush();
  
  pinMode(power_control_pin, OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, passphrase);
  Serial.println("connecting");
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.print(".");
    delay(5000);
  }
  ota_setup(hostname, password);

  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());

  server.on("/", handle_OnConnect);
  server.on("/getdata", getdata);

  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started!");


}

void loop() {
  server.handleClient();
  //Serial.println("loopin");
  float t1 = thermocouple1.readFarenheit(); // Gets the values of the temperature
  float t2 = thermocouple2.readFarenheit(); // Gets the values of the temperature
  LCD.flush();
  LCD.write(254);
  LCD.write(128);

  String text = "T1: ";
  text += (int) t1;
  text += (char)223;
  text += "F   ";
  LCD.write(text.c_str());
  Serial.println(text);

  LCD.write(254);
  LCD.write(192);
  text = "T2: ";
  text += (int) t2;
  text += (char)223;
  text += "F   ";

  LCD.write(text.c_str());
  Serial.println(text);
  Serial.println(backoff_timer);

  if (backoff_timer <= 0){
    if (((int) t2  < cooklowtemp) || ((int) t1  < cooklowtemp)){
      Serial.println("Temp is too low, turn on heat");
      digitalWrite(power_control_pin, HIGH);

    } else if (((int) t2  < cookhightemp) || ((int) t1  > cookhightemp)){
      Serial.println("Temp is too hot, turn off heat");
      Serial.println(backoff_timer);
      
      digitalWrite(power_control_pin, LOW);
    }
    backoff_timer = backoff_interval;
  } else{
    backoff_timer = backoff_timer - thermocouple_delay;
  }
  // This delay is needed between thermocouple reads.
  delay(thermocouple_delay);
}
