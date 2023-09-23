
/**
 * Created by K. Suwatchai (Mobizt)
 *
 * Email: k_suwatchai@hotmail.com
 *
 * Github: https://github.com/mobizt/Firebase-ESP-Client
 *
 * Copyright (c) 2023 mobizt
 *
 */

// This example shows how to get a document from a document collection. This operation required Email/password, custom or OAUth2.0 authentication.

#include <Arduino.h>
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

// Provide the token generation process info.
#include <addons/TokenHelper.h>
#include "yeelight.h"

/* 1. Define the WiFi credentials */
#define API_KEY "AIzaSyAHjkFGALD1RHc0IVNz8pPhM_HrjqqsPOA"
#define FIREBASE_PROJECT_ID "test-600f6"
#define USER_EMAIL "admin@admin.com"
#define USER_PASSWORD "admin1234"

#define WIFI_SSID "VOL_25"
#define WIFI_PASSWORD "135792468"

#define LED 2
// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

StaticJsonDocument<200> jsonBuffer;
Yeelight* yeelight;

double old_temp = 0.0;
double temp;

bool taskCompleted = false;

unsigned long dataMillis = 0;

#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
WiFiMulti multi;
#endif

void setup() {
  WiFi.mode(WIFI_STA);
  Serial.begin(115200);

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

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  // The WiFi credentials are required for Pico W
  // due to it does not have reconnect feature.
#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
  config.wifi.clearAP();
  config.wifi.addAP(WIFI_SSID, WIFI_PASSWORD);
#endif

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;  // see addons/TokenHelper.h

  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);

  // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

  // Limit the size of response payload to be collected in FirebaseData
  fbdo.setResponseSize(2048);

  Firebase.begin(&config, &auth);
  yeelight = new Yeelight("172.20.10.3", 55443);
  // You can use TCP KeepAlive in FirebaseData object and tracking the server connection status, please read this for detail.
  // https://github.com/mobizt/Firebase-ESP-Client#about-firebasedata-object
  // fbdo.keepAlive(5, 5, 1);
  yeelight->setBrightness(50, "smooth", 1000);
}

void loop() {

  // Firebase.ready() should be called repeatedly to handle authentication tasks.

  if (Firebase.ready() && (millis() - dataMillis > 60000 || dataMillis == 0)) {
    dataMillis = millis();

    // if (!taskCompleted) {
    //   taskCompleted = true;

    //   // For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create_Edit_Parse/Create_Edit_Parse.ino
    //   FirebaseJson content;

    //   content.set("fields/Japan/mapValue/fields/time_zone/integerValue", "9");
    //   content.set("fields/Japan/mapValue/fields/population/integerValue", "125570000");

    //   content.set("fields/Belgium/mapValue/fields/time_zone/integerValue", "1");
    //   content.set("fields/Belgium/mapValue/fields/population/integerValue", "11492641");

    //   content.set("fields/Singapore/mapValue/fields/time_zone/integerValue", "8");
    //   content.set("fields/Singapore/mapValue/fields/population/integerValue", "5703600");

    //   // info is the collection id, countries is the document id in collection info.
    //   String documentPath = "info/countries";

    //   Serial.print("Create document... ");

    //   if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, documentPath.c_str(), content.raw()))
    //     Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
    //   else
    //     Serial.println(fbdo.errorReason());
    // }

    String documentPath = "home_1/set_thermostat_temp";
    String mask = "temp";

    FirebaseJsonData result;
    // If the document path contains space e.g. "a b c/d e f"
    // It should encode the space as %20 then the path will be "a%20b%20c/d%20e%20f"

    if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), mask.c_str())) {
      //getting only temp data from received data
      FirebaseJson thermostatJSON(fbdo.payload().c_str());
      thermostatJSON.get(result, "fields/temp/stringValue");
      Serial.print("received temp: ");
      Serial.println(result.to<String>().c_str());

      temp = std::stod(result.to<String>().c_str());
      // if (yeelight->feedback()) {
      //   Serial.print("device: ");
      //   Serial.println(yeelight->getLocation());
      //   if (!yeelight->isPowered()) {
      //     digitalWrite(LED, HIGH);
      //     Serial.println(yeelight->sendCommand("set_power", "[\"on\", \"smooth\", 500]"));
      //     Serial.println(yeelight->sendCommand("set_name", "[\"testbulb\"]"));
      //     Serial.println("Power is now on");
      //   }
      // }

      Serial.print("Get a document... ");
      Serial.println(old_temp);
      if (temp > old_temp) {
        Serial.println("Its getting hot");
        digitalWrite(LED, HIGH);
        yeelight->setColorTemp(1800, "smooth", 1000);
        digitalWrite(LED, LOW);


      } else if (temp < old_temp) {
        Serial.println("Its getting cold");
        digitalWrite(LED, HIGH);
        yeelight->setRGB(100, 100,255, "smooth", 1000);
        digitalWrite(LED, LOW);

      } else {
        Serial.println("everything stays the same");
        digitalWrite(LED, HIGH);
        yeelight->setColorTemp(4000, "smooth", 1000);
        digitalWrite(LED, LOW);
      }

      old_temp = std::stod(result.to<String>().c_str());



    } else {
      Serial.println(fbdo.errorReason());
    }
  }
}