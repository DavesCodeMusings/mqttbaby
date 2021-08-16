# WeatherBaby

WeatherBaby = MQTTBaby + BME280 sensor. That's the simplest explanation I can come up with.

[MQTTBaby](https://github.com/DavesCodeMusings/mqttbaby) forms the base of this project. I've added a function to to read temperature, humidity, and barometric pressure data from an I2C bus connected BME280 sensor. That data is sent to an MQTT broker via a call to MQTTBaby's `publish()` function. The sensor reading function is called from within the main loop, so fresh readings are sent periodically.

## How Can I Use It?

As it stands, all you need is an ESP8266 wired to an I2C attached BME280 sensor, and you've got yourself a weather station. Mine is currently sitting on a shelf publishing readings to Home Assistant via the MQTT integration. Ultimately, I'm going to mount it outside, housed in a length of PVC pipe for weather sheilding. You can build something similar or simply use the code as an exmple of how to use [MQTTBaby](https://github.com/DavesCodeMusings/mqttbaby) and [ConfigBaby](https://github.com/DavesCodeMusings/mqttbaby/ConfigBaby) in a project of your own design.

## Where's This Project Going?

Nowhere. I have no desire to write a library of functions for every available sensor on every available Auduino IDE compatible microcontroller. If you've got an ESP with a BME280, great. Otherwise, feel free to use this project as a base to add your own sensor reading function. It's no more difficult than taking an example sketch from the sensor library and adapting it to send readings to the MQTTBaby `publish()` function.
