#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <ArduinoJson.h>
#include <Firebase_ESP_Client.h>
#include "src/Status.h"
#include "src/EpochTime/EpochTime.h"

#define API_KEY                 "AIzaSyBu1oFsiXLwDM_QzTEmeNTXlD0-NKakTp8"
#define DATABASE_URL            "https://tinygs2-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define SAT_PATH                "/satellite"
#define EPOCH_PATH              "/epoch"
#define ID_PATH                 "/ID"
#define PACKET_PATH             "/packet"
#define RSSI_PATH               "/rssi"
#define SNR_PATH                "/snr"
#define LON_PATH                "/longitude"
#define LAT_PATH                "/latitude"
#define TIMESTAMP_PATH          "/timestamp"
#define WFRSSI_PATH             "/wifirssi"
#define NAME_PATH               "/nameSat"

bool initFirebase(FirebaseAuth& authFirebase, FirebaseConfig& configFirebase);
bool sendPacketToDatabase();
bool sendLoginToDatabase();
bool sendListenToDatabase(String nameSat);