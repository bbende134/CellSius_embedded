#include <vector>
#include "Network.h"
#include "addons/TokenHelper.h"

#define API_KEY "AIzaSyAHjkFGALD1RHc0IVNz8pPhM_HrjqqsPOA"
#define FIREBASE_PROJECT_ID "test-600f6"
#define USER_EMAIL "admin@admin.com"
#define USER_PASSWORD "admin1234"

#define WIFI_SSID "VOL_25"
#define WIFI_PASSWORD "135792468"

static Network* instance = NULL;

Network::Network() {
  instance = this;
}

void Network::initWiFi() {
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);

#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
  multi.addAP(WIFI_SSID, WIFI_PASSWORD);
  multi.run();
#else
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
#endif

  Serial.print("Connecting to Wi-Fi");
  unsigned long ms = millis();
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
    if (millis() - ms > 10000)
      break;
#endif
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

void Network::firebaseInit() {
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  config.token_status_callback = tokenStatusCallback;  // see addons/TokenHelper.h
  Firebase.reconnectNetwork(true);
  fbdo.setBSSLBufferSize(4096, 1024);

  // Limit the size of response payload to be collected in FirebaseData
  fbdo.setResponseSize(2048);

  Firebase.begin(&config, &auth);
}

bool Network::firebaseReady() {
  return Firebase.ready();
}

std::vector<String> Network::getBulbs(String documentPath) {
  FirebaseJsonData bulb;
  int count = 1;
  String mask = "bulb1";
  std::vector<String> bulbs;

  if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str())) {
    FirebaseJson resultJSON(fbdo.payload().c_str());

    while (resultJSON.get(bulb, "fields/"+mask + "/stringValue")) {
      bulbs.push_back(bulb.to<String>());
      mask.remove(4);
      mask = mask + String(++count);
    }
    return bulbs;
    
  }else return bulbs;
}

String Network::getTemperatureData(String documentPath, String mask) {
  FirebaseJsonData resultTemp, resultDate;
  if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), mask.c_str())) {
    //getting only temp data from received data
    Serial.print("Received info: ");
    Serial.println(fbdo.payload().c_str());
    FirebaseJson resultJSON(fbdo.payload().c_str());
    resultJSON.get(resultTemp, "fields/" + mask + "/stringValue");
    return resultTemp.to<String>();

  } else {
    Serial.println("Couldn't receive data");
    return "None";
  }
}
