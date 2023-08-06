#ifndef SDMANAGER_H
#define SDMANAGER_H

#include <SPI.h>
#include <SD.h>

class SdManager {
public:
  SdManager(int SCK, int MISO, int MOSI, int CS);
  void begin();
  void createTemperatureFile(int sensorNumber);
  void saveTemperatureValue(int sensorNumber, float temperature, const String& DateAndTimeString);

private:
  int _SCK;
  int _MISO;
  int _MOSI;
  int _CS;

  File _file;
};

#endif
