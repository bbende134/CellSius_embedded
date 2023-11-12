#include <vector>
#include <NTPClient.h>
#include <WiFiUdp.h>
// #include <chrono>
#include "Network.h"
#include "addons/TokenHelper.h"
#include <HTTPClient.h>
#include <ESPDateTime.h>

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

std::vector<String> Network::getBulbs(String location, String room) {
  FirebaseJsonData bulb;
  FirebaseJson query;
  int count = 1;
  int index;
  String mask = "bulb_1";
  std::vector<String> bulbs;

  query.set("select/fields/[0]/fieldPath", "IP");
  query.set("from/collectionId", "bulbs");
  query.set("from/allDescendants", false);

  query.set("where/compositeFilter/op", "AND");
  query.set("where/compositeFilter/filters/[0]/fieldFilter/field/fieldPath", "room");
  query.set("where/compositeFilter/filters/[0]/fieldFilter/op", "EQUAL");
  query.set("where/compositeFilter/filters/[0]/fieldFilter/value/stringValue", room);
  query.set("where/compositeFilter/filters/[1]/fieldFilter/field/fieldPath", "location");
  query.set("where/compositeFilter/filters/[1]/fieldFilter/op", "EQUAL");
  query.set("where/compositeFilter/filters/[1]/fieldFilter/value/stringValue", location);
  query.set("where/compositeFilter/filters/[2]/fieldFilter/field/fieldPath", "type");
  query.set("where/compositeFilter/filters/[2]/fieldFilter/op", "EQUAL");
  query.set("where/compositeFilter/filters/[2]/fieldFilter/value/stringValue", "props");

  Serial.println(location);
  if (Firebase.Firestore.runQuery(&fbdo, FIREBASE_PROJECT_ID, "", "/", &query)) {
    Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());

    FirebaseJson resultJSON(fbdo.payload().c_str());
    while (resultJSON.get(bulb, "[0]/document/fields/IP/mapValue/fields/" + mask + "/stringValue")) {
      bulbs.push_back(bulb.to<String>());
      mask.replace(mask.substring(mask.indexOf("_") + 1), String(++count));
    }
    return bulbs;

  } else {
    Serial.println(fbdo.errorReason());
    return bulbs;
  }
}

double Network::getTemperatureData(String location, String room) {
  FirebaseJsonData resultTemp;
  FirebaseJson query;

  query.set("select/fields/[0]/fieldPath", "val");

  query.set("from/collectionId", "temperature");
  query.set("from/allDescendants", false);

  // COMPOSITE FILTERS
  query.set("where/compositeFilter/op", "AND");
  query.set("where/compositeFilter/filters/[2]/fieldFilter/field/fieldPath", "location");
  query.set("where/compositeFilter/filters/[2]/fieldFilter/op", "EQUAL");
  query.set("where/compositeFilter/filters/[2]/fieldFilter/value/stringValue", location);
  query.set("where/compositeFilter/filters/[0]/fieldFilter/field/fieldPath", "room");
  query.set("where/compositeFilter/filters/[0]/fieldFilter/op", "EQUAL");
  query.set("where/compositeFilter/filters/[0]/fieldFilter/value/stringValue", room);
  query.set("where/compositeFilter/filters/[1]/fieldFilter/field/fieldPath", "type");
  query.set("where/compositeFilter/filters/[1]/fieldFilter/op", "EQUAL");
  query.set("where/compositeFilter/filters/[1]/fieldFilter/value/stringValue", "set_thermostat");

  //ORDERING
  query.set("orderBy/field/fieldPath", "ts");
  query.set("orderBy/direction", "DESCENDING");
  query.set("limit", 1);

  if (Firebase.Firestore.runQuery(&fbdo, FIREBASE_PROJECT_ID, "", "/", &query)) {
    Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
    FirebaseJson resultJSON(fbdo.payload().c_str());
    resultJSON.get(resultTemp, "[0]/document/fields/val/doubleValue/");
    if (resultTemp.to<double>() == 0.0) resultJSON.get(resultTemp, "[0]/document/fields/val/integerValue/");
    return resultTemp.to<double>();

  } else {
    Serial.println(fbdo.errorReason());
    return 0;
  }
}

