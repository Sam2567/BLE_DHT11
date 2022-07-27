#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "DHT.h"
#define DHTPIN 19
#define DHTTYPE DHT11

int16_t dhtRead_t = 0;
int16_t dhtRead_h = 0;
