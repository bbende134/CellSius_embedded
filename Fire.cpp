#include "fire.h"

DefaultNetwork defNetwork;
void timeStatusCB(uint32_t &ts);
void authHandler();
void printError(int code, const String &msg);
void printResult(AsyncResult &aResult);

ServiceAuth sa_auth(timeStatusCB, FIREBASE_CLIENT_EMAIL, FIREBASE_PROJECT_ID, PRIVATE_KEY, 3000 /* expire period in seconds (<= 3600) */);

// UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD);

FirebaseApp app;
WiFiClientSecure ssl_client;

using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client, getNetwork(defNetwork));
Firestore::CollectionGroups::Indexes indexes;

Firestore::Documents Docs;
AsyncResult aResult_no_callback;

static Fire* instance = NULL;


Fire::Fire() {
  instance = this;
}


void Fire::initWiFi() {
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);


  Serial.print("Connecting to Wi-Fi");
  unsigned long ms = millis();
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

void Fire::firebaseInit() {
  Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);

  ssl_client.setInsecure();

  initializeApp(aClient, app, getAuth(sa_auth), aResult_no_callback);

  authHandler();

  app.getApp<Firestore::Documents>(Docs);

  aClient.setAsyncResult(aResult_no_callback);

  Serial.println("Seccessfully init firebase");
}
bool Fire::firebaseReady() {
  return app.ready();
}
void Fire::loopElements() {

  authHandler();

  Docs.loop();
}

std::vector<String> Fire::getBulbs(String location, String room) {
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
  // Docs.runQuery(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), documentPath, queryOptions, aResult_no_callback); // That was for async
  String payload = Docs.runQuery(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), documentPath, queryOptions);

  queryOptions.clear();

  if (payload.length() != 0)
  {
    Serial.println("Calling successful");
    FirebaseJson resultJSON(payload);
    while (resultJSON.get(bulb, "[0]/document/fields/IP/mapValue/fields/" + mask + "/stringValue")) {
      bulbs.push_back(bulb.to<String>());
      mask.replace(mask.substring(mask.indexOf("_") + 1), String(++count));
    }
    return bulbs;
  }
  else {
    printError(aClient.lastError().code(), aClient.lastError().message());
    return bulbs;
  }

}

double Fire::getTemperatureData(String location, String room) {
  /*Query to get the actual wanted temperature in the office*/

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
  Serial.println("now comes the query");

  // while(aResult_no_callback.isError()) Docs.runQuery(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), documentPath, queryOptions, aResult_no_callback);
  String payload = Docs.runQuery(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), documentPath, queryOptions);

  queryOptions.clear();

  if (payload.length() != 0)
  {
    Serial.printf("ok temperature\n%s\n\n", payload);
    FirebaseJson resultJSON(payload);
    resultJSON.get(resultTemp, "[0]/document/fields/val/doubleValue/");
    if (resultTemp.to<double>() == 0.0) resultJSON.get(resultTemp, "[0]/document/fields/val/integerValue/");
    return resultTemp.to<double>();
  }
  else {
    printError(aClient.lastError().code(), aClient.lastError().message());
    return resultTemp.to<double>();
  }
 
}

std::vector<double> Fire::getTransitionFunctionData(String location, String room) {
  /*Query to get the transitionfunction for the bulbs*/

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

  // Docs.runQuery(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), documentPath, queryOptions, aResult_no_callback);

  String payload = Docs.runQuery(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), documentPath, queryOptions);

  queryOptions.clear();

  if (payload.length() != 0)
  {
    Serial.println("Calling successful");
    // Serial.printf("ok transition\n%s\n\n", payload);
    FirebaseJson resultJSON(payload);

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
  else {
    printError(aClient.lastError().code(), aClient.lastError().message());
    return transitionData;
  }

}

bool Fire::writeTemperatureData(double temp, String location, String room, String ts) {  // TODO: implement for the new DB
  /*Write the actual measured temperature*/

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

  String payload = Docs.createDocument(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), documentPath, DocumentMask(), doc);
  
  if (aClient.lastError().code() == 0)
  {
    Serial.println(payload);
    return 1;
  }
  else {
    printError(aClient.lastError().code(), aClient.lastError().message());
    return 0;
  }
}

bool Fire::writeBulbState(String IP, int hue, int sat, int rgb, int ct, String location, String room, String ts) {  
  /*Write the actual setted bulb state*/

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

  String payload = Docs.createDocument(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), documentPath, DocumentMask(), doc);
  
  if (aClient.lastError().code() == 0)
  {
    Serial.println(payload);
    return 1;
  }
  else {
    printError(aClient.lastError().code(), aClient.lastError().message());
    return 0;
  }
}

bool Fire::setModifiedThermostatTemperature(double temp, String location, String room, String ts) {  // TODO: implement for the new DB
  /*Write the modified temperature(taht's differs from the ones that setted)*/

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
  doc.add("type", Values::Value(typeString)).add("room", Values::Value(roomString));
  doc.add("location", Values::Value(locationString));

  // The value of Values::xxxValue, Values::Value and Document can be printed on Serial.

  Serial.println("Create document... ");

  String payload = Docs.createDocument(aClient, Firestore::Parent(FIREBASE_PROJECT_ID), documentPath, DocumentMask(), doc);
  
  if (aClient.lastError().code() == 0)
  {
    Serial.println(payload);
    return 1;
  }
  else {
    printError(aClient.lastError().code(), aClient.lastError().message());
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

void authHandler()
{
  // Blocking authentication handler with timeout
  unsigned long ms = millis();
  while (app.isInitialized() && !app.ready() && millis() - ms < 120 * 1000)
  {
    // The JWT token processor required for ServiceAuth and CustomAuth authentications.
    // JWT is a static object of JWTClass and it's not thread safe.
    // In multi-threaded operations (multi-FirebaseApp), you have to define JWTClass for each FirebaseApp,
    // and set it to the FirebaseApp via FirebaseApp::setJWTProcessor(<JWTClass>), before calling initializeApp.
    JWT.loop(app.getAuth());
    printResult(aResult_no_callback);
  }
}
void printResult(AsyncResult &aResult)
{
  if (aResult.isEvent())
  {
    Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.appEvent().message().c_str(), aResult.appEvent().code());
  }

  if (aResult.isDebug())
  {
    Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());
  }

  if (aResult.isError())
  {
    Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());
  }

  if (aResult.available())
  {
    Firebase.printf("task: %s, payload: %s\n", aResult.uid().c_str(), aResult.c_str());
  }
}

void printError(int code, const String &msg)
{
  Firebase.printf("Error, msg: %s, code: %d\n", msg.c_str(), code);
}