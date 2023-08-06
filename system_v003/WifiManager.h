#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <WiFi.h>

class WifiManager {
public:
  WifiManager(const char* ssid, const char* password);
  void begin();
  bool isConnected();

private:
  const char* _ssid;
  const char* _password;
};

#endif
