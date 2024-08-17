#include <RadioLib.h>
#include <Sgp4.h>
// #include <EEPROM.h>
// #include <base64.h>
#include "src/Status.h"
#include "src/EpochTime/EpochTime.h"
#include "src/FireBase/FireBase.h"
#include "src/SleepFunct/SleepFunct.h"

#define BEGIN_LORA_OK           1
#define BEGIN_LORA_FAULT        0
#define EEPROM_SIZE             2
#define ADDR_ID_EEPROM          0
#define ADDR_MODE_EEPROM        1
#define LNA_GAIN                0


extern Status status;
extern Sgp4 mySat;

bool beginLoRa(LLCC68& radio, bool isPassing);
void setFlag(void);
void listenRadio(LLCC68& radio);
bool configParamsLoRa(Status& param, LLCC68& myRadio, String orderSat, bool isPassing);
bool initLoRa(Status& param, float* paramsSat, LLCC68& myRadio, bool isPassing);