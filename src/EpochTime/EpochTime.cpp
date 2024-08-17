#include "EpochTime.h"


void getEpochTimeNow(unsigned long& epochTime){
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    epochTime = 0;
    return;
  }
  time(&now); //Convert to Epoch
  epochTime = now;
}

String getTimeDate(){
  struct tm timeinfo;
  int Year, Month;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return String(0);
  }
  Year = timeinfo.tm_year + 1900;
  Month = timeinfo.tm_mon + 1;
  String timeDate = String(timeinfo.tm_mday)+"-"+String(Month)+"-"+String(Year)+" "+String(timeinfo.tm_hour)+":"+String(timeinfo.tm_min);
  return timeDate; 
}

void updateEpochTimeNow(const char* ntpServerName, unsigned long& epochTime)
{
  configTime(7 * 3600, 0, ntpServerName);
  getEpochTimeNow(epochTime);
}
