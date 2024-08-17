#include "FireBase.h"
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

FirebaseAuth auth;
FirebaseConfig config;
FirebaseData fbdo;
bool signupOK = false;
FirebaseJson json;

extern char *ssid;
extern char *password;
extern Status status;
// extern PacketInfo lastPacketInfo;

bool initFirebase(FirebaseAuth& authFirebase, FirebaseConfig& configFirebase) {
  configFirebase.api_key = "AIzaSyDirtcAZMFlDIpvWYXrpeaOTMgqK_TRhYI";
  configFirebase.database_url = "https://tinygs-sip-module-default-rtdb.asia-southeast1.firebasedatabase.app/";

  if (Firebase.signUp(&configFirebase, &authFirebase, "", "")) {
    signupOK = true;
  } else {
    Serial.print("Sign up error: ");
    Serial.println(configFirebase.signer.signupError.message.c_str());
    return false;
  }

  configFirebase.token_status_callback = tokenStatusCallback;
  Firebase.begin(&configFirebase, &authFirebase);
  return true;
}

bool sendPacketToDatabase()
{
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
    json.clear();
    json.set(ID_PATH, String(status.lastPacketInfo.id));
    json.set(SAT_PATH, status.modeminfo.satellite);
    json.set(EPOCH_PATH, getTimeDate());
    json.set(RSSI_PATH, String(status.lastPacketInfo.rssi));
    json.set(SNR_PATH, String(status.lastPacketInfo.snr));
    json.set(PACKET_PATH, String(status.lastPacketInfo.packet));
    json.set(LON_PATH, String(status.lastPacketInfo.lon));
    json.set(LAT_PATH, String(status.lastPacketInfo.lat));

    String databasePath = "/Data";
    String parentPath = databasePath + "/Package" + "/" + getTimeDate();
    
    if (Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json)) {
    Serial.println("Package sent successfully");
    return true;
  } else {
     Serial.print("Error sending data: ");
    Serial.println(fbdo.errorReason());
    Serial.println("Response code: " + String(fbdo.httpCode()));
    Serial.println("Error message: " + fbdo.errorReason());
    return false;
  }
}

bool sendLoginToDatabase()
{
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
    json.clear();

    json.set(WFRSSI_PATH, String(WiFi.RSSI()));
    json.set(EPOCH_PATH, getTimeDate());

    String databasePath = "/Data";
    String parentPath = databasePath + "/Wifi" + "/" + getTimeDate();
    
    if (Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json)) {
    Serial.println("Wifi sent successfully");
    return true;
  } else {
     Serial.print("Error sending data: ");
    Serial.println(fbdo.errorReason());
    Serial.println("Response code: " + String(fbdo.httpCode()));
    Serial.println("Error message: " + fbdo.errorReason());
    return false;
  }
}

bool sendListenToDatabase(String nameSat)
{
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
    json.clear();

    json.set(WFRSSI_PATH, String(WiFi.RSSI()));
    json.set(EPOCH_PATH, getTimeDate());
    json.set(NAME_PATH, nameSat);

    String databasePath = "/Data";
    String parentPath = databasePath + "/Listen"  + "/"+ getTimeDate();
    
    if (Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json)) {
    Serial.println("Listen sent successfully");
    return true;
  } else {
     Serial.print("Error sending data: ");
    Serial.println(fbdo.errorReason());
    Serial.println("Response code: " + String(fbdo.httpCode()));
    Serial.println("Error message: " + fbdo.errorReason());
    return false;
  }
}
  
