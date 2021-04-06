/*
 *  File:        check_i2c.cpp
 *  Last edited: 05.04.2021
 *  GitHub:
 * 
 *  Copyright 2021 Tauno Erik
 *  https://taunoerik.art/
 * 
 */
#include <Arduino.h>
#include <Wire.h>

/**
 * Returns true if I2C device found
 */
bool checkI2C() {
  Wire.begin();
  byte error, address;
  int nDevices;
  Serial.println("I2C bus Scanning...");
  nDevices = 0;
  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("\nI2C device found at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
      nDevices++;
    } else if (error == 4) {
      Serial.print("\nUnknow error at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
    return false;
  } else {
    Serial.println("done\n");
    return true;
  }
}