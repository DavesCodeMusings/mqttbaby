#include "ConfigBaby.h"

ConfigBaby deviceConfig;
char fileBuffer[256];

void setup() {
  Serial.begin(9600);
  Serial.println();
  delay(1000);
/*
  // ConfigBaby::begin lets you initialize the key-value pairs.

  // It can be called with just keys and no values.
  Serial.println("\nInitializing keys only.");
  deviceConfig.begin("IP Address,Subnet Mask,Gateway");
  deviceConfig.serialize();

  // It can be called with a full list of keys and a full list of default values.
  Serial.println("\nInitializing with all keys and and all values."); 
  deviceConfig.begin("IP Address,Subnet Mask,Gateway", "192.168.1.99,255.255.255.0,192.168.1.1");
  deviceConfig.serialize(fileBuffer);

  // If the key names or values are too long, they are truncated.
  Serial.println("\nInitializing with a key and a value that are too long."); 
  deviceConfig.begin("IP Address,Subnet Mask,Gateway,A key that is too long", "192.168.1.99,255.255.255.0,192.168.1.1,A value that is too long");
  deviceConfig.serialize(fileBuffer);

  // It can take a list of default values along with the keys. It does not have to be a complete list.
  Serial.println("\nInitializing with keys and a partial list of values.");  
  deviceConfig.begin("IP Address,Subnet Mask,Gateway", "192.168.1.99,255.255.255.0");
  deviceConfig.serialize();

  // But skipping a value in the middle will not work, because of the way the underlying strtok() function handles multiple consecutive delimiters.
  Serial.println("\nInitializing with all keys and and skipping a value in the middle doesn't work right."); 
  deviceConfig.begin("IP Address,Subnet Mask,Gateway", "192.168.1.99,,192.168.1.1");
  deviceConfig.serialize(fileBuffer);
*/

  deviceConfig.begin("WiFi SSID,WiFi Password,MQTT IP Address,MQTT Port,MQTT Username,MQTT Password", "collins,sussudio,192.168.1.18,1883,admin,abc123");
  
  // You can also use an interactive menu to enter values.
  Serial.println("\nInteractive mode...");
}

void loop() {
  int fileSize = 0;
  
  deviceConfig.input();
  fileSize = deviceConfig.serialize(fileBuffer);
  Serial.println("\nData ready to save:");
  Serial.println(fileBuffer);
  Serial.print("File size needed: ");
  Serial.println(fileSize);
  delay(2000);
}
