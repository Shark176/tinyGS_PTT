#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <RadioLib.h>
#include <string>
#include <Sgp4.h>
#include <time.h>
#include "src/Status.h"
#include "src/FireBase/FireBase.h"
#include "src/SleepFunct/SleepFunct.h"
#include "src/TLE/TLEFunct.h"
#include "src/EpochTime/EpochTime.h"
#include "src/Radio/Radio.h"

// const char *ssid = "PTT";
// const char *password = "CMT@1975";

// const char *ssid = "MTC-TTM";
// const char *password = "123322456";

const char *ssid = "TTLAB2024";
const char *password = "123322456";

// const char *ssid = "AICLUB_B8.2plus";
// const char *password = "aiclub_uit";
const char* ntpServer = "time.cloudflare.com";


#define URL_TLE_TINYGS                        "https://api.tinygs.com/v1/tinygs_supported.txt"

#define uS_TO_S_FACTOR                        1000000ULL  /* Conversion factor for micro seconds to seconds */

#define TIME_ACCEPT_PASS_LISTEN               5          /* in second */
#define SAVE_MODE                             0
#define FOCUS_MODE                            1
#define STATE_PRE_PASS                        0
#define STATE_IN_PASS                         1
#define STATE_POST_PASS                       2

#define _PREDICT_
#define GMT_OFFSET_SECOND           7
#define DAYLIGHT_OFFSET_SECOND      0
#define PREDICT_OFFSET_SECOND       600
#define NUM_ORDER_SAT               4
#define FAIL_CODE                   0
#define SUCCESS_CODE                1


extern timeInfoSat epochInfo;
extern FirebaseAuth auth;
extern FirebaseConfig config;

Sgp4 mySat;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
timeInfoSat epochInfo;
Status status;
LLCC68 radio = new Module(3, 10, 4, 5);
// String orderSatList[6] = {"Norby", "Norby-2", "Tianqi-7", "TIANQI-21", "TIANQI-22", "TIANQI-27"};
String orderSatList[4] = {"TIANQI-25", "Tianqi-7", "TIANQI-23", "TIANQI-22"};

static char tleLine1[70];
static char tleLine2[70];
String payload;
unsigned long epochNow = 1660138928;
String* upcomingSatList;
uint8_t totalSat = 0, posInList = 0;
struct stateGS{
  uint8_t current = STATE_PRE_PASS;
  uint8_t previous; 
};
stateGS state;
String timedate;

/*----------------------------------------------------------------------
|--------------------------------SETUP---------------------------------|
----------------------------------------------------------------------*/

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");

  configTime(GMT_OFFSET_SECOND * 3600, 0, ntpServer);
  getEpochTimeNow(epochNow); 
  Serial.println(epochNow);
  delay(1000);
  updateTleData(payload, URL_TLE_TINYGS);
  mySat.site(10.87, 106.803, 21);
  totalSat = NUM_ORDER_SAT;
  createUpcomingOrderList(orderSatList, mySat, payload);
  delay(1000);
  if (initFirebase(auth, config)){
    Serial.println("Init Firebase sucess!");
    if(sendLoginToDatabase()){
      Serial.println("Send login sucess!");
      delay(1000);
    }
  }else{
    Serial.println("Init Firebase fail");
  }
}

/*----------------------------------------------------------------------
|---------------------------------LOOP---------------------------------|
----------------------------------------------------------------------*/

void loop() {
  // switch(state.current){
  //   case STATE_PRE_PASS:
  //     prePass(posInList, epochNow, FOCUS_MODE, state);
  //     break;
  //   case STATE_IN_PASS:
  //     inPass(posInList, epochNow, state);
  //     break;
  //   case STATE_POST_PASS:
  //     postPass(posInList, totalSat, state);
  //     break;
  //   default:
  //     break;
  // }
  prePass(posInList, epochNow, FOCUS_MODE, state);
  if (WiFi.status() != WL_CONNECTED) {
          Serial.println("WiFi lost connection. Reconnecting...");
          WiFi.disconnect();
          WiFi.begin(ssid, password);
          Serial.println("Reconnecting to WiFi...");
          while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
          }
        Serial.println("Reconnected to WiFi");
  }
}

