#include <vector>
#include <NTPClient.h>
#include <WiFiUdp.h>
// #include <chrono>
#include "Network_comm.h"
#include <HTTPClient.h>
#include <ESPDateTime.h>

#if defined(ESP32) || defined(ESP8266) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFiClientSecure.h>
WiFiClientSecure ssl_client;
#elif defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_UNOWIFIR4) || defined(ARDUINO_GIGA) || defined(ARDUINO_PORTENTA_C33) || defined(ARDUINO_NANO_RP2040_CONNECT)
#include <WiFiSSLClient.h>
WiFiSSLClient ssl_client;
#endif

// Firebase data
#define FIREBASE_PROJECT_ID "cellsius-demo"
#define FIREBASE_CLIENT_EMAIL "firebase-adminsdk-6qn3e@cellsius-demo.iam.gserviceaccount.com"
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----\nMIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDCyh+KHejbHA3A\nmPvoZl0m2EQgCjJSQIWdrYaY1UQBOiED4fNZSsKL3RITCh85oPx98lLtFDE34ACz\nq8XVKc9yccOzgtu8WrLDiqa+M7xwh6VBr2iSDasJP+1Xv5GkkL+uq4qkwdU2MOj4\nLhl+K2RjKkZU+6gqEXKsQMk39EINMGIVpaZn56PMKsZXZ6MrGXQaRYrHIVUPGirC\ni5qEYwbDLbq08VqH8hc/jSKGA+gsmVNZaMWlqYL+Ccekph/ZEz/oh2/qSL6f9jj1\nDKLRGV8Iq8wgZs430olFRPC0kGdn0pLx16rMfGZJicCQzFC6iSX0r80FC8z2c/iN\n2RdF+aIHAgMBAAECggEABYyhgz3Pgz1IIhg5qLLVj5+XM7nbkDncgPF7tCtBskJU\nddfbTlYUI5HFENIaO3oSBjDYhwiNdwZar/7HQGBUQAXO7EM5Mw+YlHwE5BUSyAlQ\nVwYavSwBlxlBSvr8pRJYKJHB6gXbpv+T3pu2iA8x8xxdhJWrvWmJOZLBY9zaMS/L\nj4neAjsfeW1bgxeF7V/VjWpz64IlY1naMEr0ua9zkk9uxDbgtA6UUCAuAiuvIBXy\nuf2sikuZ3ddY2TKL/OZJlEnId0ZlzmbKYOpjXJM+iq/KTrxyF92mM8ZXAjNfnHdq\nbPGKmX9Plony9J+f6ZpdKMBBLXs5GPw9NVBFrBWKAQKBgQDqnVJ2qcndEt2JgX25\nXb15lByMe8eYc9/FC1XCnmy//cR+pUfdueJHZqtg4p6aERVFwYxjTjOoVJMd5mVj\nCCJurKTHT5huzbb3gGR5wta+im4s+/KmHnMjG5cH6GVngLLt6+hG5um0e/IZuPtF\nfWlarK2Q0kDJ375z5l96nAJvdwKBgQDUi35i52PptQLQmkPNyuG8oX9hjFU56Xcm\nDeto5Y7eaIWbPqVE94E/kjue4+F/y4F5REXNwgJcqHQO68XZPiZvw0V8wHoJIVpn\nCzT9Z/m4O8/AwfznCzuM8yO9cB+dCOxpMVoFZlLXTuaCFOOo8J6dHy/0ydCxAOIQ\nVqZYJKGl8QKBgQC9GZa47Ds3RBhp14RVXxf0IYwtMyt6VpMbB/rUxcqTqN+y0Fcv\ngL7AnCEwExiQx4ok0k7sT8DVQW3eoLBBdPID0IVz3FTZoleB/+t8NK3PoFYTTfOT\nL2HP/1HpXsg094fAuu9aDo56GZjWYMGujdnX6uxumPj9fajNU/bLJWnVGwKBgAzT\nmj33vtTUwqW6GYtqyfKrQFevs1j6WOFoB6Pd7qBHaIC06B5gXWFrvsNY9zvSGS8r\nSpcfNStIUzCvU9JHaORnwWLE7thsNdtwFrOQOca/fUshqdR7ng1kguykvpOofTPf\n8ZHP2gH6VfjVr+N7GedbgXZhU4HLDtlzkThShZrRAoGBAKGlo85kSjGkEy1h/SXG\nniVXrflFIFlk0RAlAhL2xnCuTSdSNgGf4pJ3vcpT6Saq6AGR4OzN/YSxs9V0bpSJ\nz+w5K/KT1A3fKY15Fg6rzprAqkb9wZXUFgd5GgRNbde3rCLb9j7SMOmRwy3WshQz\n5a7FkKNVhvfx9SkJfUPPMR/v\n-----END PRIVATE KEY-----\n";

