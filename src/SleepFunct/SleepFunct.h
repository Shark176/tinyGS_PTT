#include <Arduino.h>
#include <cstdint>
#include <esp_sleep.h> 
#include <RadioLib.h>
extern LLCC68 radio;

#define uS_TO_S_FACTOR                        1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_PREPARE_AFTER_WAKEUP             5          /* in second */


void goToSleep(uint8_t timeToSleep);
uint8_t calculateSleepTime(unsigned long Now, unsigned long Start);