/*----------------------------------------------------------------------
|----------------------------STATE MACHINE-----------------------------|
----------------------------------------------------------------------*/
void prePass(uint8_t positionInOrder, unsigned long& unixtNow, uint8_t modeOP, stateGS& state){
  findSatEpochInfo(positionInOrder, unixtNow);
  state.previous = STATE_PRE_PASS;
  Serial.println("PrePass");
  Serial.println(calculateSleepTime(unixtNow, epochInfo.epochStart));
  if(epochInfo.epochStart - TIME_PREPARE_AFTER_WAKEUP > unixtNow){
    if(modeOP == SAVE_MODE){
      Serial.println("Sleep");
      uint64_t timeToSleep = calculateSleepTime(unixtNow, epochInfo.epochStart);
      goToSleep(timeToSleep);
    }else{
      Serial.print("Config ");  Serial.println(orderSatList[positionInOrder]);
      if(configParamsLoRa(status, radio, orderSatList[positionInOrder], false)){
              Serial.println("Config OK");
              sendListenToDatabase(orderSatList[positionInOrder]);
              delay(1000);
              
      }
      unsigned long unixtStart = epochInfo.epochStart - TIME_PREPARE_AFTER_WAKEUP;
      Serial.println("Listen");
      delay(500);
      while(unixtStart > unixtNow){
        // while(n<3){
        if (WiFi.status() != WL_CONNECTED) {
          Serial.println("WiFi lost connection. Reconnecting...");
          WiFi.disconnect();
          WiFi.begin(ssid, password);
          Serial.println("Reconnecting to WiFi...");
          while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
          }
        Serial.println("Reconnected to WiFi");
        } 
        listenRadio(radio);
        getEpochTimeNow(unixtNow);
      } 
    }
  }else if(unixtNow < epochInfo.epochStop){
    state.current = STATE_IN_PASS;
  }else{
    state.current = STATE_POST_PASS;
  }
}


void inPass(uint8_t positionInOrder, unsigned long& unixtNow, stateGS& state){
  Serial.print("Sat listen: "); Serial.println(orderSatList[positionInOrder]);
  if(configParamsLoRa(status, radio, orderSatList[positionInOrder], true)){
          sendListenToDatabase(orderSatList[positionInOrder]);
  } 
  bool endTime = false;
  delay(500);
  Serial.println("InPass");
  while(unixtNow <= epochInfo.epochStop){
    if (WiFi.status() != WL_CONNECTED) {
          Serial.println("WiFi lost connection. Reconnecting...");
          WiFi.disconnect();
          WiFi.begin(ssid, password);
          Serial.println("Reconnecting to WiFi...");
          while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
          }
        Serial.println("Reconnected to WiFi");
        }
    listenRadio(radio);
    getEpochTimeNow(unixtNow);
    mySat.findsat(unixtNow);
  }
  endTime = true;
  state.previous = STATE_IN_PASS;
  state.current = STATE_POST_PASS;
}


void postPass(uint8_t& positionInOrder, uint8_t totalSatOrder, stateGS& state){
  Serial.println("PostPass");
  updateTleData(payload, URL_TLE_TINYGS);
  sortUpcomingList(orderSatList, mySat, payload, totalSatOrder);
  if(state.previous != STATE_PRE_PASS){
    positionInOrder = 0;
  }else{
    ++positionInOrder;
  }
  state.previous = STATE_POST_PASS;
  state.current = STATE_PRE_PASS;
}

/*----------------------------------------------------------------------
|---------------------------ORTHER FUNCTION----------------------------|
----------------------------------------------------------------------*/

void findSatEpochInfo(uint8_t positionInOrder, unsigned long& unixtNow){
  getEpochTimeNow(unixtNow);
  initialize_Sat(orderSatList[positionInOrder], mySat, payload); 
  status.statePredict = Predict(mySat, unixtNow);
}

