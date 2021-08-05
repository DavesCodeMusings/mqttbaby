# ConfigBaby

## What is it?

ConfigBaby is a class used to store a short list of configuration parameters in a key-value style associative array. It features an interactive setup menu that can be invoked to gather parameter values when a terminal emulator like Putty is attached to the COM port of the device. (The built-in Arduino IDE Serial Monitor does not work properly.)

ConfigBaby is not an all encompassing associative array solution. It is limited to only 9 total key-value pairs. The intent is to allow configuration of a few parameters for network configuration and authentication.

Speaking of authentication, all of the parameters are stored in plain text. Anyone with physical access to you device could retreive them. You have been warned.

Future enhancements will allow writing the key-value pairs to a file for persistent storage.

## Why should you care?

If you want to create a sketch that allows configuration of certain parameters in the field, rather than having them compiled in as #define macros, ConfigBaby might be useful for your project. For example, you may want to program a device in the lab and then let the end user set it up with networking parameters applicable to the site.

## Sample device configuration using interactive menu method ConfigBaby::input() and PuTTY terminal emulator.

```
Device Configuration Menu
(1) IP Address        192.168.1.10
(2) Subnet Mask       255.255.255.0
(3) Gateway           192.168.1.1
(0) Save and exit.
Parameter? 1
Enter new value for IP Address: 172.16.1.100

(1) IP Address        172.16.1.100
(2) Subnet Mask       255.255.255.0
(3) Gateway           192.168.1.1
(0) Save and exit.
Parameter? 2
Enter new value for Subnet Mask: 255.255.0.0

(1) IP Address        172.16.1.100
(2) Subnet Mask       255.255.0.0
(3) Gateway           192.168.1.1
(0) Save and exit.
Parameter? 3
Enter new value for Gateway: 172.16.1.1

(1) IP Address        172.16.1.100
(2) Subnet Mask       255.255.0.0
(3) Gateway           172.16.1.1
(0) Save and exit.
Parameter?
```

## Where is all this going?

I created ConfigBaby to use in my MQTTBaby project. (That might help explain the name.) MQTTBaby is a sketch that can be flashed to an ESP8266 to allow polling sensors and publishing data to MQTT topics. Originally, I was gathering parameters using hardcoded parameter names and a C struct to hold the values. ConfigBaby adds more flexibility.

My plan is to add serialize() and deserialize() methods to ConfigBaby so that the parameters can be stored to flash memory with SPIFFS or using a micro SD card.
