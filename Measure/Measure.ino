#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>
#include "HX711.h"

#define WIFI_SSID "home_wg"
#define WIFI_PASSWORD "omisoksbwn"

#define MQTT_HOST IPAddress(192, 168, 0, 7)
#define MQTT_PORT 1883

//Calibration factor for HX711
#define calibration_factor 11280.00 //This value is obtained using the caliberation sketch

#define DOUT 13
#define CLK 15
#define RED_L 12
#define YELLOW_L 14

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

HX711 scale(DOUT, CLK);

int detection_time = 0;
double lastWeight;
char publishValue[10];


void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Connected to Wi-Fi.");
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi.");
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(2, connectToWifi);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
  uint16_t packetIdSub = mqttClient.subscribe("test/lol", 2);
  Serial.print("Subscribing at QoS 2, packetId: ");
  Serial.println(packetIdSub);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  Serial.println(total);
}

void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void setup() {
  scale.set_scale(calibration_factor);
  scale.tare();
  scale.power_down();
  
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  connectToWifi();
  
  //Caliberation of HX711
}

void loop() {
  scale.power_up();
  double measured_weight = scale.get_units(10) * 0.453592; //If you want average weight you can pass the num of iterations in get_units()
  if (measured_weight > 20) {
//      mqttClient.disconnect();
      boolean break_weight_loop = true;
      Serial.println("Weight detected.");
      digitalWrite(RED_L, HIGH);
      delay(500);
      digitalWrite(RED_L, LOW);
      detection_time = millis();
      while (break_weight_loop) {
        delay(10);
        lastWeight = scale.get_units(10)* 0.453592;
        if (lastWeight < 20) {
          break_weight_loop = false;
          digitalWrite(RED_L, HIGH);
//        //PUSH Measured Weight
          lastWeight = lastWeight + 2;
          sprintf(publishValue, "%f", measured_weight);
          yield();
          mqttClient.publish("home/bathroom/scale/weight", 1, true, publishValue);
       }
       else
        measured_weight=lastWeight;
        if (((millis() - detection_time) > 15000)) {
          break_weight_loop = false;
          digitalWrite(YELLOW_L, HIGH);
          //PUSH Measured Weight
          sprintf(publishValue, "%f", lastWeight);
          yield();
          mqttClient.publish("home/bathroom/scale/weight", 1, true, publishValue);
        }
      }
  }
  scale.power_down();
  delay(2000);
}

