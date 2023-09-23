
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

#include "Network.h"
#include "yeelight.h"

#define LED 2

Yeelight* yeelight;
Network* network;

unsigned long dataMillis = 0;
int count = 0;

double old_temp = 0.0;
double temp;

bool taskCompleted = false;

void setup() {
  Serial.begin(115200);


  // Init of network and firebase
  initNetwork();
  network->firebaseInit();
  // Init of yeelight
  yeelight = new Yeelight("172.20.10.3", 55443);
  yeelight->setBrightness(50, "smooth", 1000);
}

void loop() {

  // Firebase.ready() should be called repeatedly to handle authentication tasks.

  if ((millis() - dataMillis > 5000 || dataMillis == 0)) {
    dataMillis = millis();

    String documentPath = "home_1/set_thermostat_temp";
    String mask = "temp";

    temp = network->getTemperatureData(documentPath, mask).toDouble();

    Serial.print("received temp: ");
    Serial.println(temp);
    Serial.print("old temp: ");
    Serial.println(old_temp);
    if (temp > old_temp) {
      Serial.println("Its getting hot");
      digitalWrite(LED, HIGH);
      yeelight->setColorTemp(1800, "smooth", 1000);
      digitalWrite(LED, LOW);


    } else if (temp < old_temp) {
      Serial.println("Its getting cold");
      digitalWrite(LED, HIGH);
      yeelight->setRGB(100, 100, 255, "smooth", 1000);
      digitalWrite(LED, LOW);

    } else {
      Serial.println("everything stays the same");
      digitalWrite(LED, HIGH);
      yeelight->setColorTemp(4000, "smooth", 1000);
      digitalWrite(LED, LOW);
    }
    old_temp = temp;
  }
}

void initNetwork() {
  network = new Network();
  network->initWiFi();
}