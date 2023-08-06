#include "TimeManager.h"
#include <time.h>
#include <Wire.h>

TimeManager::TimeManager(const char* ntpServer, long gmtOffset_sec, int daylightOffset_sec) 
  : _ntpServer(ntpServer), _gmtOffset_sec(gmtOffset_sec), _daylightOffset_sec(daylightOffset_sec), _time_error(false) {
}

void TimeManager::init() {
  _rtc.begin();
  configTime(_gmtOffset_sec, _daylightOffset_sec, _ntpServer);
}

bool TimeManager::getTime(struct tm &timeinfo){
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Error obtaining time. Possibly the Wi-Fi is not connected. The RTC time will be used instead.");
    _time_error = true;
    return false;
  }
  return true;
}

void TimeManager::adjustRTC(bool wifiConnected) {
  if (wifiConnected) {
    Serial.println("Adjusting the RTC timer based on the pool.ntp.org time");
    configTime(_gmtOffset_sec, _daylightOffset_sec, _ntpServer);

    struct tm timeinfo;
    if (getTime(timeinfo)) {
      DateTime dt(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
      _rtc.adjust(dt);
      Serial.println("Successfully adjusted the time");
    } else {
      Serial.println("Error adjusting the RTC with the online server. Possibly the Wi-Fi is not connected.");
    }
  }
  _time_error = false;
}

String TimeManager::getDateTime() {
  DateTime now = _rtc.now();
  char buffer[21];
  sprintf(buffer, "%4d-%02d-%02d %02d:%02d:%02d", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
  return String(buffer);
}