std::vector<double> Network::getTransitionFunctionData(String location, String room) {

  FirebaseJsonData resultData;
  FirebaseJson query;
  std::vector<double> transitionData;

  query.set("select/fields/[0]/fieldPath", "zero");
  query.set("select/fields/[1]/fieldPath", "min");
  query.set("select/fields/[2]/fieldPath", "max");
  query.set("select/fields/[3]/fieldPath", "speed");

  query.set("from/collectionId", "bulbs");
  query.set("from/allDescendants", false);

  // COMPOSITE FILTERS
  query.set("where/compositeFilter/op", "AND");
  query.set("where/compositeFilter/filters/[2]/fieldFilter/field/fieldPath", "location");
  query.set("where/compositeFilter/filters/[2]/fieldFilter/op", "EQUAL");
  query.set("where/compositeFilter/filters/[2]/fieldFilter/value/stringValue", "location_1");
  query.set("where/compositeFilter/filters/[0]/fieldFilter/field/fieldPath", "room");
  query.set("where/compositeFilter/filters/[0]/fieldFilter/op", "EQUAL");
  query.set("where/compositeFilter/filters/[0]/fieldFilter/value/stringValue", "main_room");
  query.set("where/compositeFilter/filters/[1]/fieldFilter/field/fieldPath", "type");
  query.set("where/compositeFilter/filters/[1]/fieldFilter/op", "EQUAL");
  query.set("where/compositeFilter/filters/[1]/fieldFilter/value/stringValue", "transition_data");

  //ORDERING
  query.set("orderBy/field/fieldPath", "ts");
  query.set("orderBy/direction", "DESCENDING");
  query.set("limit", 1);

  if (Firebase.Firestore.runQuery(&fbdo, FIREBASE_PROJECT_ID, "", "/", &query)) {
    Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
    FirebaseJson resultJSON(fbdo.payload().c_str());

    resultJSON.get(resultData, "[0]/document/fields/zero/doubleValue/");
    resultJSON.get(resultData, "[0]/document/fields/zero/integerValue/");
    transitionData.push_back(resultData.to<double>());

    resultJSON.get(resultData, "[0]/document/fields/min/doubleValue/");
    resultJSON.get(resultData, "[0]/document/fields/min/integerValue/");
    transitionData.push_back(resultData.to<double>());

    resultJSON.get(resultData, "[0]/document/fields/max/doubleValue/");
    resultJSON.get(resultData, "[0]/document/fields/max/integerValue/");
    transitionData.push_back(resultData.to<double>());

    resultJSON.get(resultData, "[0]/document/fields/speed/doubleValue/");
    resultJSON.get(resultData, "[0]/document/fields/speed/integerValue/");
    transitionData.push_back(resultData.to<double>());

    return transitionData;

  } else {
    Serial.println(fbdo.errorReason());
    return transitionData;
  }
}

bool Network::writeTemperatureData(double temp, String location, String room, String ts) {  // TODO: implement for the new DB

  FirebaseJson content;
  ts.remove(ts.length() - 5, 5);
  ts += "Z";

  String documentPath = "temperature/actual_temp_" + ts;
  content.set("fields/type/stringValue/", "actual_temperature");
  content.set("fields/value/doubleValue/", temp);
  content.set("fields/location/stringValue/", location);
  content.set("fields/room/stringValue/", room);
  content.set("fields/ts/timestampValue/", ts);

  if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), content.raw())) {
    Serial.println("ok write actual temperature");
    return 1;
  } else {
    Serial.println(fbdo.errorReason());
    return 0;
  }
}

bool Network::writeBulbState(String IP, int hue, int sat, int rgb, int ct, String location, String room, String ts) {  // TODO: implement for the new DB

  FirebaseJson content;
  ts.remove(ts.length() - 5, 5);
  ts += "Z";

  String documentPath = "bulbs/actual_state_" + ts + "_" + IP;
  content.set("fields/type/stringValue/", "state");
  content.set("fields/IP/stringValue/", IP);
  content.set("fields/hue/integerValue/", hue);
  content.set("fields/sat/integerValue/", sat);
  content.set("fields/rgb/integerValue/", rgb);
  content.set("fields/ct/integerValue/", ct);
  content.set("fields/location/stringValue/", location);
  content.set("fields/room/stringValue/", room);
  content.set("fields/ts/timestampValue/", ts);

  if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), content.raw())) {
    Serial.println("ok bulb props");
    return 1;
  } else {
    Serial.println(fbdo.errorReason());
    return 0;
  }
}

bool Network::setModifiedThermostatTemperature(double temp, String location, String room, String ts) {  // TODO: implement for the new DB

  FirebaseJson content;
  ts.remove(ts.length() - 5, 5);
  ts += "Z";

  String documentPath = "temperature/set_modified_thermostat_temperature_" + ts;
  content.set("fields/type/stringValue/", "set_modified_thermostat_temperature");
  content.set("fields/value/doubleValue/", temp);
  content.set("fields/location/stringValue/", location);
  content.set("fields/room/stringValue/", room);
  content.set("fields/ts/timestampValue/", ts);

  if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), content.raw())) {
    Serial.println("ok write thermostat temperature");
    return 1;
  } else {
    Serial.println(fbdo.errorReason());
    return 0;
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
