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


//Temperature setup
#define TempSensorPin 17
OneWire oneWire(TempSensorPin);
DallasTemperature sensors(&oneWire);
float Celcius=0;
float Fahrenheit=0;


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

//Pluviometer setup
uint8_t hallSensorPin   = 25;
bool state = HIGH;
bool stateNow = HIGH;
int rain = 0;

//Time setup
unsigned long startMillis;
unsigned long currentMillis;
unsigned long printMillis;
unsigned long temperatureMillis;
//const unsigned long printPeriod = 1000;
const unsigned long timeadjPeriod = 86400000;
const unsigned long temperaturePeriod = 60000;

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
  sensors.begin();

  //SD card configuration
  SPIClass spi = SPIClass(HSPI);
  spi.begin(SCK, MISO, MOSI, CS);
  
  Serial.print("Initializing SD card...");
    if (!SD.begin(CS)) {
      Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");

  createTemperatureFile();
  createRainfallFile();
  
  //Pluviometer configuration
  pinMode(hallSensorPin, INPUT);

  //Temperature configuration
  startMillis = millis();
  printMillis = millis();
  temperatureMillis = millis();

  //WiFi configuration
  Wire.begin();
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(wifiTries != 120) {
    if (WiFi.status() != WL_CONNECTED){
          delay(500);
          Serial.print(".");
          wifiTries++;
    } else{
        Serial.println("");
        Serial.print("Connected to WiFi network with IP Address: ");
        Serial.println(WiFi.localIP());
    }
  }

  //RTC configuration
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
  adjustRTC();  
}

void loop() { 
  currentMillis = millis();
  if (currentMillis - startMillis >= timeadjPeriod) {
    adjustRTC();
    startMillis = currentMillis;
  }

  if (currentMillis - temperatureMillis >= temperaturePeriod) {
    sensors.requestTemperatures();
    Celcius=sensors.getTempCByIndex(0);
    Fahrenheit=sensors.toFahrenheit(Celcius);
    RTCTimeNow();
    Serial.print(DateAndTimeString);
    Serial.print("  ");
    Serial.print(Fahrenheit);
    Serial.println();
    temperatureMillis = currentMillis;
    saveTemperatureValue();
  }

  precipitation();
  
}

void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%Y-%m-%d %H:%M:%S");
}

byte getSeconds() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    time_error = true;
    return time_error;
  }
  sec = timeinfo.tm_sec;
  return sec; 
}

byte getMinutes() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    time_error = true;
    return time_error;
  }
  mins = timeinfo.tm_min;
  return mins; 
}

byte getHours() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    time_error = true;
    return time_error;
  }
  hr = timeinfo.tm_hour;
  return hr; 
}

byte getDays() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    time_error = true;
    return time_error;
  }
  d = timeinfo.tm_mday;
  return d; 
}

byte getMonths() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    time_error = true;
    return time_error;
  }
  m = timeinfo.tm_mon + 1;
  return m; 
}

int getYears() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
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
    Serial.println("Error adjusting the RTC");
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

void precipitation(){
  stateNow = digitalRead(hallSensorPin);
  if (state != stateNow){
    state = digitalRead(hallSensorPin);
    if (state == HIGH){
      rain++;
      RTCTimeNow();
      Serial.print(DateAndTimeString);
      Serial.print("  ");
      Serial.print(rain);
      Serial.println();
      saveRainfallValue();
    }   
  }
  
}

void createTemperatureFile(){
  myFile = SD.open("/temperature.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    header = String("Date") + "," + String("Temperature"), + "\r\n";
    myFile.println(header.c_str());
    myFile.close();
    Serial.println("Temperature File Created");
  } else {
    Serial.println("Error opening the temperature.txt file");
  }
}

void createRainfallFile(){
  myFile = SD.open("/rainfall.txt", FILE_WRITE);
  // if the file opened okay, write to it:
  if (myFile) {
    header = String("Date") + "," + String("Rainfall"), + "\r\n";
    myFile.println(header.c_str());
    myFile.close();
    Serial.println("Rainfall File Created");
  } else {
    Serial.println("Error opening the rainfall.txt file");
  }
}

void saveTemperatureValue(){
  myFile = SD.open("/temperature.txt", FILE_APPEND);
  if (myFile) {
    data_str = String(DateAndTimeString) + "," + String(Fahrenheit), + "\r\n";
    myFile.println(data_str.c_str());
    myFile.close();
    Serial.println("Appendend to the temperature.txt file");
  } else {
      Serial.println("Error opening temperature.txt file");
  }
}

void saveRainfallValue(){
  myFile = SD.open("/rainfall.txt", FILE_APPEND);
  if (myFile) {
    data_str = String(DateAndTimeString) + "," + String(rain), + "\r\n";
    myFile.println(data_str.c_str());
    myFile.close();
    Serial.println("Appendend to the rainfall.txt file");
  } else {
      Serial.println("Error opening rainfall.txt file");
  }
}
