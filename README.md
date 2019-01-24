## Title

IoT Weighing Scale

## About

A Smart Bathroom weighing scale using a generic bathromm scale and WeMOS.The Wemos feteches data from the load sensors of the weighing scale using HX711 load cell amplifier. 

### Libraries Used

* [AsyncMQTTClient](https://github.com/marvinroger/async-mqtt-client)
* [HX711](https://github.com/bogde/HX711)

### Video and Blog
Since I got into Home Automation System, I have always wanted a central system with as much as information about me starting from my food habit to each and everything that can be measured or collected. You can say that it is greatly influenced by JARVIS of Iron Man which came around my Engineering days. Coming to the topic, I recently managed to connect a regular bathroom weighing scale to my Home Automation System (OpenHAB) using MQTT. So basically this post is all about how to do that.

For the weighing scale, I went with a cheap bathroom scale that comes around rupees 600 and readily available in Amazon or Flipkart in India. You might think why not to use a smart scale already available like the MI Scale, that also features a lot of other parameters like body fat etc. It is due to issues like, not all the scale comes with open API (I have not checked all scales though..), and I wanted full control........ [Read More](https://www.weargenius.in/hx711-and-esp8266/)
