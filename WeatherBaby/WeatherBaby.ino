#include <Arduino.h>
#include <string.h>
#include <FS.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_BME280.h>
#include <Adafruit_BMP280.h>

#include "ConfigBaby.h"

#define CONFIG_FILE "device.cfg"
#define MQTT_RETAINED true

// ESP8266s need a jumper wire from WAKE to RST, otherwise they'll deep sleep forever. ESP32 doesn't need it. Set to false only for ESP8266 without the jumper wire.
#define WAKE_WIRE true

// Verify the correct I2C address for your hardware. Adafruit uses 0x77, imports tend toward 0x76.
#define I2C_ADDRESS 0x76

ConfigBaby deviceConfig;
WiFiClient wifi;
PubSubClient mqtt(wifi);
Adafruit_BME280 bme;

void wifiConnect() {
  int timeout = 30;
  if (!WiFi.isConnected()) {
    digitalWrite(LED_BUILTIN, LOW);  // LED on (active low) to indicate start of connect attempt.
    WiFi.mode(WIFI_STA);
    Serial.print("Connecting to WiFi network ");
    Serial.print(deviceConfig.read("WiFi SSID"));
    WiFi.begin(deviceConfig.read("WiFi SSID"), deviceConfig.read("WiFi Password"));
    while (!WiFi.isConnected()) {
      Serial.print(".");
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      delay(1000);
      timeout--;
      if (timeout <= 0) {
        Serial.print("Error ");
        Serial.print(WiFi.status());
        Serial.print(": ");
        switch (WiFi.status()) {
          case WL_NO_SSID_AVAIL:
            Serial.println("SSID not found.");
            break;
          case WL_CONNECT_FAILED:
            Serial.println("Incorrect password.");
            break;
          default:
            Serial.println("Unknown error.");
            break;
        }
        Serial.println("Waiting 2 minutes to try again.");
        digitalWrite(LED_BUILTIN, HIGH);
        delay(120000);
        timeout = 30;
      }
    }
    Serial.println(WiFi.localIP());
    digitalWrite(LED_BUILTIN, HIGH);
  }
}

void mqttConnect() {
  if (!mqtt.connected()) {
    Serial.print("Connecting to MQTT broker at ");
    Serial.print(deviceConfig.read("MQTT IP Address"));
    mqtt.setServer(deviceConfig.read("MQTT IP Address"), atoi(deviceConfig.read("MQTT Port")));
    if (!mqtt.connect(deviceConfig.read("Device ID"), deviceConfig.read("MQTT Username"), deviceConfig.read("MQTT Password"))) {
      Serial.print("Error ");
      Serial.print(mqtt.state());
      Serial.print(": ");
      switch (mqtt.state()) {
        case MQTT_CONNECTION_TIMEOUT:
          Serial.println("Connection timeout.");
          break;
        case MQTT_CONNECTION_LOST:
          Serial.println("Connection lost.");
          break;
        case MQTT_CONNECT_FAILED:
          Serial.println("Connection failed.");
          break;
        case MQTT_DISCONNECTED:
          Serial.println("Disconnected.");
          break;
        case MQTT_CONNECT_BAD_PROTOCOL:
          Serial.println("Bad protocol.");
          break;
        case MQTT_CONNECT_BAD_CLIENT_ID:
          Serial.println("Bad client ID.");
          break;
        case MQTT_CONNECT_UNAVAILABLE:
          Serial.println("Server unavailable.");
          break;
        case MQTT_CONNECT_BAD_CREDENTIALS:
          Serial.println("Bad credentials.");
          break;
        case MQTT_CONNECT_UNAUTHORIZED:
          Serial.println("Unauthorized.");
          break;
        default:
          Serial.print("Failed.");
          break;
      }
    }
    Serial.println();  // By this point, the connection is established.
  }
}

