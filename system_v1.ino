/*
* Arduino Date adjustment to PP project
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



void setup() {
  sensors.begin();

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
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

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
    }   
  }
}
