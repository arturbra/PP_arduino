#ifndef TIMEMANAGER_H
#define TIMEMANAGER_H

#include <RTClib.h>

class TimeManager {
public:
  TimeManager(const char* ntpServer, long gmtOffset_sec, int daylightOffset_sec);
  void init();
  void adjustRTC(bool wifiConnected);
  String getDateTime();

private:
  const char* _ntpServer;
  long _gmtOffset_sec;
  int _daylightOffset_sec;
  RTC_DS3231 _rtc;
  bool _time_error;

  bool getTime(struct tm &timeinfo);
};

#endif
