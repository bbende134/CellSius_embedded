#include <vector>
#include <chrono>
#include "Network.h"
#include "addons/TokenHelper.h"
#include <HTTPClient.h>

// Firebase data
#define API_KEY "AIzaSyAHjkFGALD1RHc0IVNz8pPhM_HrjqqsPOA"
#define FIREBASE_PROJECT_ID "test-600f6"
#define USER_EMAIL "admin@admin.com"
#define USER_PASSWORD "admin1234"

// Wifi data
#define WIFI_SSID "VOL 23"
#define WIFI_PASSWORD "135792468"

static Network* instance = NULL;

// IFTTT data
String eventNAME = "new_mail";
String webhooksKEY = "bk-rNjIlmxawG-VkvLWg9K";
const int httpsPort = 443;
String url = "https://maker.ifttt.com/trigger/" + eventNAME + "/with/key/" + webhooksKEY;

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

std::vector<String> Network::getBulbs(String location) {
  FirebaseJsonData bulb;
  FirebaseJson query;
  int count = 1;
  int index;
  String mask = "bulb_1";
  String mask_temp;
  std::vector<String> bulbs;

  query.set("select/fields/[0]/fieldPath", "IP");
  query.set("from/collectionId", "bulbs");
  query.set("from/allDescendants", false);
  query.set("where/fieldFilter/field/fieldPath", "location");
  query.set("where/fieldFilter/op", "EQUAL");
  query.set("where/fieldFilter/value/stringValue", location);
  Serial.println(location);
  if (Firebase.Firestore.runQuery(&fbdo, FIREBASE_PROJECT_ID, "", "/", &query)) {
    Serial.printf("ok\n%s\n\n");
    
    FirebaseJson resultJSON(fbdo.payload().c_str());
    while (resultJSON.get(bulb, "[0]/document/fields/IP/mapValue/fields/" + mask + "/stringValue")) {
      bulbs.push_back(bulb.to<String>());
      mask.replace(mask.substring(mask.indexOf("_") + 1), String(++count));
    }
    return bulbs;

  } else return bulbs;
}

String Network::getTemperatureData(String documentPath) {  // TODO: implement for the new DB
  FirebaseJsonData resultTemp, resultDate;

  if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str())) {
    //getting only temp data from received data
    // Serial.print("Received info: ");
    // Serial.println(fbdo.payload().c_str());
    FirebaseJson resultJSON(fbdo.payload().c_str());
    resultJSON.get(resultTemp, "fields/stringValue");
    return resultTemp.to<String>();

  } else {
    Serial.println("Couldn't receive data");
    return "None";
  }
}

String Network::writeTemperatureData(double temp, String documentPath) {  // TODO: implement for the new DB
  FirebaseJson content;
  String time = String(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());
  content.set("fields/temperatures/mapValue/fields/key" + time + "/doubleValue", temp);

  if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), content.raw(), "temperatures.key" + time)) {
    Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());

    return "true";
  } else {
    Serial.println(fbdo.errorReason());
    return "false";
  }
}

int Network::postWebhooks(String value1) {

  if (WiFi.status() == WL_CONNECTED) {  //Check WiFi connection status
    HTTPClient http;

    String url_out = url + "?value1=" + value1;
    Serial.print("url: ");
    Serial.println(url_out);

    http.begin(url_out);                   //Specify destination for HTTP request
    int httpResponseCode = http.POST("");  //Send the actual POST request

    if (httpResponseCode > 0) {

      String response = http.getString();  //Get the response to the request

      Serial.println(httpResponseCode);  //Print return code
      Serial.println(response);
      return httpResponseCode;

    } else {

      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
      return httpResponseCode;
    }

    http.end();  //Free resources

  } else {
    Serial.println("Error in WiFi connection");
    return 0;
  }
}
