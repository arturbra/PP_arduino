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
#include <DS3231.h>
#include <Wire.h>
#include <time.h>
#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>


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

float f3 = 0.9924;
float f2 = 0.9900;

//Date and time RTC setup
DS3231 RTC;
RTClib myRTC;

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -18000;
const int daylightOffset_sec = 0;
const char* ssid = "ONEPLUS_co_apyjsi";
const char* password = "yjsi5896";
char DateAndTimeString[20];
char DateAndTimeNow[20];
byte seconds;
byte sec;
byte minutes;
byte mins;
byte hours;
byte hr;
byte day;
byte d;
byte month;
byte m;
int year;
int y;
int wifiTries = 0;
boolean time_error = false;

//Time setup
unsigned long startMillis;
unsigned long currentMillis;
unsigned long temperatureMillis;
unsigned long blinkMillis;
unsigned long blinkCurrentMillis;
unsigned long lastRainfallMillis = 0; // Variable to hold the last time the reading was taken
unsigned long debounceTime = 100; // Time interval to sum up values (in milliseconds)

bool blinkState = 0;
const unsigned long timeadjPeriod = 86400000;
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

// LED and Button
#define LED1 26
#define LED2 25
#define BUTTON_PIN 27

bool buttonState = false;
bool lastButtonState = false;

unsigned long lastLEDChange = 0;
unsigned long blinkInterval = 500; // for alternating blink during Wi-Fi connect

bool led1State = LOW;
bool led2State = LOW;
bool ledState = false;
bool wifiConnected = false;

void setup() {
  Serial.begin(115200);

  sensors.begin();
  sensors2.begin();
  pinMode(OnBoardLED, OUTPUT);

  //Temperature configuration
  startMillis = millis();
  temperatureMillis = millis();
  blinkMillis = millis();

  //LED and Button
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  //WiFi configuration
  Wire.begin();
  WiFi.begin(ssid, password);
  Serial.println("Connecting to the Wi-Fi");
  digitalWrite(OnBoardLED, HIGH);

  while (!wifiConnected && wifiTries < 60) {
      if (WiFi.status() != WL_CONNECTED) {
          if (millis() - lastLEDChange >= blinkInterval) {
              // Alternate the LEDs
              if (ledState) {
                  digitalWrite(LED1, LOW);
                  digitalWrite(LED2, HIGH);
              } else {
                  digitalWrite(LED1, HIGH);
                  digitalWrite(LED2, LOW);
              }
              ledState = !ledState;
              lastLEDChange = millis();
              wifiTries++;
          }
      } else {
          wifiConnected = true;
      }
  }

  if (wifiConnected) {
    // Blink both LEDs twice to indicate successful connection
    for (int i = 0; i < 2; i++) {
        digitalWrite(LED1, HIGH);
        digitalWrite(LED2, HIGH);
        delay(200);
        digitalWrite(LED1, LOW);
        digitalWrite(LED2, LOW);
        delay(200);
    }
  } else {
      Serial.println("Failed to connect to WiFi after 120 tries");
  }
  
  //SD card configuration
  SPI.begin(SCK, MISO, MOSI, CS);
  
  Serial.println("Initializing SD card");
    if (!SD.begin(CS)) {
      Serial.println("SD card initialization failed!");
    while (1){
      digitalWrite(LED1, HIGH);  // turn LED1 ON
      digitalWrite(LED2, HIGH);  // turn LED2 ON
      delay(500);                // wait for 500ms
      digitalWrite(LED1, LOW);   // turn LED1 OFF
      digitalWrite(LED2, LOW);   // turn LED2 OFF
      delay(500);                // wait for 500ms
    }
  }
  Serial.println("SD card initialization done");

  createTemperatureFile(1);
  createTemperatureFile(2);
  createTestFile();

  //RTC configuration
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
  adjustRTC();
  digitalWrite(OnBoardLED, LOW);
}

void loop() { 
  currentMillis = millis();
  blinkCurrentMillis = millis();
  
  if (currentMillis - startMillis >= timeadjPeriod) {
    adjustRTC();
    startMillis = currentMillis;
  }

  if (currentMillis - temperatureMillis >= temperaturePeriod) {
    // Reading temperature from the first sensor
    sensors.requestTemperatures(); 
    Celcius=sensors.getTempCByIndex(0);
    Fahrenheit=sensors.toFahrenheit(Celcius);
    Fahrenheit = Fahrenheit * f3;

// 
    // Reading temperature from the second sensor
    sensors2.requestTemperatures();
    Celcius2=sensors2.getTempCByIndex(0);
    Fahrenheit2=sensors2.toFahrenheit(Celcius2);
    Fahrenheit2 = Fahrenheit2 * f2;

    // Getting the current time
    RTCTimeNow();

    // Logging the first temperature
    Serial.print(DateAndTimeString);
    Serial.print(" Sensor 1: ");
    Serial.print(Fahrenheit);
    Serial.print(" Sensor 2: ");
    Serial.print(Fahrenheit2);
    Serial.println();

    temperatureMillis = currentMillis;

    // Saving the temperatures to their respective files
    saveTemperatureValue(1, Fahrenheit);
    saveTemperatureValue(2, Fahrenheit2);
  }

  // Check button press
  bool currentButtonState = digitalRead(BUTTON_PIN);
  
  if (currentButtonState == HIGH && lastButtonState == LOW) {
      // button has been pressed
      delay(50);  // debounce delay
      if(digitalRead(BUTTON_PIN) == HIGH) {  // check the button state again after the debounce delay
          Serial.println("Button Pressed!");  // Debug message for button press
          
          // Save value to SD card and blink if succeed
          RTCTimeNow();
          temperatureTest(1, Fahrenheit);
          temperatureTest(2, Fahrenheit2);
      }
  }
  lastButtonState = currentButtonState;
}

