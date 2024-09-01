#include <vector>
#include <NTPClient.h>
#include <WiFiUdp.h>
// #include <chrono>
#include "Network_comm.h"
#include <HTTPClient.h>
#include <ESPDateTime.h>
#include <FirebaseJson.h>

#if defined(ESP32) || defined(ESP8266) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFiClientSecure.h>
WiFiClientSecure ssl_client;
#elif defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_UNOWIFIR4) || defined(ARDUINO_GIGA) || defined(ARDUINO_PORTENTA_C33) || defined(ARDUINO_NANO_RP2040_CONNECT)
#include <WiFiSSLClient.h>
WiFiSSLClient ssl_client;
#endif

// Firebase data
#define API_KEY "AIzaSyAgD73O-useNuuE2X6T-Olho-32sgXmNIk"
#define FIREBASE_PROJECT_ID "cellsius-demo"
#define FIREBASE_CLIENT_EMAIL "firebase-adminsdk-6qn3e@cellsius-demo.iam.gserviceaccount.com"
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----\nMIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDCyh+KHejbHA3A\nmPvoZl0m2EQgCjJSQIWdrYaY1UQBOiED4fNZSsKL3RITCh85oPx98lLtFDE34ACz\nq8XVKc9yccOzgtu8WrLDiqa+M7xwh6VBr2iSDasJP+1Xv5GkkL+uq4qkwdU2MOj4\nLhl+K2RjKkZU+6gqEXKsQMk39EINMGIVpaZn56PMKsZXZ6MrGXQaRYrHIVUPGirC\ni5qEYwbDLbq08VqH8hc/jSKGA+gsmVNZaMWlqYL+Ccekph/ZEz/oh2/qSL6f9jj1\nDKLRGV8Iq8wgZs430olFRPC0kGdn0pLx16rMfGZJicCQzFC6iSX0r80FC8z2c/iN\n2RdF+aIHAgMBAAECggEABYyhgz3Pgz1IIhg5qLLVj5+XM7nbkDncgPF7tCtBskJU\nddfbTlYUI5HFENIaO3oSBjDYhwiNdwZar/7HQGBUQAXO7EM5Mw+YlHwE5BUSyAlQ\nVwYavSwBlxlBSvr8pRJYKJHB6gXbpv+T3pu2iA8x8xxdhJWrvWmJOZLBY9zaMS/L\nj4neAjsfeW1bgxeF7V/VjWpz64IlY1naMEr0ua9zkk9uxDbgtA6UUCAuAiuvIBXy\nuf2sikuZ3ddY2TKL/OZJlEnId0ZlzmbKYOpjXJM+iq/KTrxyF92mM8ZXAjNfnHdq\nbPGKmX9Plony9J+f6ZpdKMBBLXs5GPw9NVBFrBWKAQKBgQDqnVJ2qcndEt2JgX25\nXb15lByMe8eYc9/FC1XCnmy//cR+pUfdueJHZqtg4p6aERVFwYxjTjOoVJMd5mVj\nCCJurKTHT5huzbb3gGR5wta+im4s+/KmHnMjG5cH6GVngLLt6+hG5um0e/IZuPtF\nfWlarK2Q0kDJ375z5l96nAJvdwKBgQDUi35i52PptQLQmkPNyuG8oX9hjFU56Xcm\nDeto5Y7eaIWbPqVE94E/kjue4+F/y4F5REXNwgJcqHQO68XZPiZvw0V8wHoJIVpn\nCzT9Z/m4O8/AwfznCzuM8yO9cB+dCOxpMVoFZlLXTuaCFOOo8J6dHy/0ydCxAOIQ\nVqZYJKGl8QKBgQC9GZa47Ds3RBhp14RVXxf0IYwtMyt6VpMbB/rUxcqTqN+y0Fcv\ngL7AnCEwExiQx4ok0k7sT8DVQW3eoLBBdPID0IVz3FTZoleB/+t8NK3PoFYTTfOT\nL2HP/1HpXsg094fAuu9aDo56GZjWYMGujdnX6uxumPj9fajNU/bLJWnVGwKBgAzT\nmj33vtTUwqW6GYtqyfKrQFevs1j6WOFoB6Pd7qBHaIC06B5gXWFrvsNY9zvSGS8r\nSpcfNStIUzCvU9JHaORnwWLE7thsNdtwFrOQOca/fUshqdR7ng1kguykvpOofTPf\n8ZHP2gH6VfjVr+N7GedbgXZhU4HLDtlzkThShZrRAoGBAKGlo85kSjGkEy1h/SXG\nniVXrflFIFlk0RAlAhL2xnCuTSdSNgGf4pJ3vcpT6Saq6AGR4OzN/YSxs9V0bpSJ\nz+w5K/KT1A3fKY15Fg6rzprAqkb9wZXUFgd5GgRNbde3rCLb9j7SMOmRwy3WshQz\n5a7FkKNVhvfx9SkJfUPPMR/v\n-----END PRIVATE KEY-----\n";

