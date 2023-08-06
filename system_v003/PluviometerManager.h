#ifndef PLUVIOMETERMANAGER_H
#define PLUVIOMETERMANAGER_H

#include <Arduino.h>
#include <SD.h>
#include <PubSubClient.h>
#include "TimeManager.h"
#include "MqttManager.h"

class PluviometerManager {
public:
  PluviometerManager(int hallSensorPin, MqttManager& mqttManager, TimeManager& timeManager);
  void setup();
  void checkRainfall();
  void setupMQTTClient(const char* server, uint16_t port, const char* topic);

private:
  const int _hallSensorPin;
  PubSubClient& _mqttClient;
  String _mqttTopic;
  unsigned long _rain; // Counter for magnet passes
  bool _lastSensorState; // Used to store the previous sensor state
  int _sumSensorValues; // Variable to hold the sum of sensor values
  int _numReadings; // Variable to count the number of readings
  unsigned long _lastRainfallMillis;
  unsigned long _debounceTime;
  File _myFile;
  TimeManager& _timeManager;
  MqttManager& _mqttManager;

  void createRainfallFile();
  void saveRainfallValue();
  String getDateAndTimeString();
  void publishRainfallData();
};

#endif
