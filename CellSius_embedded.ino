
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
#include <ESPDateTime.h>
#include "Adafruit_MCP9808.h"
#include "Network.h"
#include "yeelight.h"

// I2C communication with MCP9809
#define SDA_0 18
#define SCL_0 19

// location informations
#define location "location_1"
#define room "main_room"

#define LED 2

Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();
TwoWire wires = (TwoWire(0));

StaticJsonDocument<200> jsonBuffer;
std::vector<Yeelight*> bulbs;
std::vector<String> IPs;
Network* network;

unsigned long dataMillis = 0;
int count = 0;
String ts;

double old_temp = 0.0;

int bulb_trans_time = 50000;

// bool taskCompleted = false;

void setup() {

  Serial.begin(115200);

  // Init of network
  initNetwork();

  // set up datetime
  setupDateTime();

  // init firebase
  network->firebaseInit();

  // get bulbs for the actual room in the actual location
  while (IPs.empty()) IPs = network->getBulbs(location, room);


  for (int i = 0; i < IPs.size(); i++) {
    bulbs.push_back(new Yeelight(IPs[i], 55443));
    Serial.println("IPs: ");
    Serial.println(IPs[i]);
  }

  // Init of yeelight, make sure, all of them are turned on
  for (Yeelight* bulb : bulbs) {
    Serial.print("bulb on: ");
    int count = 0;
    Serial.println(bulb->on());

    Serial.println("Connected");
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

  if (network->firebaseReady() && (millis() - dataMillis > 1000 || dataMillis == 0)) {

    if (!DateTime.isTimeValid()) {
      Serial.println("Failed to get time from server, retry.");
      DateTime.begin();
    } else {

      ts = DateTime.formatUTC(DateFormatter::ISO8601);
    }
    // sensor measurement
    tempsensor.wake();
    double c = (double)tempsensor.readTempC();
    Serial.print("Room temp: ");
    Serial.print(c, 4);
    Serial.println("°C\t  ");
    tempsensor.shutdown_wake(1);

    Serial.print("writing temp data:");
    Serial.println(network->writeTemperatureData(c, location, room, ts));

    double temp = network->getTemperatureData(location, room);
    double delta = c - temp;
    Serial.print("Set temp: ");
    Serial.print(temp);
    Serial.println("°C\t  ");
    Serial.print("transzfer:  ");
    Serial.println(transferFunction(delta, 4000, 1700, 6500, 0.2));

    if (temp != old_temp || temp != 0) {
      if (delta < -1) {
        Serial.println("Its getting hot");
        digitalWrite(LED, HIGH);
        for (Yeelight* bulb : bulbs) {
          // Serial.println("getIPs ------- : " + bulb->getIP());
          // Serial.println(bulb->setBrightness(30, "smooth", 100));

          Serial.println(bulb->setColorTemp(transferFunction(delta, 4000, 1700, 6500, 0.2), "smooth", bulb_trans_time));
          // Serial.println(bulb->setBrightness(100, "smooth", 100));
        }

        digitalWrite(LED, LOW);


      } else if (delta > 1) {
        Serial.println("Its getting cold");
        digitalWrite(LED, HIGH);
        for (Yeelight* bulb : bulbs) {
          // Serial.println("getIPs ------- : " + bulb->getIP());

          Serial.println(bulb->setColorTemp(transferFunction(delta, 4000, 1700, 6500, 0.2), "smooth", bulb_trans_time));
        }

        digitalWrite(LED, LOW);

      } else {
        Serial.println("everything stays the same");
        digitalWrite(LED, HIGH);
        for (Yeelight* bulb : bulbs) {
          // Serial.println("getIPs ------- : " + bulb->getIP());
          Serial.println(bulb->setColorTemp(transferFunction(delta, 4000, 1700, 6500, 0.2), "smooth", bulb_trans_time));
        }

        digitalWrite(LED, LOW);
      }
      // for (Yeelight* bulb : bulbs) {
      //   // Serial.println("getIPs ------- : " + bulb->getIP());
      //   Serial.println(bulb->setColorTemp(transferFunction(delta, 4000, 1700, 6500, 0.2), "smooth", bulb_trans_time));
      // }

      delay(bulb_trans_time);
      Serial.println("Temperature changeing over");
      for (Yeelight* bulb : bulbs) {
        deserializeJson(jsonBuffer, bulb->sendCommand("get_prop", "[\"hue\", \"sat\", \"rgb\", \"ct\"]"));
        JsonObject root = jsonBuffer.as<JsonObject>();
        int hue = (int)root["result"][0];
        int sat = root["result"][1];
        int rgb = root["result"][2];
        int ct = root["result"][3];
        Serial.print("- hue is: ");
        Serial.println((int)root["result"][0]);
        Serial.print("- satis: ");
        Serial.println((int)root["result"][1]);
        Serial.print("- rgb is:  ");
        Serial.println((int)root["result"][2]);
        Serial.print("- ct is: ");
        Serial.println((int)root["result"][3]);
        network->writeBulbState(
          bulb->getIP(),
          (int)root["result"][0],
          (int)root["result"][1],
          (int)root["result"][2],
          (int)root["result"][3],
          location, room, ts);
      }
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
  int y = (int)(std::abs((max - min) / 2) * tanh(deltaT * speed)) + zero;
  return y;
}

void setupDateTime() {

  DateTime.setServer("hu.pool.ntp.org");
  DateTime.setTimeZone("GMT+1");
  DateTime.begin();
  while (!DateTime.isTimeValid()) {
    DateTime.begin();
    Serial.println("Failed to get time from server.");
  }
  Serial.printf("Date Now is %s\n", DateTime.toISOString().c_str());
  Serial.printf("Timestamp is %ld\n", DateTime.now());
}