void publish(const char* subTopic, const char* message, const bool retained) {
  char topic[64];  // MQTT spec says topics can be longer, but this is enough to accomodate most embedded systems.

  strncpy(topic, deviceConfig.read("Device ID"), 15);
  if (strcmp(subTopic, "") != 0) {  // Skip if there's no subTopic.
    strcat(topic, "/");
    strncat(topic, subTopic, 47);  // What's left from topic[] size after subtracting size of Device ID string and the "/" separator.
  }
  Serial.print("Publishing to ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(message);
  if (!mqtt.publish(topic, message, retained)) {
    Serial.println("FAILED.");
  }
}

void readBME280(int address) {
  if (!bme.begin(address)) {
    Serial.print("No BME280 found at I2C address: 0x");
    Serial.println(address, HEX);
  }
  else {
    char buffer[8];  // Temporary storage for floating point to string conversions.

    // Set up device for "weather station mode" (See data sheet for definition.)
    bme.setSampling(Adafruit_BME280::MODE_FORCED, Adafruit_BME280::SAMPLING_X1, Adafruit_BME280::SAMPLING_X1, Adafruit_BME280::SAMPLING_X1, Adafruit_BME280::FILTER_OFF);
    bme.takeForcedMeasurement();  // Take a reading and then go back to sleep. This mitigates self-heating that causes temperature readings to be too high.
    float temperature = bme.readTemperature();
    float humidity = bme.readHumidity();
    float pressure = bme.readPressure();

    // Readings come the sensor as floating point values and must be converted to strings for publishing.
    // See https://www.nist.gov/pml/weights-and-measures/writing-metric-units for uppercase naming rules.
    snprintf(buffer, 6, "%3.1f", temperature);
    publish("temperature/C", buffer, true);
    snprintf(buffer, 6, "%3.1f", 1.8 * temperature + 32);
    publish("temperature/F", buffer, true);
    snprintf(buffer, 4, "%3.0f", humidity);
    publish("humidity/pct", buffer, true);
    snprintf(buffer, 7, "%6.0f", pressure);
    publish("pressure/Pa", buffer, true);
    snprintf(buffer, 8, "%4.2f", pressure / 100);    
    publish("pressure/mbar", buffer, true);
    snprintf(buffer, 6, "%2.2f", pressure / 3386.39);
    publish("pressure/inHg", buffer, true);
  }
}

void setup() {
  char *fileBuffer = (char *)malloc(MAX_CONFIG_DESERIALIZE);  // Holds config data temporarily for reading from / saving to the flash filesystem.
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);  // LED on during boot.
  Serial.begin(9600);
  Serial.println();
  Serial.println("MQTT Baby (c)2021 David Horton");
  Serial.println("https://github.com/DavesCodeMusings/mqttbaby\n");

  deviceConfig.begin("Device ID,WiFi SSID,WiFi Password,MQTT IP Address,MQTT Port,MQTT Username,MQTT Password,Update Interval", "mqttbaby,Collins,Sussudio,192.168.1.100,1883,admin,password,15");
  
  if (!SPIFFS.begin()) {
    Serial.println("Error mounting flash memory filesystem.");    
  }
  else {
    if (!SPIFFS.exists(CONFIG_FILE)) {  // Present the configuration menu and save the parameters.
      Serial.println("No configuration file found.");
      deviceConfig.input();
      File filePointer = SPIFFS.open(CONFIG_FILE, "w");
      if (!filePointer) {
        Serial.println("Error opening config file for writing.");
      }
      else {
        deviceConfig.serialize(fileBuffer);
        if (filePointer.print(fileBuffer) == 0) {
          Serial.println("Failed to write config file.");
        }
        else {
          filePointer.close();
        }
      }    
    }
    else {  // Read the configuration parameters from the file.
      File filePointer = SPIFFS.open(CONFIG_FILE, "r");
      if (!filePointer) {
        Serial.println("Failed to open the config file for reading.");
      }
      else {
        filePointer.readBytes(fileBuffer, MAX_CONFIG_DESERIALIZE);
        deviceConfig.deserialize(fileBuffer);
        filePointer.close();
      }

      // Similar to PC BIOS prompt.
      Serial.print("Press 1 to configure device.");
      for (int t = 0; t < 10; t++) {
        if (Serial.available() && Serial.read() == '1') {
          deviceConfig.input();
          deviceConfig.serialize(fileBuffer);
          File filePointer = SPIFFS.open(CONFIG_FILE, "w");
          if (!filePointer) {
            Serial.println("Error opening config file for writing.");
          }
          else {
            if (filePointer.print(fileBuffer) == 0) {
              Serial.println("Failed to write config file.");
            }
            else {
              filePointer.close();
            }
          }
          break;
        }
        delay(1000);
        Serial.print(".");
      }
    }
  }
  Serial.println();
  free(fileBuffer);
  digitalWrite(LED_BUILTIN, HIGH);  // LED off to indicate startup is complete.
}

void loop() {
  unsigned long startTime = millis();
  digitalWrite(LED_BUILTIN, LOW);  // A short blink to indicate activity
  delay(10);
  digitalWrite(LED_BUILTIN, HIGH);
  wifiConnect();
  mqttConnect();
  publish("", "ping", false);  // Send a ping message to the device topic (no /subtopic) and flag it as not retained.

  // This is where you would call any functions you create to read and publish sensor data.
  readBME280(I2C_ADDRESS);

  unsigned long duration = millis() - startTime;  // Calculate the length of time (in milliseconds) it took for update and publish.
  if (WAKE_WIRE) {
    Serial.println();  // Makes serial output cleaner when waking up.
    ESP.deepSleep(atoi(deviceConfig.read("Update Interval")) * 60e6 - duration * 1000);  // Update Interval is in minutes, deepSleep expects microseconds.  
  }
  else {
    delay(atoi(deviceConfig.read("Update Interval")) * 60e3 - duration);  // Delay expects milliseconds.
  }
}
