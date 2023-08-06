#ifndef TEMPERATURESENSOR_H
#define TEMPERATURESENSOR_H

#include <OneWire.h>
#include <DallasTemperature.h>

class TemperatureSensor {
public:
  TemperatureSensor(int pin, float calibrationFactor);
  void begin();
  bool isTemperatureReady(unsigned long currentMillis);
  float readTemperature();

private:
  OneWire _oneWire;
  DallasTemperature _sensors;
  float _calibrationFactor;
  unsigned long _lastReadMillis;
  const unsigned long _readPeriod = 300000;
};

#endif
