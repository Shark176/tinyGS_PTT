#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <time.h>
#include <string>

#ifndef EPOCH_TIME_H
#define EPOCH_TIME_H

struct timeInfoSat{
  unsigned long epochStart = 0;
  unsigned long epochStop = 0;
  unsigned long epochMax = 0;
};

#endif // EPOCH_TIME
extern timeInfoSat epochInfo;



void getEpochTimeNow(unsigned long& epochTime);
String getTimeDate();
void updateEpochTimeNow(const char* ntpServerName, unsigned long& epochTime);