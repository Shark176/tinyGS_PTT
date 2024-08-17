#include "Radio.h"


float paramsNorby2[6] = {436.5 , 250, 10, 5, 18, 8};
float paramsTIANQI[6] = {400.45, 500, 9, 5, 18, 8};



volatile bool receivedFlag = false;
volatile bool enableInterrupt = true;

void setFlag(void) {
  if(!enableInterrupt) {
    return;
  }
  receivedFlag = true;
}

bool beginLoRa(LLCC68& radio, bool isPassing)
{
  ModemInfo &m = status.modeminfo;
  int state;
    state = radio.begin(m.frequency, m.bw, m.sf, m.cr, m.sw, m.power, m.preambleLength); 
  if (state == RADIOLIB_ERR_NONE) {
    radio.setCRC(m.crc);
    radio.forceLDRO(m.fldro);
    Serial.print("Begin lora sucess");
    if (radio.setCurrentLimit(120) == RADIOLIB_ERR_INVALID_CURRENT_LIMIT) {
      Serial.println(F("Selected current limit is invalid for this module!"));
    }
    radio.setPacketReceivedAction(setFlag);
    state = radio.startReceive();
    if (state == RADIOLIB_ERR_NONE) {
      return BEGIN_LORA_OK;
    } else {
      return BEGIN_LORA_FAULT;
    }
  } else {
    return BEGIN_LORA_FAULT;
  }
}

void listenRadio(LLCC68& radio)
{
  // Serial.println("Listen Function");
  if(receivedFlag) {
    enableInterrupt = false;
    receivedFlag = false;
    size_t respLen = 0;
    uint8_t *respFrame = 0;
    int16_t state = 0;
    String encoded;
    delay(1000);
    respLen = radio.getPacketLength();
    respFrame = new uint8_t[respLen];
    state = radio.readData(respFrame, respLen);
    Serial.print("Frame "); Serial.println(respLen);
    status.lastPacketInfo.rssi = radio.getRSSI();
    status.lastPacketInfo.snr = radio.getSNR();
  
    status.lastPacketInfo.id += 1;
    // EEPROM.write(ADDR_ID_EEPROM, status.lastPacketInfo.id);
    // EEPROM.commit();
      
    unsigned long unixt = 0;
    getEpochTimeNow(unixt);
    mySat.findsat(unixt);
    status.lastPacketInfo.lat = mySat.satLat;
    status.lastPacketInfo.lon = mySat.satLon;
    if(state == RADIOLIB_ERR_NONE){
      // encoded = base64::encode(respFrame, respLen); 
      Serial.println("sended");
    }else if(RADIOLIB_ERR_CRC_MISMATCH){
      encoded = "CRC_ERR";
    }else{
      encoded = "FAIL_CODE";
    }
    status.lastPacketInfo.packet = encoded;
    sendPacketToDatabase();
    delay(2000);
    delete[] respFrame;
    enableInterrupt = true;
    radio.startReceive();
  }
} 

bool configParamsLoRa(Status& param, LLCC68& myRadio, String orderSat, bool isPassing){
  bool state = true;
  param.modeminfo.satellite = orderSat;
  //if-else to death: I don't know the best solution to clean this function. Sorry!
  // if(orderSat == "Norbi"){
  //   state = initLoRa(param, paramsNorbi, myRadio, isPassing);
  // }else 
  if(orderSat == "Norby-2"){
    state = initLoRa(param, paramsNorby2, myRadio, isPassing);
  }else if(orderSat == "TIANQI-7"){
    state = initLoRa(param, paramsTIANQI, myRadio, isPassing);
  }
   else if(orderSat == "TIANQI-25"){
    state = initLoRa(param, paramsTIANQI, myRadio, isPassing);
  }
  else if(orderSat == "TIANQI-23"){
    state = initLoRa(param, paramsTIANQI, myRadio, isPassing);
  }else if(orderSat == "TIANQI-22"){
    state = initLoRa(param, paramsTIANQI, myRadio, isPassing);
  }else{
    state = initLoRa(param, paramsTIANQI, myRadio, isPassing);
  }
  return state;
}
bool initLoRa(Status& param, float* paramsSat, LLCC68& myRadio, bool isPassing){
  param.modeminfo.frequency = paramsSat[0]; 
  param.modeminfo.bw = paramsSat[1];
  param.modeminfo.sf = paramsSat[2];
  param.modeminfo.cr = paramsSat[3];
  param.modeminfo.sw = paramsSat[4];
  param.modeminfo.preambleLength = paramsSat[5];
  param.stateLoRa = beginLoRa(myRadio, isPassing);
  if(!param.stateLoRa){
    return false;
  }
  return true;
}