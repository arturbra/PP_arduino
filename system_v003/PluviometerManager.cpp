#include "PluviometerManager.h"

PluviometerManager::PluviometerManager(int hallSensorPin, MqttManager& mqttManager, TimeManager& timeManager)
  : _hallSensorPin(hallSensorPin), _mqttManager(mqttManager), _timeManager(timeManager), _rain(0), _lastSensorState(HIGH), _numReadings(0), _sumSensorValues(0), _lastRainfallMillis(0), _debounceTime(100)
{
  pinMode(_hallSensorPin, INPUT);
}

void PluviometerManager::setup() {
  createRainfallFile();
}

void PluviometerManager::checkRainfall() {
  unsigned long currentRainfallMillis = millis(); // Get the current time

  // If 50ms have passed since the last reading
  if (currentRainfallMillis - _lastRainfallMillis >= _debounceTime) {
    int averageSensorValue = 0; // Default value
    
    if (_numReadings != 0) {
      averageSensorValue = _sumSensorValues / _numReadings;
    }

    // If the average sensor value is greater than zero, consider the state as HIGH, else consider it as LOW
    bool currentSensorState = (averageSensorValue > 0) ? HIGH : LOW;

    // If sensor state changes from HIGH to LOW, consider it a magnet pass
    if (_lastSensorState == HIGH && currentSensorState == LOW) {
      _rain++;
      saveRainfallValue();
      publishRainfallData();
    }

    // Update the _lastSensorState and _lastRainfallMillis for the next iteration
    _lastSensorState = currentSensorState;
    _lastRainfallMillis = currentRainfallMillis;

    // Reset the sum and the count
    _sumSensorValues = 0;
    _numReadings = 0;
  }
  else {
    // Add the current sensor value to the sum
    _sumSensorValues += digitalRead(_hallSensorPin);

    // Increment the number of readings
    _numReadings++;
  }
}

void PluviometerManager::publishRainfallData() {
  String payload = "Date: " + _timeManager.getDateTime() + ", Rainfall: " + String(_rain);
  _mqttManager.publishRainfall("rainfall/sensor", payload.c_str());
}

void PluviometerManager::createRainfallFile(){
  _myFile = SD.open("/box_b_rainfall.txt");
  if (!_myFile) {
    _myFile = SD.open("/rainfall.txt", FILE_WRITE);
    Serial.println("Rainfall file doesn't exist. Creating file");
    String header = String("Date") + "," + String("Rainfall"), + "\r\n";
    _myFile.println(header.c_str());
    _myFile.close();
    Serial.println("Rainfall File Created");
  } else {
      Serial.println("Rainfall file already exists");
  }
}

void PluviometerManager::saveRainfallValue(){
  _myFile = SD.open("/box_b_rainfall.txt", FILE_APPEND);
  if (_myFile) {
    String data_str = _timeManager.getDateTime() + "," + String(_rain), + "\r\n";
    _myFile.println(data_str.c_str());
    _myFile.close();
    Serial.println("Appended to the rainfall.txt file");
  } else {
      Serial.println("Error opening rainfall.txt file");
  }
}
