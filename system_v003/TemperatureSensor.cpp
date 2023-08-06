#include "TemperatureSensor.h"

TemperatureSensor::TemperatureSensor(int pin, float calibrationFactor)
  : _oneWire(pin), _sensors(&_oneWire), _calibrationFactor(calibrationFactor), _lastReadMillis(0) {
}

void TemperatureSensor::begin() {
  _sensors.begin();
}

bool TemperatureSensor::isTemperatureReady(unsigned long currentMillis) {
  return currentMillis - _lastReadMillis >= _readPeriod;
}

float TemperatureSensor::readTemperature() {
  _sensors.requestTemperatures();
  float Celsius = _sensors.getTempCByIndex(0);
  float Fahrenheit = _sensors.toFahrenheit(Celsius) * _calibrationFactor;
  _lastReadMillis = millis();
  return Fahrenheit;
}
