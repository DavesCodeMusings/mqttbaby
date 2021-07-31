/* mqttbaby 
 * For ESP8266, NodeMCU, D1, and clones. Publish data to MQTT topics.
 */
#include <FS.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define CONFIG_FILE "device.cfg"
#define UPDATE_INTERVAL 120000
#define SERIAL_BPS 9600
#define WIFI_TIMEOUT 30

typedef struct {
  char deviceID[16];
  char wifiSSID[32];
  char wifiPass[32];
  char mqttIP[16];
  unsigned int mqttPort;
  char mqttUser[32];
  char mqttPass[32];
} DeviceConfig;

DeviceConfig config;
WiFiClient wifi;
PubSubClient mqtt(wifi);

/**
 * Read device configuration parameters from non-volitile memory filesystem.
 */
int readConfig() {
  int success = 0;
  File configFile = SPIFFS.open(CONFIG_FILE, "r");
  if (configFile) {
    if (configFile.readBytes((char *) &config, sizeof config) == sizeof config) {
      success = 1;
      configFile.close();
    }
    else {
      Serial.println("Error reading configuration.");
    }
  }
  else {
    Serial.println("Error opening configuration for reading.");
  }

  return success;
}

/**
 * Write device configuration structure to non-volitile memory filesystem.
 */
int writeConfig() {
  int success = 0;
  File configFile = SPIFFS.open(CONFIG_FILE, "w");
  if (configFile) {
    if (configFile.write((char *) &config, sizeof config) == sizeof config) {
      success = 1;
      configFile.close();
      Serial.println("Configuration saved.");
    }
    else {
      Serial.println("Error writing config.");
    }
  }
  else {
    Serial.println("Error opening config for writing.");
  }

  return success;
}

/**
 * Reset configuration values to defaults.
 */
void setConfigDefaults() {
  strcpy(config.deviceID, "mqttbaby");
  strcpy(config.wifiSSID, "ssid");
  strcpy(config.wifiPass, "password");
  strcpy(config.mqttIP, "192.168.1.1");
  config.mqttPort = 1883;
  strcpy(config.mqttUser, "mqtt");
  strcpy(config.mqttPass, "password");
}

/**
 * Interactively gather device configuration parameters from the serial connection.
 */
void inputConfig() {
  Serial.println();
  Serial.println("Device Setup");
  boolean done = false;

  while (!done) {
    //Show current values.
    Serial.print("(1) Device ID:        ");
    Serial.println(config.deviceID);
    Serial.print("(2) WiFi SSID:        ");
    Serial.println(config.wifiSSID);
    Serial.print("(3) WiFi password:    ");
    Serial.println(config.wifiPass);
    Serial.print("(4) MQTT IP Address:  ");
    Serial.println(config.mqttIP);
    Serial.print("(5) MQTT IP port:     ");
    Serial.println(config.mqttPort);
    Serial.print("(6) MQTT IP user:     ");
    Serial.println(config.mqttUser);
    Serial.print("(7) MQTT IP password: ");
    Serial.println(config.mqttPass);
    Serial.println("(0) Save and exit.");

    // Prompt for changes.
    Serial.print("Parameter? ");
    while (!Serial.available());
    int choice = Serial.read() - '0';  // Convert ASCII character to a number by subracting ASCII value of zero.
    Serial.println(choice);

    if (choice == 0) {
      writeConfig();
      done = true;
    }
    else {
      Serial.print("New value: ");
      Serial.setTimeout(60000); // 60 seconds expressed in milliseconds.
      String value = Serial.readStringUntil('\r');
      Serial.println("\n");

      // Validate and store in the appropriate parameter.
      switch (choice) {
        case 1:
          if (value.length() < sizeof config.deviceID) {
            strcpy(config.deviceID, value.c_str());
          }
          else {
            Serial.println("Input too long.");
          }
          break;
        case 2:
          if (value.length() < sizeof config.wifiSSID) {
            strcpy(config.wifiSSID, value.c_str());
          }
          else {
            Serial.println("Input too long.");
          }
          break;
        case 3:
          if (value.length() < sizeof config.wifiPass) {
            strcpy(config.wifiPass, value.c_str());
          }
          else {
            Serial.println("Input too long.");
          }
          break;
        case 4:
          if (value.length() < sizeof config.mqttIP) {
            strcpy(config.mqttIP, value.c_str());
          }
          else {
            Serial.println("Input too long.");
          }
          break;
        case 5:
          if (value.toInt() > 0 && value.toInt() < 65536) {
            config.mqttPort = value.toInt();
          }
          else {
            Serial.println("Input out of range.");
          }
          break;
        case 6:
          if (value.length() < sizeof config.mqttUser) {
            strcpy(config.mqttUser, value.c_str());
          }
          else {
            Serial.println("Input too long.");            
          }
          break;
        case 7:
          if (value.length() < sizeof config.mqttPass) {
            strcpy(config.mqttPass, value.c_str());
          }
          else {
            Serial.println("Input too long.");            
          }
          break;
      }
    }
  }
}