void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time with the online server");
    return;
  }
  Serial.println(&timeinfo, "%Y-%m-%d %H:%M:%S");
}

byte getSeconds() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Error obtaining the date and time with the online server. Possibly the Wi-Fi is not connected. The RTC time will be used instead");
    time_error = true;
    return time_error;
  }
  sec = timeinfo.tm_sec;
  return sec; 
}

byte getMinutes() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Error obtaining the date and time with the online server. Possibly the Wi-Fi is not connected. The RTC time will be used instead");
    time_error = true;
    return time_error;
  }
  mins = timeinfo.tm_min;
  return mins; 
}

byte getHours() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Error obtaining the date and time with the online server. Possibly the Wi-Fi is not connected. The RTC time will be used instead");
    time_error = true;
    return time_error;
  }
  hr = timeinfo.tm_hour;
  return hr; 
}

byte getDays() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Error obtaining the date and time with the online server. Possibly the Wi-Fi is not connected. The RTC time will be used instead");
    time_error = true;
    return time_error;
  }
  d = timeinfo.tm_mday;
  return d; 
}

byte getMonths() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Error obtaining the date and time with the online server. Possibly the Wi-Fi is not connected. The RTC time will be used instead");
    time_error = true;
    return time_error;
  }
  m = timeinfo.tm_mon + 1;
  return m; 
}

int getYears() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Error obtaining the date and time with the online server. Possibly the Wi-Fi is not connected. The RTC time will be used instead");
    time_error = true;
    return time_error;
  }
  y = timeinfo.tm_year - 100;
  return y; 
}

void adjustRTC(){
  Serial.println("Adjusting the RTC timer based on the pool.ntp.org time");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  seconds = getSeconds();
  minutes = getMinutes();
  hours = getHours();
  day = getDays();
  month = getMonths();
  year = getYears();
  if (time_error == true){
    Serial.println("Error adjusting the RTC with the online server. Possibly the Wi-Fi is not connected");
    time_error = false;
    return;
  } 
  else {
    Serial.println("Sucessfully adjusted the time");
    RTC.setClockMode(false);
    RTC.setYear(year);
    RTC.setMonth(month);
    RTC.setDate(day);
    RTC.setHour(hours);
    RTC.setMinute(minutes);
    RTC.setSecond(seconds);
  }
}

void RTCTimeNow() {
  DateTime now = myRTC.now();
  byte n_month = now.month();
  byte n_day = now.day();
  int n_year = now.year();
  byte n_hour = now.hour();
  byte n_minute = now.minute();
  byte n_second = now.second();
  
  sprintf_P(DateAndTimeString, PSTR("%4d-%02d-%02d %02d:%02d:%02d"), n_year, n_month, n_day, n_hour, n_minute, n_second);
}

void createTemperatureFile(int sensorNumber){
  String filename = "/box_c_temperature_" + String(sensorNumber) + ".txt";
  myFile = SD.open(filename.c_str(), FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    header = String("Date") + "," + "Temperature" + String(sensorNumber) + "\r\n";
    myFile.println(header.c_str());
    myFile.close();
    Serial.println("Temperature File " + String(sensorNumber) + " Created");
  } else {
    Serial.println("Error opening the temperature_" + String(sensorNumber) + ".txt file");
  }
}

void saveTemperatureValue(int sensorNumber, float temperature){
    String filename = "/box_c_temperature_" + String(sensorNumber) + ".txt";
    myFile = SD.open(filename.c_str(), FILE_APPEND);
    if (myFile) {
        data_str = String(DateAndTimeString) + "," + String(temperature) + "\r\n";
        myFile.println(data_str.c_str());
        myFile.close();
        Serial.println("Appended to the temperature_" + String(sensorNumber) + ".txt file");
        if (sensorNumber == 1) {
            digitalWrite(LED1, HIGH);
            delay(100); // short blink
            digitalWrite(LED1, LOW);
        } else if (sensorNumber == 2) {
            digitalWrite(LED2, HIGH);
            delay(100); // short blink
            digitalWrite(LED2, LOW);
        }
    } else {
        Serial.println("Error opening temperature_" + String(sensorNumber) + ".txt file");
    }
}

void createTestFile(){
  String filename = "/test.txt";
  myFile = SD.open(filename.c_str(), FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.println("Test File Created");
  } else {
    Serial.println("Error opening the button_presses.txt file");
  }
}

void temperatureTest(int sensorNumber, float temperature){
  String filename = "/test.txt";
  myFile = SD.open(filename.c_str(), FILE_APPEND);
  if (myFile) {
      data_str = String(DateAndTimeString) + "," + String(temperature) + "\r\n";
      myFile.println(data_str.c_str());
      myFile.close();
      Serial.println("Appended to the temperature_" + String(sensorNumber) + ".txt file");
      if (sensorNumber == 1) {
          digitalWrite(LED1, HIGH);
          delay(100); // short blink
          digitalWrite(LED1, LOW);
      } else if (sensorNumber == 2) {
          digitalWrite(LED2, HIGH);
          delay(100); // short blink
          digitalWrite(LED2, LOW);
      }
  } else {
      Serial.println("Error opening temperature_" + String(sensorNumber) + ".txt file");
  }
}