// Wifi data
#define WIFI_SSID "VOL 25"
#define WIFI_PASSWORD "135792468"

// Firebase communication variables
UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD);
FirebaseApp app;
DefaultNetwork network;
using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client, getNetwork(network));
Firestore::Documents Docs;
Firestore::CollectionGroups::Indexes indexes;
AsyncResult aResult_no_callback;


// IFTTT data
String eventNAME = "new_mail";
String webhooksKEY = "bk-rNjIlmxawG-VkvLWg9K";
const int httpsPort = 443;
String url = "https://maker.ifttt.com/trigger/" + eventNAME + "/with/key/" + webhooksKEY;

static Network_comm* instance_c = NULL;


Network_comm::Network_comm() {
  instance_c = this;
}

void Network_comm::initWiFi() {
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

void Network_comm::firebaseInit() {
  Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);

  initializeApp(aClient, app, getAuth(sa_auth), aResult_no_callback);

#if defined(ESP32) || defined(ESP8266) || defined(PICO_RP2040)
  ssl_client.setInsecure();
#if defined(ESP8266)
  ssl_client.setBufferSizes(4096, 1024);
#endif
#endif

  app.getApp<Firestore::Documents>(Docs);
}

void Network_comm::loopElements() {
  JWT.loop(app.getAuth());

  app.loop();

  Docs.loop();

}

bool Network_comm::firebaseReady() {
  return Firebase.ready();
}

std::vector<String> Network_comm::getBulbs(String location, String room) {
  FirebaseJsonData bulb;
  int count = 1;
  int index;
  String mask = "bulb_1";
  std::vector<String> bulbs;
  StructuredQuery query;

  Projection projection(FieldReference("IP"));
  query.select(projection);

  query.from(CollectionSelector("bulbs", false));

  // Creating the FieldFilter for the type == "props"
  FieldFilter typeFilter;
  typeFilter.field(FieldReference("type"));
  typeFilter.op(FieldFilterOperator::EQUAL);
  Values::StringValue typeString("props");
  typeFilter.value(Values::Value(typeString));        
  
  // Creating the FieldFilter for the location == "locationstring"
  FieldFilter locationFilter;
  locationFilter.field(FieldReference("location"));
  locationFilter.op(FieldFilterOperator::EQUAL);
  Values::StringValue locationString(location);
  locationFilter.value(Values::Value(locationString));

  // Creating the FieldFilter for the room == "main_room_string"
  FieldFilter roomFilter;
  roomFilter.field(FieldReference("room"));
  roomFilter.op(FieldFilterOperator::EQUAL);
  Values::StringValue roomString(room);
  roomFilter.value(Values::Value(roomString));

  // Create compositeFilter from the above
  CompositeFilter bulbFilter;
  bulbFilter.op(CompositFilterOperator::AND);
  bulbFilter.addFilter(FirestoreQuery::Filter(typeFilter));
  bulbFilter.addFilter(FirestoreQuery::Filter(locationFilter));
  bulbFilter.addFilter(FirestoreQuery::Filter(roomFilter));

  query.where(FirestoreQuery::Filter(bulbFilter));

  query.limit(1);

  QueryOptions queryOptions;
  queryOptions.structuredQuery(query);

  query.clear();
  projection.clear();
  typeFilter.clear();
  locationFilter.clear();
  roomFilter.clear();
  bulbFilter.clear();

  String documentPath = "/"; // Query from all collections under root

  // You can set the content of queryOptions object directly with queryOptions.setContent("your content")

  Docs.runQuery(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), documentPath, queryOptions, aResult_no_callback);
  queryOptions.clear();

  FirebaseJson resultJSON(aResult_no_callback.c_str());
  while (resultJSON.get(bulb, "[0]/document/fields/IP/mapValue/fields/" + mask + "/stringValue")) {
    bulbs.push_back(bulb.to<String>());
    mask.replace(mask.substring(mask.indexOf("_") + 1), String(++count));
  }

}

