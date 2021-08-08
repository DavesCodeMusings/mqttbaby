#include "ConfigBaby.h"

ConfigBaby deviceConfig;
char fileBuffer[256];

void setup() {
  Serial.begin(9600);
  Serial.println();
  delay(1000);
  
  // ConfigBaby::begin lets you initialize the key-value pairs.

  // It can be called with just keys and no values. And it's okay to have spaces in keys and values.
  Serial.println("Initializing keys only.");
  deviceConfig.begin("IP Address,Subnet Mask,Gateway");
  deviceConfig.serialize(fileBuffer);
  Serial.println(fileBuffer);
  
  // It can be called with a full list of keys and a full list of default values.
  Serial.println("Initializing with all keys and and all values."); 
  deviceConfig.begin("IP Address,Subnet Mask,Gateway", "192.168.1.99,255.255.255.0,192.168.1.1");
  deviceConfig.serialize(fileBuffer);
  Serial.println(fileBuffer);

  // It can take a list of default values along with the keys. It does not have to be a complete list.
  Serial.println("Initializing with keys and a partial list of values.");
  deviceConfig.begin("Red,Green,Blue", "Rojo,Verde");
  deviceConfig.serialize(fileBuffer);
  Serial.println(fileBuffer);

  // But skipping a value in the middle will not work, because of the way the underlying strtok() function handles multiple consecutive delimiters.
  Serial.println("Skipping a value in the middle doesn't work right. Don't do it."); 
  deviceConfig.begin("Red,Green,Blue", "Rojo,,Azul");
  deviceConfig.serialize(fileBuffer);
  Serial.println(fileBuffer);
  
  // If the key names or values are too long, they get truncated to avoid overflows.
  Serial.println("Initializing with a key and a value that are too long truncates."); 
  deviceConfig.begin("A key that is too long.,Vogon Poetry", "A value that is too long.,Ode to a Small Lump of Green Putty");
  deviceConfig.serialize(fileBuffer);
  Serial.println(fileBuffer);

  // A more typical initialization, is with both keys and default values specified.
  Serial.println("Normally, a list of default values is passed with the keys.");
  deviceConfig.begin("Early,On Time,Late", "On Time,Late,Deep Shame");
  deviceConfig.serialize(fileBuffer);
  Serial.println(fileBuffer);

  // Key-value pairs can be saved to a file and then retrieved to be load back in at start up.
  // Formatting is strict with one space on either side of the = so best to use deserialize and avoid hand-formatting the config.
  Serial.println("Inputting values from the contents of a file is also available.");

  // This hand-formatted string is for example only. Normally, you would read from a file. Formatting is very strict. Having NO SPACES surrounding the equal sign is critcal.
  char fileContents[] = "WiFi SSID=collins\r\nWiFi Password=sussudio\r\nMQTT IP Address=192.168.1.18\r\nMQTT Port=1883\r\nMQTT Username=admin\r\nMQTT Password=abc123\r\n";
  deviceConfig.deserialize(fileContents);
  deviceConfig.serialize(fileBuffer);
  Serial.println(fileBuffer);
    
  // You can also use an interactive menu to enter values.
  Serial.println("Users can also input and update values using a menu...");
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
