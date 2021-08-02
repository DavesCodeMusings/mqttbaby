/*
 * i2c_scan -- probe I2C addresses and report on those that respond.
 */

#include <Wire.h>

void setup() {
  Serial.begin(9600);
  Wire.begin();
}

void loop() {
  int result = 0;

  Serial.println();
  Serial.println("Scanning I2C bus...");
  
  for (int addr = 0; addr < 0x7F; addr++) {  // I2C addresses are 7-bits long
    Wire.beginTransmission(addr);     // beginTransmission() returns nothing
    result = Wire.endTransmission();  // endTransmission() returns zero on success or non-zero on error
    if (result == 0) {
      Serial.print("Found device at: 0x");
      Serial.println(addr, HEX);
    }
  }
  delay(10e3);
}
