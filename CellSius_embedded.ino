
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
#include <math.h>
#include "Adafruit_MCP9808.h"
#include "Network.h"
#include "yeelight.h"

// I2C communication with MCP9809
#define SDA_0 18
#define SCL_0 19

#define LED 2

Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();
TwoWire wires = (TwoWire(0));

std::vector<Yeelight*> bulbs;
std::vector<String> IPs;
Network* network;

unsigned long dataMillis = 0;
int count = 0;

double old_temp = 0.0;
double temp;
int bulb_trans_time = 5000;

// bool taskCompleted = false;

void setup() {

  Serial.begin(115200);

  // Init of network and firebase
  initNetwork();
  network->firebaseInit();

  IPs = network->getBulbs("home_1/bulbs");


  for (int i = 0; i < IPs.size(); i++) {
    bulbs.push_back(new Yeelight(IPs[i], 55443));
    Serial.println("IPs: ");
    Serial.println(IPs[i]);
  }

  // Init of yeelight
  for (Yeelight* bulb : bulbs) {
    Serial.print("bulb on: ");
    int count = 0;
    while(bulb->on() == "false" && count <= 10) {
      Serial.print(".");
      delay(500);
    }
    if (count = 10) Serial.println("Couldn't connect to bulb");
    else Serial.println("Connected");
  }

  wires.setPins(SDA_0, SCL_0);
  if (!tempsensor.begin(0x18, &wires)) {
    Serial.println("Couldn't find MCP9808! Check your connections and verify the address is correct.");
    while (1)
      ;
  }

  Serial.println("Found MCP9808!");
  tempsensor.setResolution(3);
}

void loop() {

  if (network->firebaseReady() && (millis() - dataMillis > 5000 || dataMillis == 0)) {

    // sensor measurement
    tempsensor.wake();
    double c = (double)tempsensor.readTempC();
    Serial.print("Temp: ");
    Serial.print(c, 4);
    Serial.print("°C\t  ");
    tempsensor.shutdown_wake(1);  

    String documentPath = "home_1/set_thermostat_temp";
    String mask = "temp";

    temp = network->getTemperatureData(documentPath, mask).toDouble();
    Serial.print("writing temp data:");
    Serial.println(network->writeTemperatureData(c, "home_1/measured_temperature"));
    double delta = c - temp;
    Serial.print("transzfer:  ");
    Serial.println(transferFunction(delta, 4000, 1700, 6500, 0.2));
    if (temp != old_temp || std::abs(delta) > 0.5) {
      if (delta < -1) {
        Serial.println("Its getting hot");
        digitalWrite(LED, HIGH);
        for (Yeelight* bulb : bulbs) {
          Serial.println("getIPs ------- : " + bulb->getIP());
          // Serial.println(bulb->setBrightness(30, "smooth", 100));

          Serial.println(bulb->setColorTemp(transferFunction(delta, 4000, 1700, 6500, 0.2), "smooth", bulb_trans_time));
          // Serial.println(bulb->setBrightness(100, "smooth", 100));
        }

        digitalWrite(LED, LOW);


      } else if (delta > 1) {
        Serial.println("Its getting cold");
        digitalWrite(LED, HIGH);
        for (Yeelight* bulb : bulbs) {
          Serial.println("getIPs ------- : " + bulb->getIP());

          Serial.println(bulb->setColorTemp(transferFunction(delta, 4000, 1700, 6500, 0.2), "smooth", bulb_trans_time));
          
        }

        digitalWrite(LED, LOW);

      } else {
        Serial.println("everything stays the same");
        digitalWrite(LED, HIGH);
        for (Yeelight* bulb : bulbs) {
          Serial.println("getIPs ------- : " + bulb->getIP());
          Serial.println(bulb->setColorTemp(transferFunction(delta, 4000, 1700, 6500, 0.2), "smooth", bulb_trans_time));
        }

        digitalWrite(LED, LOW);
      }

      delay(bulb_trans_time);
      Serial.println("Temperature changeing over");

    } else {

      Serial.println("everything stays the same");
      digitalWrite(LED, HIGH);
      for (Yeelight* bulb : bulbs) {
        Serial.println("getIPs ------- : " + bulb->getIP());
        Serial.println(bulb->setColorTemp(transferFunction(delta, 4000, 1700, 6500, 0.2), "smooth", bulb_trans_time));
      }

      digitalWrite(LED, LOW);

      Serial.print("Temperature stays: ");
      Serial.println(temp);
    }
    dataMillis = millis();
    old_temp = temp;
  }
}

void initNetwork() {
  network = new Network();
  network->initWiFi();
}

int transferFunction(double deltaT, int zero, int min, int max, double speed) {
  // Serial.print("Tan: ");
  // Serial.println(tanh(deltaT*speed));
  // Serial.print("Abs: ");
  // Serial.println(std::abs((max - min) / 2);
  int y = (int)(std::abs((max - min) / 2)*tanh(deltaT*speed)) + zero;
  return y;
}