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

#include <FirebaseClient.h>

class Network_comm {
private:
  Firestore::Documents Docs;
  FirebaseApp app;
  AsyncResult aResult_no_callback;


public:
  Network_comm();
  void initWiFi();
  void firebaseInit();
  double getTemperatureData(String location, String room);
  std::vector<String> getBulbs(String location, String room);
  std::vector<double> getTransitionFunctionData(String location, String room);
  bool writeTemperatureData(double temp, String location, String room, String ts);
  void loopElements();
  // void timeStatusCB(uint32_t &ts);
  bool writeBulbState(String IP, int hue, int sat, int rgb, int ct, String location, String room, String ts);
  bool setModifiedThermostatTemperature(double temp, String location, String room, String ts);
  bool firebaseReady();
  int postWebhooks(String value1);
  // void firestoreDataUpdate(double temp, double humi);
};