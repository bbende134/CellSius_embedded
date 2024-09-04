#ifndef THERMOCOMM_H
#define THERMOCOMM_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <vector>
#include <cstdint>  // for uint8_t, uint16_t, uint32_t

class ThermoComm {
  public:
  ThermoComm();
  void init();
  void auth();
  void setTemp(double temp);
  void testTemp();

};

#endif
