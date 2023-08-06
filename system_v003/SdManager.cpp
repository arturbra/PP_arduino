#include "SdManager.h"

SdManager::SdManager(int SCK, int MISO, int MOSI, int CS) 
  : _SCK(SCK), _MISO(MISO), _MOSI(MOSI), _CS(CS) {
}

void SdManager::begin() {
  SPI.begin(_SCK, _MISO, _MOSI, _CS);
  if (!SD.begin(_CS)) {
    Serial.println("SD card initialization failed!");
    while (1);
  }
}

void SdManager::createTemperatureFile(int sensorNumber) {
  String filename = "/box_a_temperature_" + String(sensorNumber) + ".txt";
  _file = SD.open(filename.c_str(), FILE_WRITE);
  if (_file) {
    String header = String("Date") + "," + "Temperature" + String(sensorNumber) + "\r\n";
    _file.println(header.c_str());
    _file.close();
  } else {
    Serial.println("Error opening the box_a_temperature_" + String(sensorNumber) + ".txt file");
  }
}

void SdManager::saveTemperatureValue(int sensorNumber, float temperature, const String& DateAndTimeString) {
  String filename = "/box_a_temperature_" + String(sensorNumber) + ".txt";
  _file = SD.open(filename.c_str(), FILE_APPEND);
  if (_file) {
    String data_str = DateAndTimeString + "," + String(temperature) + "\r\n";
    _file.println(data_str.c_str());
    _file.close();
  } else {
    Serial.println("Error opening box_a_temperature_" + String(sensorNumber) + ".txt file");
  }
}
