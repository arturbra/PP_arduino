#include "MqttManager.h"

MqttManager::MqttManager(const char* mqtt_server, uint16_t mqtt_port, const char* mqtt_user, const char* mqtt_password) 
  : _client(_espClient) {
    _client.setServer(mqtt_server, mqtt_port);
    if (!_client.connect("ESP8266Client", mqtt_user, mqtt_password )) {
      Serial.println("Failed to connect to MQTT");
      while (1);
    }
}

void MqttManager::begin() {
  if (!_client.connected()) {
    _client.connect("ESP8266Client");
  }
  _client.loop();
}

void MqttManager::publishTemperature(int sensorNumber, float temperature, const String& DateAndTimeString) {
  String topic = "temperature/sensor" + String(sensorNumber);
  String payload = DateAndTimeString + ", " + String(temperature);
  _client.publish(topic.c_str(), payload.c_str());
}

void MqttManager::publishRainfall(const char* topic, const char* payload) {
  _client.publish(topic, payload);
}
