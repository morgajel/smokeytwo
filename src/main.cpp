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
#include <InfluxDbClient.h>
#include "pins.h"


SoftwareSerial LCD(deadpin, lcdPin);

#define cookhightemp 230
#define cooklowtemp 220
#define backoff_interval 3000  // how often to wait before evaluating the relay again
#define thermocouple_delay 500 // delay each loop

// Values that change often
volatile int backoff_timer = 0 ; // used to slow how often the switch is thrown
volatile int case_temp;          // Case Thermocouple temp
volatile int meat_temp;          // Meat Thermocouple temp

// Objects
MAX6675 case_sensor(thermo1CLK, thermo1CS, thermo1S0); 
MAX6675 meat_sensor(thermo2CLK, thermo2CS, thermo2S0); 
ESP8266WebServer server(80);
InfluxDBClient influx_client;
Point sensor("smoker_status");


// Webserver handlers
void handle_OnConnect() {
    server.send(200, "text/html", indexPage);
}

void handle_NotFound(){
    server.send(404, "text/plain", "Not found");
}

void getdata(){
    String data = " {";
    data += "    maxtemp: "+ String(cookhightemp)+",";
    data += "    mintemp: "+ String(cooklowtemp)+",";
    data += "    interval: "+ String(backoff_interval)+",";
    data += "    case_temp: "+ String(case_temp)+",";
    data += "    meat_temp: "+ String(meat_temp)+",";
    data += "    enabled: true";
    data += "}";
    server.send(206, "text/json", data);
}


void setup(){
    Serial.begin(115200);
    Serial.println("Booting");
    //delay(500);
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
    Serial.print("Got IP: ");    Serial.println(WiFi.localIP());

    influx_client.setConnectionParamsV1(INFLUXDB_URL, INFLUXDB_DB, INFLUXDB_USER, INFLUXDB_PASS);

    sensor.addTag("device", "SmokeyTwo");
    sensor.addTag("SSID", WiFi.SSID());
    sensor.addTag("cookhightemp", String(cookhightemp));
    sensor.addTag("cooklowtemp", String(cooklowtemp));
    sensor.addTag("backoff_interval", String(backoff_interval));
    sensor.addTag("thermocouple_delay", String(thermocouple_delay));

    server.on("/", handle_OnConnect);
    server.on("/getdata", getdata);
    server.onNotFound(handle_NotFound);

    server.begin();
    Serial.println("HTTP server started!");

}

void loop() {
    sensor.clearFields();
    server.handleClient();
    sensor.addField("rssi", WiFi.RSSI());

    //Serial.println("loopin");
    case_temp = case_sensor.readFarenheit();
    meat_temp = meat_sensor.readFarenheit();
    sensor.addField("Case Temp", case_temp);
    LCD.flush();
    LCD.write(254); // moves to char 1
    LCD.write(128); // moves to line 1

    String text = "Case: ";
    text += (int) case_temp;
    text += (char)223;
    text += "F     ";
    LCD.write(text.c_str());

    LCD.write(254); // moves to char 1
    LCD.write(192); // moves to line 2

    String text2 =    "Meat: ";
    text2 += (int) meat_temp;
    text2 += (char)223;
    text2 += "F     ";
    LCD.write(text2.c_str());

    if (backoff_timer <= 0) {
        // if the backoff_time has expired, evaluate case temp
        // to determine if the heat needs to be turned on
        if ( (int) case_temp  < cooklowtemp && digitalRead(power_control_pin) == LOW ){
            Serial.println("Temp is too low, turn on heat");
            digitalWrite(power_control_pin, HIGH);

        } else if ((int) case_temp > cookhightemp && digitalRead(power_control_pin) == HIGH){
            Serial.println("Temp is too hot, turn off heat");
            digitalWrite(power_control_pin, LOW);
        } else{
            Serial.printf("case_temp Status: %d-->[%d]-->%d, ",cooklowtemp, case_temp, cookhightemp);
            if (digitalRead(power_control_pin)){
                Serial.println("Heater: on");
            }else{
                Serial.println("Heater: off");
            }
        }
        // reset backoff interval
        backoff_timer = backoff_interval;
        // attempt to write data to influxdb
        if (!influx_client.writePoint(sensor)) {
            Serial.print("InfluxDB write failed: ");
            Serial.println(influx_client.getLastErrorMessage());
        }
    } else {
        // keep the countdown going.
        backoff_timer = backoff_timer - thermocouple_delay;
    }
    // This delay is needed between thermocouple reads.
    delay(thermocouple_delay);
}