/**
 * Connect to WiFi network unless there's already an established connection.
 * If a connection cannot be made, pause for a while and try again.
 */
void wifiConnect() {
  if (!WiFi.isConnected()) {
    digitalWrite(LED_BUILTIN, LOW);  // LED on (active low) to indicate startup.
    Serial.print("Connecting to WiFi network ");
    Serial.print(config.wifiSSID);
    Serial.print("...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(config.wifiSSID, config.wifiPass);
    
    // Flash LED at 50% duty cycle while waiting for WiFi connection.
    int timeout = WIFI_TIMEOUT;
    while (!WiFi.isConnected()) {
      delay(1000);
      Serial.print(".");
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      timeout--;
      if (timeout <= 0) {
        Serial.println();
        switch (WiFi.status()) {
          case WL_NO_SSID_AVAIL:
            Serial.println("SSID not found.");
            break;
          case WL_CONNECT_FAILED:
            Serial.println("Incorrect password.");
            break;
          default:
            Serial.println("Failed.");
            break;
        }
        Serial.println("Waiting 2 minutes to try again.");
        digitalWrite(LED_BUILTIN, LOW);  // Leaving LED on indicates there is a problem.
        delay(120000);
        timeout = WIFI_TIMEOUT;
      }
    }

    // Turn off LED to indicate sucessful connection.
    digitalWrite(LED_BUILTIN, HIGH);  // LED off on success.
    Serial.println("OK");
    Serial.print("Obtained IP address: ");
    Serial.println(WiFi.localIP());   
  }
}

/**
 * Log into the MQTT server unless there's already a session established.
 */
void mqttConnect() {
  if (!mqtt.connected()) {
    Serial.print("Connecting to MQTT server at ");
    Serial.println(config.mqttIP);
    mqtt.setServer(config.mqttIP, config.mqttPort);
    if (!mqtt.connect(config.deviceID, config.mqttUser, config.mqttPass)) {
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
  }
}

/**
 * Publish a MQTT message to config.deviceID/subTopic and report failures. 
 */
void publish(char* subTopic, char* message) {
  char *topic = (char *) malloc(sizeof config.deviceID + sizeof subTopic + 2);  // +1 for / and +1 for null terminator.
  strcpy(topic, config.deviceID);
  strcat(topic, "/");
  strcat(topic, subTopic);
  Serial.print("Publishing to ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(message);
  if (!mqtt.publish(topic, message)) {
    Serial.println("FAILED.");
  }
  free (topic);
}

/**
 * Start the WiFi and MQTT connections. Publish a startup message to MQTT.
 */
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(SERIAL_BPS);
  Serial.println();

  // Normal startup if device config exists. Otherwise, prompt for parameters.
  if (SPIFFS.begin()) {
    if (SPIFFS.exists(CONFIG_FILE)) {
      Serial.print("Press any key to enter setup.");
      for (int i=0; i<10; i++) {
        if (Serial.available()) {
          while(Serial.available()) {
            Serial.read();  // Clear the keypress(es).
          }
          Serial.println();
          readConfig();
          inputConfig();
          break;
        }
        Serial.print(".");
        delay(1000);
      }
      Serial.println();
    }
    else {
      Serial.print("No configuration found.");
      setConfigDefaults();
      inputConfig();
    }  
  }
  else {
    Serial.println("Failed to mount flash filesystem.");
  }
}

/**
 * Update sensor data and publish to MQTT.
 */
void loop() {
  digitalWrite(LED_BUILTIN, LOW);  // Indicates activity.
  SPIFFS.begin();
  readConfig();
  // TO DO: Take readings from sensors.
  wifiConnect();
  mqttConnect();
  publish("system", "Ping.");
  // TO DO: Publish sensor readings. 
  digitalWrite(LED_BUILTIN, HIGH);
  delay(UPDATE_INTERVAL);
}