// Wifi data
#define WIFI_SSID "VOL 23"
#define WIFI_PASSWORD "135792468"

#define USER_EMAIL "admin@admin.com"
#define USER_PASSWORD "admin1234"

#define FIREBASE_PROJECT_ID "cellsius-demo"

void timeStatusCB(uint32_t &ts);

// Firebase communication variables
// ServiceAuth sa_auth(timeStatusCB, FIREBASE_CLIENT_EMAIL, FIREBASE_PROJECT_ID, PRIVATE_KEY, 3000 /* expire period in seconds (<= 3600) */);
UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD);
DefaultNetwork defNetwork;
using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client, getNetwork(defNetwork));
Firestore::CollectionGroups::Indexes indexes;


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

  initializeApp(aClient, app, getAuth(user_auth), aResult_no_callback);

#if defined(ESP32) || defined(ESP8266) || defined(PICO_RP2040)
  ssl_client.setInsecure();
#if defined(ESP8266)
  ssl_client.setBufferSizes(4096, 1024);
#endif
#endif

  app.getApp<Firestore::Documents>(Docs);
}

void Network_comm::loopElements() {
  // JWT.loop(app.getAuth());

  app.loop();

  Docs.loop();

}

bool Network_comm::firebaseReady() {
  return app.ready();
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

  if (aResult_no_callback.available())
  {
    FirebaseJson resultJSON(aResult_no_callback.c_str());
    while (resultJSON.get(bulb, "[0]/document/fields/IP/mapValue/fields/" + mask + "/stringValue")) {
      bulbs.push_back(bulb.to<String>());
      mask.replace(mask.substring(mask.indexOf("_") + 1), String(++count));
    }
    return bulbs;
  }
  else if (aResult_no_callback.isError()) {
    Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult_no_callback.uid().c_str(), aResult_no_callback.error().message().c_str(), aResult_no_callback.error().code());
    return bulbs;
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
  Values::StringValue locationString(location);
  locationFilter.value(Values::Value(locationString));

  // Creating the FieldFilter for the room == "main_room_string"
  FieldFilter roomFilter;
  roomFilter.field(FieldReference("room"));
  roomFilter.op(FieldFilterOperator::EQUAL);
  Values::StringValue roomString(room);
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
  if (aResult_no_callback.available())
  {
    Serial.printf("ok temperature\n%s\n\n", aResult_no_callback.c_str());
    FirebaseJson resultJSON(aResult_no_callback.c_str());
    resultJSON.get(resultTemp, "[0]/document/fields/val/doubleValue/");
    if (resultTemp.to<double>() == 0.0) resultJSON.get(resultTemp, "[0]/document/fields/val/integerValue/");
    return resultTemp.to<double>();
  }
  else if (aResult_no_callback.isError()) {
    Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult_no_callback.uid().c_str(), aResult_no_callback.error().message().c_str(), aResult_no_callback.error().code());
    return resultTemp.to<double>();
  }
}

std::vector<double> Network_comm::getTransitionFunctionData(String location, String room) {

  // Variable definition
  FirebaseJsonData resultData;
  std::vector<double> transitionData;
  StructuredQuery query;

  // Select collection
  query.from(CollectionSelector("bulbs", false));

  // Select the field
  Projection projection(FieldReference("zero"));
  projection.add(FieldReference("min"));
  projection.add(FieldReference("max"));
  projection.add(FieldReference("speed"));
  query.select(projection);

  // Creating the FieldFilter for the type == ""
  FieldFilter typeFilter;
  typeFilter.field(FieldReference("type"));
  typeFilter.op(FieldFilterOperator::EQUAL);
  Values::StringValue typeString("transition_data");
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
  CompositeFilter transFilter;
  transFilter.op(CompositFilterOperator::AND);
  transFilter.addFilter(FirestoreQuery::Filter(typeFilter));
  transFilter.addFilter(FirestoreQuery::Filter(locationFilter));
  transFilter.addFilter(FirestoreQuery::Filter(roomFilter));
  query.where(FirestoreQuery::Filter(transFilter));

  // Create the ordering of the query
  Order order;
  order.field(FieldReference("ts"));
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
  transFilter.clear();
  typeString.clear();
  locationString.clear();
  roomString.clear();
  order.clear();

  String documentPath = "/"; // Query from all collections under root

  // You can set the content of queryOptions object directly with queryOptions.setContent("your content")

  Docs.runQuery(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), documentPath, queryOptions, aResult_no_callback);
  queryOptions.clear();

  if (aResult_no_callback.available())
  {
    Serial.printf("ok transition\n%s\n\n", aResult_no_callback.c_str());
    FirebaseJson resultJSON(aResult_no_callback.c_str());

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
  }
  else if (aResult_no_callback.isError()) {
    Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult_no_callback.uid().c_str(), aResult_no_callback.error().message().c_str(), aResult_no_callback.error().code());
    return transitionData;
  }

}

