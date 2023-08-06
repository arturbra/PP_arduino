/*
* Arduino Date adjustment and monitoring to PP project
*
* Crated by Jose Artur Teixeira Brasil,
* jose.brasil@utsa.edu
*
* 
*
*/

//Libraries
#include <Wire.h>
#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "TimeManager.h"

//Temperature 1 setup
#define TempSensorPin 17
#define OnBoardLED 2
OneWire oneWire(TempSensorPin);
DallasTemperature sensors(&oneWire);
float Celcius=0;
float Fahrenheit=0;

//Temperature 2 setup
#define TempSensorPin2 4
OneWire oneWire2(TempSensorPin2);
DallasTemperature sensors2(&oneWire2);
float Celcius2=0;
float Fahrenheit2=0;

//Temperature calibration factors:
float f1 = 0.9857;
float f4 = 0.9837;

// RTC variables
const long gmtOffset_sec = -18000;
const int daylightOffset_sec = 0;
TimeManager timeManager("pool.ntp.org", gmtOffset_sec, daylightOffset_sec);
const unsigned long timeadjPeriod = 86400000;

// WiFi variables
const char* ssid = "ONEPLUS_co_apyjsi";
const char* password = "yjsi5896";
int wifiTries = 0;

//Time setup
unsigned long startMillis;
unsigned long currentMillis;
unsigned long temperatureMillis;
unsigned long blinkMillis;
unsigned long blinkCurrentMillis;
unsigned long lastRainfallMillis = 0; // Variable to hold the last time the reading was taken
unsigned long debounceTime = 100; // Time interval to sum up values (in milliseconds)

bool blinkState = 0;
const unsigned long temperaturePeriod = 300000;
const unsigned long blinkPeriod = 1000;

//SD card setup
#define SCK  18
#define MISO  19
#define MOSI  23
#define CS  13

#include <SPI.h>
#include <SD.h>

File myFile;
String header;
String data_str;

void setup() {
  Serial.begin(115200);

  sensors.begin();
  sensors2.begin();
  pinMode(OnBoardLED, OUTPUT);

  //Temperature configuration
  startMillis = millis();
  temperatureMillis = millis();
  blinkMillis = millis();

  //WiFi configuration
  Wire.begin();
  WiFi.begin(ssid, password);
  Serial.println("Connecting to the Wi-Fi");
  digitalWrite(OnBoardLED, HIGH);
  while(wifiTries != 120) {
    if (WiFi.status() != WL_CONNECTED){
          delay(500);
          Serial.print(".");
          wifiTries++;
    } else{
        Serial.println("");
        Serial.print("Connected to WiFi network with IP Address: ");
        Serial.println(WiFi.localIP());
        break;
    }
  }

  if (WiFi.status() != WL_CONNECTED){
    Serial.println("Wi-Fi not connected succesfully");
  }
  
  //SD card configuration
  SPI.begin(SCK, MISO, MOSI, CS);
  
  Serial.println("Initializing SD card");
    if (!SD.begin(CS)) {
      Serial.println("SD card initialization failed!");
    while (1);
  }
  Serial.println("SD card initialization done");

  createTemperatureFile(1);
  createTemperatureFile(4);

  //RTC configuration
  timeManager.init();
  digitalWrite(OnBoardLED, LOW);
}

void loop() { 
  currentMillis = millis();
  blinkCurrentMillis = millis();
  bool wifiConnected = (WiFi.status() == WL_CONNECTED);

  if (currentMillis - startMillis >= timeadjPeriod) {
    timeManager.adjustRTC(wifiConnected);
    startMillis = currentMillis;
  }

  if (currentMillis - temperatureMillis >= temperaturePeriod) {
    // Reading temperature from the first sensor
    sensors.requestTemperatures(); 
    Celcius=sensors.getTempCByIndex(0);
    Fahrenheit=sensors.toFahrenheit(Celcius);
    Fahrenheit = Fahrenheit * f1;

// 
    // Reading temperature from the second sensor
    sensors2.requestTemperatures();
    Celcius2=sensors2.getTempCByIndex(0);
    Fahrenheit2=sensors2.toFahrenheit(Celcius2);
    Fahrenheit2 = Fahrenheit2 * f4;

    // Getting the current time
    String DateAndTimeString = timeManager.getDateTime();

    // Logging the first temperature
    Serial.print(DateAndTimeString);
    Serial.print(" Sensor 1: ");
    Serial.print(Fahrenheit);
    Serial.print(" Sensor 2: ");
    Serial.print(Fahrenheit2);
    Serial.println();

    temperatureMillis = currentMillis;

    // Saving the temperatures to their respective files
    saveTemperatureValue(1, Fahrenheit, DateAndTimeString);
    saveTemperatureValue(4, Fahrenheit2, DateAndTimeString);
  }

  if (blinkState){
    digitalWrite(OnBoardLED, HIGH);
    if (blinkCurrentMillis - blinkMillis >= blinkPeriod){
      digitalWrite(OnBoardLED, LOW);  
      blinkState = 0;
    }
  }
}

void createTemperatureFile(int sensorNumber){
  String filename = "/box_a_temperature_" + String(sensorNumber) + ".txt";
  myFile = SD.open(filename.c_str(), FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    header = String("Date") + "," + "Temperature" + String(sensorNumber) + "\r\n";
    myFile.println(header.c_str());
    myFile.close();
    Serial.println("Temperature File " + String(sensorNumber) + " Created");
  } else {
    Serial.println("Error opening the box_a_temperature_" + String(sensorNumber) + ".txt file");
  }
}

void saveTemperatureValue(int sensorNumber, float temperature, const String& DateAndTimeString){
  String filename = "/box_a_temperature_" + String(sensorNumber) + ".txt";
  File myFile = SD.open(filename.c_str(), FILE_APPEND);
  if (myFile) {
    String data_str = DateAndTimeString + "," + String(temperature) + "\r\n";
    myFile.println(data_str.c_str());
    myFile.close();
    Serial.println("Appended to the box_a_temperature_" + String(sensorNumber) + ".txt file");
  } else {
    Serial.println("Error opening box_a_temperature_" + String(sensorNumber) + ".txt file");
  }
}