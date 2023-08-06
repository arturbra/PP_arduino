#include "WifiManager.h"

WifiManager::WifiManager(const char* ssid, const char* password) 
  : _ssid(ssid), _password(password) {
}

void WifiManager::begin() {
  WiFi.begin(_ssid, _password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

bool WifiManager::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}
