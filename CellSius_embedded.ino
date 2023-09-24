
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

std::vector<Yeelight*> bulbs;
std::vector<String> IPs;
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


  IPs = network->getBulbs("home_1/bulbs");

  for (int i = 0; i < IPs.size(); i++) {
    bulbs.push_back(new Yeelight(IPs[i], 55443));
  }

  // Init of yeelight
  for (Yeelight* bulb : bulbs) {
    bulb->on();
  }
}

void loop() {

  // Firebase.ready() should be called repeatedly to handle authentication tasks.

  if (network->firebaseReady() && (millis() - dataMillis > 10000 || dataMillis == 0)) {
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
      for (Yeelight* bulb : bulbs) {
        Serial.println("getIPs ------- : " + bulb->getIP());
        bulb->setColorTemp(1800, "smooth", 1000);
        bulb->feedback();
      }

      digitalWrite(LED, LOW);


    } else if (temp < old_temp) {
      Serial.println("Its getting cold");
      digitalWrite(LED, HIGH);
      for (Yeelight* bulb : bulbs) {
        Serial.println("getIPs ------- : " + bulb->getIP());
        bulb->setRGB(150, 150, 255, "smooth", 1000);
      }

      digitalWrite(LED, LOW);

    } else {
      Serial.println("everything stays the same");
      digitalWrite(LED, HIGH);
      for (Yeelight* bulb : bulbs) {
        Serial.println("getIPs ------- : " + bulb->getIP());
        bulb->setColorTemp(4000, "smooth", 1000);
      }

      digitalWrite(LED, LOW);
    }
    old_temp = temp;
  }
}

void initNetwork() {
  network = new Network();
  network->initWiFi();
}