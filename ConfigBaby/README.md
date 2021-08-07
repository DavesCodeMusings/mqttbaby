# ConfigBaby

## What is it?

ConfigBaby is a class used to store a short list of configuration parameters in a key-value style associative array. It features an interactive setup menu that can be invoked to gather parameter values when a terminal emulator like Putty is attached to the COM port of the device. (The built-in Arduino IDE Serial Monitor does not work properly.)

ConfigBaby is not an all encompassing associative array solution. It is limited to only 9 total key-value pairs. The intent is to allow configuration of a few parameters for network configuration and authentication.

Speaking of authentication, all of the parameters are stored in plain text. Anyone with physical access to you device could retreive them. You have been warned.

Future enhancements will allow writing the key-value pairs to text for persistent storage.

## Why should you care?

If you want to create a sketch that allows certain parameters to be configured in the field, rather than having them compiled in as #define macros, ConfigBaby might be useful for your project. For example, you may want to program a device in the lab and then let the end user set it up with networking parameters applicable to the site.

## Sample device configuration using interactive menu method ConfigBaby::input() and PuTTY terminal emulator.

```
Device Configuration Menu
(1) WiFi SSID         collins
(2) WiFi Password     sussudio
(3) MQTT IP Address   192.168.1.18
(4) MQTT Port         1883
(5) MQTT Username     admin
(6) MQTT Password     abc123
(0) Save and exit.
Parameter? 5
Enter new value for MQTT Username: Contoso

(1) WiFi SSID         collins
(2) WiFi Password     sussudio
(3) MQTT IP Address   192.168.1.18
(4) MQTT Port         1883
(5) MQTT Username     Contoso
(6) MQTT Password     abc123
(0) Save and exit.
Parameter? 6
Enter new value for MQTT Password: P@ssw0rd
```

Pretty neat, huh?

## How can you get started using it?

Take a look at the ConfigBaby.ino sketch for examples of creating a config, let the user interact with it through the menu, and finally serialize the data for persistent storage.

For a deeper look, the ConfigBaby.h file has comments describing with the methods do and what parameters they expect. Currently, all of the variables and methods are public, though using some of them are intended for internal use. These include writeValue(), indexOf(), and readln(). Using writeValue() and indexOf() in the main sketch would defeat the idea of the key-value concept. And readln() only exists because Serial.readUntil() does not echo characters as they are typed.

## Where is all this going?

I created ConfigBaby to use in my MQTTBaby project. (That might help explain the name.) MQTTBaby is a sketch that can be flashed to an ESP8266 to allow polling sensors and publishing data to MQTT topics. Originally, I was gathering parameters using hardcoded parameter names and a C struct to hold the values. ConfigBaby adds more flexibility.

My plan is to add a deserialize() method to so the parameters can be retrieved from persistent storage and put back into the configuration. I'm also looking at ways to make the internal storage of key-value pairs more space efficient.