double Network_comm::getTemperatureData(String location, String room) {
  FirebaseJsonData resultTemp;
  StructuredQuery query;

  Projection projection(FieldReference("val"));
  query.select(projection);

  query.from(CollectionSelector("temperature", false));

  // Creating the FieldFilter for the type == "props"
  FieldFilter typeFilter;
  typeFilter.field(FieldReference("type"));
  typeFilter.op(FieldFilterOperator::EQUAL);
  Values::StringValue typeString("set_temp");
  typeFilter.value(Values::Value(typeString));        
  
  // Creating the FieldFilter for the location == "locationstring"
  FieldFilter locationFilter;
  locationFilter.field(FieldReference("location"));
  locationFilter.op(FieldFilterOperator::EQUAL);
  Values::StringValue locationString("location_1_string");
  locationFilter.value(Values::Value(locationString));

  // Creating the FieldFilter for the room == "main_room_string"
  FieldFilter roomFilter;
  roomFilter.field(FieldReference("room"));
  roomFilter.op(FieldFilterOperator::EQUAL);
  Values::StringValue roomString("main_room_string");
  roomFilter.value(Values::Value(roomString));

  // Create compositeFilter from the above
  CompositeFilter tempFilter;
  tempFilter.op(CompositFilterOperator::AND);
  tempFilter.addFilter(FirestoreQuery::Filter(typeFilter));
  tempFilter.addFilter(FirestoreQuery::Filter(locationFilter));
  tempFilter.addFilter(FirestoreQuery::Filter(roomFilter));
  query.where(FirestoreQuery::Filter(tempFilter));

  // Create the ordering of the query
  Order order;
  order.field(FieldReference("timestamp"));
  order.direction(FilterSort::DESCENDING);
  query.orderBy(order);

  query.limit(1);

  QueryOptions queryOptions;
  queryOptions.structuredQuery(query);

  query.clear();
  projection.clear();
  typeFilter.clear();
  locationFilter.clear();
  roomFilter.clear();
  tempFilter.clear();
  typeString.clear();
  locationString.clear();
  roomString.clear();
  order.clear();

  String documentPath = "/"; // Query from all collections under root

  // You can set the content of queryOptions object directly with queryOptions.setContent("your content")

  Docs.runQuery(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), documentPath, queryOptions, aResult_no_callback);
  queryOptions.clear();

  Serial.printf("ok temperature\n%s\n\n", aResult_no_callback.c_str());
  FirebaseJson resultJSON(aResult_no_callback.c_str());
  resultJSON.get(resultTemp, "[0]/document/fields/val/doubleValue/");
  if (resultTemp.to<double>() == 0.0) resultJSON.get(resultTemp, "[0]/document/fields/val/integerValue/");
  return resultTemp.to<double>();
}

std::vector<double> Network_comm::getTransitionFunctionData(String location, String room) {

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
  query.set("where/compositeFilter/filters/[2]/fieldFilter/value/stringValue", location);
  query.set("where/compositeFilter/filters/[0]/fieldFilter/field/fieldPath", "room");
  query.set("where/compositeFilter/filters/[0]/fieldFilter/op", "EQUAL");
  query.set("where/compositeFilter/filters/[0]/fieldFilter/value/stringValue", room);
  query.set("where/compositeFilter/filters/[1]/fieldFilter/field/fieldPath", "type");
  query.set("where/compositeFilter/filters/[1]/fieldFilter/op", "EQUAL");
  query.set("where/compositeFilter/filters/[1]/fieldFilter/value/stringValue", "transition_data");

  //ORDERING
  query.set("orderBy/field/fieldPath", "ts");
  query.set("orderBy/direction", "DESCENDING");
  query.set("limit", 1);

  if (Firebase.Firestore.runQuery(&fbdo, FIREBASE_PROJECT_ID, "", "/", &query)) {
    Serial.printf("ok transition\n%s\n\n", fbdo.payload().c_str());
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

bool Network_comm::writeTemperatureData(double temp, String location, String room, String ts) {  // TODO: implement for the new DB

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

bool Network_comm::writeBulbState(String IP, int hue, int sat, int rgb, int ct, String location, String room, String ts) {  

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

bool Network_comm::setModifiedThermostatTemperature(double temp, String location, String room, String ts) {  // TODO: implement for the new DB

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

int Network_comm::postWebhooks(String value1) {

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

void Network_comm::timeStatusCB(uint32_t &ts)
{
#if defined(ESP8266) || defined(ESP32) || defined(CORE_ARDUINO_PICO)
    if (time(nullptr) < FIREBASE_DEFAULT_TS)
    {

        configTime(3 * 3600, 0, "pool.ntp.org");
        while (time(nullptr) < FIREBASE_DEFAULT_TS)
        {
            delay(100);
        }
    }
    ts = time(nullptr);
#elif __has_include(<WiFiNINA.h>) || __has_include(<WiFi101.h>)
    ts = WiFi.getTime();
#endif
}