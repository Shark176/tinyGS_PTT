#include "src/SleepFunct/SleepFunct.h"



void goToSleep(uint8_t timeToSleep)
{
  radio.sleep(true);
  esp_sleep_enable_timer_wakeup(timeToSleep * uS_TO_S_FACTOR);
  Serial.println("Going to sleep now");
  Serial.println(timeToSleep * uS_TO_S_FACTOR);
  Serial.flush(); 
  esp_deep_sleep_start();
}
uint8_t calculateSleepTime(unsigned long Now, unsigned long Start)
{
  return (uint8_t)(Start - Now - TIME_PREPARE_AFTER_WAKEUP);
}