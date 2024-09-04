#ifndef NETWORK_COMM_H
#define NETWORK_COMM_H

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

#include <WiFiClientSecure.h>
#include <FirebaseClient.h>

// Firebase data

// #define API_KEY "AIzaSyAgD73O-useNuuE2X6T-Olho-32sgXmNIk"
// #define FIREBASE_PROJECT_ID "cellsius-demo"
// #define FIREBASE_CLIENT_EMAIL "firebase-adminsdk-6qn3e@cellsius-demo.iam.gserviceaccount.com"
// const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----\nMIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDCyh+KHejbHA3A\nmPvoZl0m2EQgCjJSQIWdrYaY1UQBOiED4fNZSsKL3RITCh85oPx98lLtFDE34ACz\nq8XVKc9yccOzgtu8WrLDiqa+M7xwh6VBr2iSDasJP+1Xv5GkkL+uq4qkwdU2MOj4\nLhl+K2RjKkZU+6gqEXKsQMk39EINMGIVpaZn56PMKsZXZ6MrGXQaRYrHIVUPGirC\ni5qEYwbDLbq08VqH8hc/jSKGA+gsmVNZaMWlqYL+Ccekph/ZEz/oh2/qSL6f9jj1\nDKLRGV8Iq8wgZs430olFRPC0kGdn0pLx16rMfGZJicCQzFC6iSX0r80FC8z2c/iN\n2RdF+aIHAgMBAAECggEABYyhgz3Pgz1IIhg5qLLVj5+XM7nbkDncgPF7tCtBskJU\nddfbTlYUI5HFENIaO3oSBjDYhwiNdwZar/7HQGBUQAXO7EM5Mw+YlHwE5BUSyAlQ\nVwYavSwBlxlBSvr8pRJYKJHB6gXbpv+T3pu2iA8x8xxdhJWrvWmJOZLBY9zaMS/L\nj4neAjsfeW1bgxeF7V/VjWpz64IlY1naMEr0ua9zkk9uxDbgtA6UUCAuAiuvIBXy\nuf2sikuZ3ddY2TKL/OZJlEnId0ZlzmbKYOpjXJM+iq/KTrxyF92mM8ZXAjNfnHdq\nbPGKmX9Plony9J+f6ZpdKMBBLXs5GPw9NVBFrBWKAQKBgQDqnVJ2qcndEt2JgX25\nXb15lByMe8eYc9/FC1XCnmy//cR+pUfdueJHZqtg4p6aERVFwYxjTjOoVJMd5mVj\nCCJurKTHT5huzbb3gGR5wta+im4s+/KmHnMjG5cH6GVngLLt6+hG5um0e/IZuPtF\nfWlarK2Q0kDJ375z5l96nAJvdwKBgQDUi35i52PptQLQmkPNyuG8oX9hjFU56Xcm\nDeto5Y7eaIWbPqVE94E/kjue4+F/y4F5REXNwgJcqHQO68XZPiZvw0V8wHoJIVpn\nCzT9Z/m4O8/AwfznCzuM8yO9cB+dCOxpMVoFZlLXTuaCFOOo8J6dHy/0ydCxAOIQ\nVqZYJKGl8QKBgQC9GZa47Ds3RBhp14RVXxf0IYwtMyt6VpMbB/rUxcqTqN+y0Fcv\ngL7AnCEwExiQx4ok0k7sT8DVQW3eoLBBdPID0IVz3FTZoleB/+t8NK3PoFYTTfOT\nL2HP/1HpXsg094fAuu9aDo56GZjWYMGujdnX6uxumPj9fajNU/bLJWnVGwKBgAzT\nmj33vtTUwqW6GYtqyfKrQFevs1j6WOFoB6Pd7qBHaIC06B5gXWFrvsNY9zvSGS8r\nSpcfNStIUzCvU9JHaORnwWLE7thsNdtwFrOQOca/fUshqdR7ng1kguykvpOofTPf\n8ZHP2gH6VfjVr+N7GedbgXZhU4HLDtlzkThShZrRAoGBAKGlo85kSjGkEy1h/SXG\nniVXrflFIFlk0RAlAhL2xnCuTSdSNgGf4pJ3vcpT6Saq6AGR4OzN/YSxs9V0bpSJ\nz+w5K/KT1A3fKY15Fg6rzprAqkb9wZXUFgd5GgRNbde3rCLb9j7SMOmRwy3WshQz\n5a7FkKNVhvfx9SkJfUPPMR/v\n-----END PRIVATE KEY-----\n";

// Wifi data
// #define WIFI_PASSWORD "135792468"

// #define USER_EMAIL "admin@admin.com"
// #define USER_PASSWORD "admin1234"

// #define FIREBASE_PROJECT_ID "cellsius-demo"



class Network_comm {

public:
  Network_comm();
  void initWiFi();
  void firebaseInit();
  double getTemperatureData(String location, String room);
  std::vector<String> getBulbs(String location, String room, Firestore::Documents Docs, AsyncResult aResult_no_callback, AsyncClient aClient);
  std::vector<double> getTransitionFunctionData(String location, String room, Firestore::Documents Docs, AsyncResult aResult_no_callback, AsyncClient aClient);
  bool writeTemperatureData(double temp, String location, String room, String ts, Firestore::Documents Docs, AsyncResult aResult_no_callback, AsyncClient aClient);
  void loopElements();
  // void timeStatusCB(uint32_t &ts);
  bool writeBulbState(String IP, int hue, int sat, int rgb, int ct, String location, String room, String ts, Firestore::Documents Docs, AsyncResult aResult_no_callback, AsyncClient aClient);
  bool setModifiedThermostatTemperature(double temp, String location, String room, String ts, Firestore::Documents Docs, AsyncResult aResult_no_callback, AsyncClient aClient);
  bool firebaseReady();
  int postWebhooks(String value1);
  // void firestoreDataUpdate(double temp, double humi);
private:
  // DefaultNetwork defNetwork;
  // Firestore::Documents Docs;
  // FirebaseApp app;
  // AsyncResult aResult_no_callback;
  // WiFiClientSecure ssl_client;
  // Firestore::CollectionGroups::Indexes indexes;

};

#endif