/*
* Arduino Date adjustment and monitoring to PP project
*
* Crated by Jose Artur Teixeira Brasil,
* jose.brasil@utsa.edu
*
* 
*
*/

#include "WifiManager.h"
#include "SdManager.h"
#include "TemperatureSensor.h"
#include "TimeManager.h"
#include "MqttManager.h"
#include "PluviometerManager.h"

//Temperature Sensor setup
#define TEMP_SENSOR_PIN1 17
#define TEMP_SENSOR_PIN2 4
TemperatureSensor tempSensor1(TEMP_SENSOR_PIN1, 0.9857);
TemperatureSensor tempSensor2(TEMP_SENSOR_PIN2, 0.9837);

// RTC variables
const long gmtOffset_sec = -18000;
const int daylightOffset_sec = 0;
TimeManager timeManager("pool.ntp.org", gmtOffset_sec, daylightOffset_sec);
const unsigned long timeadjPeriod = 86400000;

// WiFi variables
const char* ssid = "ONEPLUS_co_apyjsi";
const char* password = "yjsi5896";
WifiManager wifiManager(ssid, password);

// MQTT variables
const char* mqtt_server = "";
uint16_t mqtt_port = 1880;
const char* mqtt_user = "";
const char* mqtt_password = "";
MqttManager mqttManager(mqtt_server, mqtt_port, mqtt_user, mqtt_password);

// SD card setup
#define SCK  18
#define MISO  19
#define MOSI  23
#define CS  13
SdManager sdManager(SCK, MISO, MOSI, CS);

//Time setup
unsigned long startMillis;
unsigned long currentMillis;

// Pluviometer setup
#define HALL_SENSOR_PIN 16
PluviometerManager pluviometerManager(HALL_SENSOR_PIN, mqttManager, timeManager);

void setup() {
  Serial.begin(115200);

  // Temperature configuration
  tempSensor1.begin();
  tempSensor2.begin();

  //WiFi configuration
  wifiManager.begin();

  //SD card configuration
  sdManager.begin();
  sdManager.createTemperatureFile(1);
  sdManager.createTemperatureFile(4);

  //RTC configuration
  timeManager.init();

  //MQTT configuration
  mqttManager.begin();

  // Pluviometer Configuration
  pluviometerManager.setupMQTTClient()
}

void loop() { 
  currentMillis = millis();

  if (currentMillis - startMillis >= timeadjPeriod) {
    timeManager.adjustRTC(wifiManager.isConnected());
    startMillis = currentMillis;
  }

  if (tempSensor1.isTemperatureReady(currentMillis) && tempSensor2.isTemperatureReady(currentMillis)) {
    float Fahrenheit1 = tempSensor1.readTemperature();
    float Fahrenheit2 = tempSensor2.readTemperature();

    // Getting the current time
    String DateAndTimeString = timeManager.getDateTime();

    // Logging the temperatures
    Serial.print(DateAndTimeString);
    Serial.print(" Sensor 1: ");
    Serial.print(Fahrenheit1);
    Serial.print(" Sensor 2: ");
    Serial.print(Fahrenheit2);
    Serial.println();

    // Saving the temperatures to their respective files
    sdManager.saveTemperatureValue(1, Fahrenheit1, DateAndTimeString);
    sdManager.saveTemperatureValue(4, Fahrenheit2, DateAndTimeString);

    // Sending the temperatures via MQTT
    mqttManager.publishTemperature(1, Fahrenheit1, DateAndTimeString);
    mqttManager.publishTemperature(4, Fahrenheit2, DateAndTimeString);

    pluviometerManager.checkRainfall();
  }
}
