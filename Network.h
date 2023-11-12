#if defined(ESP32) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#elif __has_include(<WiFiNINA.h>)
#include <WiFiNINA.h>
#elif __has_include(<WiFi101.h>)
#include <WiFi101.h>
#elif __has_include(<WiFiS3.h>)
#include <WiFiS3.h>
#endif


#include <Firebase_ESP_Client.h>
#include <ArduinoJson.h>

class Network {
private:
  FirebaseData fbdo;
  FirebaseAuth auth;
  FirebaseConfig config;

public:
  Network();
  void initWiFi();
  void firebaseInit();
  double getTemperatureData(String location, String room);
  std::vector<String> getBulbs(String location, String room);
  std::vector<double> getTransitionFunctionData(String location, String room);
  bool writeTemperatureData(double temp, String location, String room, String ts);
  bool writeBulbState(String IP, int hue, int sat, int rgb, int ct, String location, String room, String ts);
  bool setModifiedThermostatTemperature(double temp, String location, String room, String ts);
  bool firebaseReady();
  int postWebhooks(String value1);
  // void firestoreDataUpdate(double temp, double humi);
};