/*
 * Reformat the SPIFFS non-volitile filesystem of an ESP-8266 board.
 * The built-in LED lights when it starts and goes out when done.
 */

#include <FS.h> //spiff file system

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(9600);
  Serial.println();
  Serial.print("Formatting... ");
  SPIFFS.begin();
  SPIFFS.format();
  Serial.println("Done.");
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
}