/*----------------------------------------------------------------------
|---------------------------PREDICT FUNCTION---------------------------|
----------------------------------------------------------------------*/
void initialize_Sat(String nameOfSat, Sgp4& sat, String payload){
  char arrSatName[20];
  // static char tleLine1[70] ; 
  // static char tleLine2[70] ; 
  Serial.print("Init: "); Serial.println(nameOfSat);
  uint8_t len = 0;
  getTLE(nameOfSat, tleLine1, tleLine2, payload);
  len = nameOfSat.length() + 1; 
  nameOfSat.toCharArray(arrSatName, len);
  sat.init(arrSatName, tleLine1,tleLine2);
  Serial.println("Init done!");
}
unsigned long Predict(Sgp4& sat, unsigned long unix_t){
  passinfo overpass;                       
  sat.initpredpoint(unix_t , 0.0);       
  int  year, mon, day, hr, minute; double sec;
  bool nonError;
  nonError = sat.nextpass(&overpass,10);  
  if(nonError){ 
    invjday(overpass.jdstop ,GMT_OFFSET_SECOND ,true , year, mon, day, hr, minute, sec);
    epochInfo.epochStop = getUnixFromJulian(overpass.jdstop);

    invjday(overpass.jdmax ,GMT_OFFSET_SECOND ,true , year, mon, day, hr, minute, sec);
    epochInfo.epochMax = getUnixFromJulian(overpass.jdmax);
    
    invjday(overpass.jdstart ,GMT_OFFSET_SECOND ,true , year, mon, day, hr, minute, sec);
    epochInfo.epochStart = getUnixFromJulian(overpass.jdstart);
    return epochInfo.epochStart;
  }else{
    Serial.println("Prediction error");
    return 0;
  }
}
bool createUpcomingOrderList(String* listUpcomingSat, Sgp4 sat, String payload)
{  
  unsigned long unix_t = 0;
  String tmp_name;
  getEpochTimeNow(unix_t);
  if(!unix_t)
    return FAIL_CODE;
  unsigned long timeMaxElevationList[NUM_ORDER_SAT];
  for(uint8_t i = 0; i < NUM_ORDER_SAT; i++){
    delay(1000);
    Serial.println(i);
    initialize_Sat(listUpcomingSat[i], sat, payload);
    timeMaxElevationList[i] = Predict(sat, unix_t);
    Serial.println(listUpcomingSat[i]); //Serial.print(" - "); Serial.println(timeMaxElevationList[i]);
  }
  for(uint8_t i = 0; i < NUM_ORDER_SAT - 1; ++i){
    for(uint8_t j = i + 1; j < NUM_ORDER_SAT; ++j){
      if(timeMaxElevationList[i] > timeMaxElevationList[j]){
        tmp_name = listUpcomingSat[i];
        unsigned long tmp_t = timeMaxElevationList[i];
        
        listUpcomingSat[i] = listUpcomingSat[j];
        timeMaxElevationList[i] = timeMaxElevationList[j];
        
        listUpcomingSat[j] = tmp_name;
        timeMaxElevationList[j] = tmp_t; 
      }
    }
  }
  for(uint8_t i = 0; i < NUM_ORDER_SAT - 1; i++){
     Serial.println(listUpcomingSat[i]);
    delay(500);
  }
  return SUCCESS_CODE;
}
String getHigherSat(String* listUpcomingSat)
{
  return listUpcomingSat[0];
}
bool sortUpcomingList(String* listUpcomingSat, Sgp4 sat, String payload, uint8_t totalSat)
{
  unsigned long unix_t = 0;
  getEpochTimeNow(unix_t);
  if(unix_t == 0 || totalSat == 0)
    return FAIL_CODE;
  unsigned long* timeMaxElevationList = new unsigned long[totalSat];  
  initialize_Sat(listUpcomingSat[0], sat, payload);
  unsigned long tmp_t = unix_t + PREDICT_OFFSET_SECOND;
  timeMaxElevationList[0] = Predict(sat, tmp_t);
  for(uint8_t i = 1; i < totalSat; ++i){
    initialize_Sat(listUpcomingSat[i], sat, payload);
    timeMaxElevationList[i] = Predict(sat, unix_t);
  }
  int j;
  for (uint8_t i = 1; i < totalSat; ++i){
    tmp_t = timeMaxElevationList[i];
    String tmp_name = listUpcomingSat[i];
    j = i - 1;
    while (j >= 0 && timeMaxElevationList[j] > tmp_t)
    {
      timeMaxElevationList[j + 1] = timeMaxElevationList[j];
      listUpcomingSat[j + 1] = listUpcomingSat[j];
      j = j - 1;
    }
    timeMaxElevationList[j + 1] = tmp_t;
    listUpcomingSat[j + 1] = tmp_name;
  }
  delete[] timeMaxElevationList;
  return SUCCESS_CODE;
}

