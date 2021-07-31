# MQTT Baby
This project centers around an Arduino IDE sketch for the ESP8266 microcontroller. It allows the board to connect to WiFi and publish messages to an MQTT server on the local network. It also includes a serial monitor-based setup utility to allow WiFi and MQTT credentials, along with other parameters, to be stored in non-volitile memory. Writing the sketch to an 8266 microcontroller will create a system than sends a periodic ping to an MQTT server. Publishing data from attached sensors should simply be a matter of adding a function to the loop.

## How to use it
You'll first want to be familiar with using the Arduino IDE for ESP2866 devices. There are several internet tutorials on this topic along with the [official project](https://github.com/esp8266/Arduino) to get you going. You'll also need an MQTT server and a home automation system to integrate with. My [CloudPi](https://github.com/DavesCodeMusings/CloudPi) project may be helpful if you don't have MQTT or home automation installed yet.

Once you have the prerequisites out of the way, here are the steps to get started.
1. Use the Arduino IDE to flash the mqttbaby.ino sketch.
2. Set up PuTTY or another serial program to attach to your ESP8266's COM port at 9600bps, 8 bits, one stop bit, no parity.
3. Turn on Local Echo for the serial connection, otherwise you won't see what you're typing.
4. Reset the 8266.
5. On first boot, the device will enter the setup menu. Enter the parameters for your network.
6. Save the parameters and verify the device can find your WiFi and MQTT server.
7. Further verify by using Home Assistant's MQTT integration utility to listen to the MQTT topic being sent. (The 8266 serial debug output will show the topic and message.)

If you can do that, your MQTT Baby is ready to customize. Add any sensor reading functions you want and repeat the steps above to verify the data is being sent to MQTT.

## Why?
I wanted a way to collect outdoor temperature and humidity data for my Home Assistant home automation installation. There's a cute little Xaiomi Mijia device that does this very nicely when paired with ESPHome, but it's really an indoor device. It's not weather resistant and the temperature range only goes down to 0 Celsius. The popular DHT22 temperature / humidity sensor and ESP8266 can withstand -40C.

Generic 8266 microcontrollers, like NodeMCU and D1 clones, are incredibly cheap and well documented when it comes to interfacing with environmental sensors.  I also wanted something more DIY than ESPHome, but more polished than a custom sketch. So I created MQTT Baby.

## What does it do?
A lot of the work that went into MQTT Baby was to create a semi-friendly menu-based setup utility for gathering network parameters and saving them to non-volitile memory. Restarting after flashing the sketch will enter the configuration utility. The utility is also accessible by pressing a key within the first 10 seconds of start-up.

Once network parameters are entered, the device go into a loop to do the following tasks:
1. Check the WiFi connection and re-attach if needed.
2. Check the MQTT session and re-connect if needed.
3. Publish a simple ping message to MQTT with the topic of device_name/system; where device_name is assigned in setup.

## So what?
Inside the loop are a couple of commented lines where you can add code to read sensors and publish the readings to MQTT, similar to the way the ping message is sent. Find an example sketch for the sensor you want to use. Write a function to reads that sensor's data. Call the function in the loop() part of the MQTT Baby sketch. Use the included publish() function to send the readings to MQTT.

## What else is it good for?
Tinkering with microcontrollers can be a great learning experience. Things you can pick up from examining this code are:
* How to use 8266 non-volitile memory to store parameters.
* How to use the serial connection for more than just debug output.
* How to read and save binary data to files.

## Next Steps
My focus for future enhancements is to create sample functions for reading DHT22 temperature/humidity data and BMP280 barometric pressure. Deep sleep capability for battery-powered devices is also something I'm considering. And, if I can figure out how to echo characters when using Serial.readStringUntil(), I will add that feature.

I probably will not be adding any capability for receiving MQTT messages, outputting to a display, JSON / YAML config files, or over the air (OTA) updates.
