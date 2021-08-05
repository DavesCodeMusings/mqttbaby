#include "ConfigBaby.h"

ConfigBaby deviceConfig;
  
void setup() {
  Serial.begin(9600);
  Serial.println();
  delay(5000);
  deviceConfig.begin("IP Address,Subnet Mask,Gateway");
  Serial.print("Number of key-value pairs created: ");
  Serial.println(deviceConfig.numPairs);
  deviceConfig.write("IP Address", "192.168.1.10");
  deviceConfig.write("Subnet Mask", "255.255.255.0");
  deviceConfig.write("Gateway", "192.168.1.1");
  
  Serial.println();
  Serial.print("The 'IP Address' key is at index ");
  Serial.println(deviceConfig.indexOf("IP Address"));
  Serial.print("The value for 'IP Address' is: ");
  Serial.println(deviceConfig.read("IP Address"));
  
  Serial.print("The 'Subnet Mask' key is at index ");
  Serial.println(deviceConfig.indexOf("Subnet Mask"));
  Serial.print("The value for 'Subnet Mask' is: ");
  Serial.println(deviceConfig.read("Subnet Mask"));
    
  Serial.print("The 'Gateway' key is at index ");
  Serial.println(deviceConfig.indexOf("Gateway"));
  Serial.print("The value for 'Gateway' is: ");
  Serial.println(deviceConfig.read("Gateway"));

  Serial.print("The 'Foo' key is at index ");
  Serial.println(deviceConfig.indexOf("Foo"));
  Serial.print("The value for 'Foo' is: ");
  Serial.println(deviceConfig.read("Foo"));
  

}

void loop() {
  deviceConfig.input();
  delay(5000);
}
