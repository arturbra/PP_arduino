#ifndef MQTTMANAGER_H
#define MQTTMANAGER_H

#include <WiFi.h>
#include <PubSubClient.h>

class MqttManager {
public:
  MqttManager(const char* mqtt_server, uint16_t mqtt_port, const char* mqtt_user, const char* mqtt_password);
  void begin();
  void publishTemperature(int sensorNumber, float temperature, const String& DateAndTimeString);
  void publishRainfall(const char* topic, const char* payload);

private:
  WiFiClient _espClient;
  PubSubClient _client;
};

#endif