bool Network_comm::writeTemperatureData(double temp, String location, String room, String ts) {  // TODO: implement for the new DB

  ts.remove(ts.length() - 5, 5);
  ts += "Z";

  Values::TimestampValue tsV(ts);

  String documentPath = "temperature/acutal_temp_" + String(tsV.c_str());

  Values::DoubleValue temperature(temp);
  Values::StringValue typeString("actual_temperature");
  Values::StringValue locationString(location);
  Values::StringValue roomString(room);

  String doc_path = "projects/";
  doc_path += FIREBASE_PROJECT_ID;
  doc_path += "/databases/(default)/documents/coll_id/doc_id"; // coll_id and doc_id are your collection id and document id

  Document<Values::Value> doc("value", Values::Value(temperature));
  doc.add("ts", Values::Value(tsV)).add("type", Values::Value(typeString)).add("room", Values::Value(roomString));
  doc.add("location", Values::Value(locationString));

  // The value of Values::xxxValue, Values::Value and Document can be printed on Serial.

  Serial.println("Create document... ");

  Docs.createDocument(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), documentPath, DocumentMask(), doc, aResult_no_callback);
  if (aResult_no_callback.available())
  {
    Firebase.printf("task: %s, payload: %s\n", aResult_no_callback.uid().c_str(), aResult_no_callback.c_str());
    return 1;
  }
  else {
    Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult_no_callback.uid().c_str(), aResult_no_callback.error().message().c_str(), aResult_no_callback.error().code());
    return 0;
  }
}

bool Network_comm::writeBulbState(String IP, int hue, int sat, int rgb, int ct, String location, String room, String ts) {  

  ts.remove(ts.length() - 5, 5);
  ts += "Z";

  Values::TimestampValue tsV(ts);
  String documentPath = "bulbs/actual_state_" + String(tsV.c_str()) + "_" + IP;

  Values::StringValue IPV(IP);
  Values::StringValue typeString("state");
  Values::IntegerValue hueV(hue);
  Values::IntegerValue satV(sat);
  Values::IntegerValue rgbV(rgb);
  Values::IntegerValue ctV(ct);
  Values::StringValue locationString(location);
  Values::StringValue roomString(room);

  String doc_path = "projects/";
  doc_path += FIREBASE_PROJECT_ID;
  doc_path += "/databases/(default)/documents/coll_id/doc_id"; // coll_id and doc_id are your collection id and document id

  Document<Values::Value> doc("IP", Values::Value(IPV));
  doc.add("ts", Values::Value(tsV)).add("type", Values::Value(typeString)).add("room", Values::Value(roomString));
  doc.add("location", Values::Value(locationString)).add("hue", Values::Value(hueV)).add("sat", Values::Value(satV));
  doc.add("rgb", Values::Value(rgbV)).add("ct", Values::Value(ctV));

  // The value of Values::xxxValue, Values::Value and Document can be printed on Serial.

  Serial.println("Create document... ");

  Docs.createDocument(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), documentPath, DocumentMask(), doc, aResult_no_callback);
  if (aResult_no_callback.available())
  {    
    Firebase.printf("task: %s, payload: %s\n", aResult_no_callback.uid().c_str(), aResult_no_callback.c_str());
    return 1;
  }
  else {
    Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult_no_callback.uid().c_str(), aResult_no_callback.error().message().c_str(), aResult_no_callback.error().code());
    return 0;
  }
}

bool Network_comm::setModifiedThermostatTemperature(double temp, String location, String room, String ts) {  // TODO: implement for the new DB

  ts.remove(ts.length() - 5, 5);
  ts += "Z";

  Values::TimestampValue tsV(ts);

  String documentPath = "temperature/set_modified_thermostat_temperature_" + String(tsV.c_str());

  Values::DoubleValue temperature(temp);
  Values::StringValue typeString("actual_temperature");
  Values::StringValue locationString(location);
  Values::StringValue roomString(room);

  String doc_path = "projects/";
  doc_path += FIREBASE_PROJECT_ID;
  doc_path += "/databases/(default)/documents/coll_id/doc_id"; // coll_id and doc_id are your collection id and document id

  Document<Values::Value> doc("value", Values::Value(temperature));
  doc.add("ts", Values::Value(tsV)).add("type", Values::Value(typeString)).add("room", Values::Value(roomString));
  doc.add("location", Values::Value(locationString));

  // The value of Values::xxxValue, Values::Value and Document can be printed on Serial.

  Serial.println("Create document... ");

  Docs.createDocument(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), documentPath, DocumentMask(), doc, aResult_no_callback);
  if (aResult_no_callback.available())
  {
    Firebase.printf("task: %s, payload: %s\n", aResult_no_callback.uid().c_str(), aResult_no_callback.c_str());
    return 1;
  }
  else {
    Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult_no_callback.uid().c_str(), aResult_no_callback.error().message().c_str(), aResult_no_callback.error().code());
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

void timeStatusCB(uint32_t &ts)
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