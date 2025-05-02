#ifndef FIRE_H
#define FIRE_H

#include <Arduino.h>
#include <vector>
#include <WiFi.h>
#include <FirebaseClient.h>
#include <FirebaseJson.h>
#include <WiFiClientSecure.h>
using AsyncClient = AsyncClientClass;


#define WIFI_SSID "<wifi_name>"
#define WIFI_PASSWORD "<wifi_pass>"

// The API key can be obtained from Firebase console > Project Overview > Project settings.
#define API_KEY "<firebase_api_key>"
#define FIREBASE_PROJECT_ID "<firebase_project_id>"
#define FIREBASE_CLIENT_EMAIL "<service_account_email>"
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----<private_key_content>\n-----END PRIVATE KEY-----\n";


#define USER_EMAIL "admin@admin.com"
#define USER_PASSWORD "admin1234"

#define FIREBASE_PROJECT_ID "cellsius-demo"

class Fire {
public:
  Fire();
  void initWiFi();
  void firebaseInit();
  void loopElements();
  bool firebaseReady();

  std::vector<String> getBulbs(String location, String room);
  double getTemperatureData(String location, String room);
  bool setModifiedThermostatTemperature(double temp, String location, String room, String ts);
  bool writeTemperatureData(double temp, String location, String room, String ts);
  std::vector<double> getTransitionFunctionData(String location, String room);
  bool writeBulbState(String IP, int hue, int sat, int rgb, int ct, String location, String room, String ts); 

  // TODO: implement for the new DB

  // void timeStatusCB(uint32_t &ts);
};

#endif
