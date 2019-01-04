#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>
#include "HX711.h"

#define WIFI_SSID "home_wg"
#define WIFI_PASSWORD "omisoksbwn"

#define MQTT_HOST IPAddress(192, 168, 0, 7)
#define MQTT_PORT 1883

//Calibration factor for HX711
#define CAL_FACTOR 11280.00 //This value is obtained using the caliberation sketch
#define CONV_FACTOR 0.453592
#define DOUT 13
#define CLK 15
#define RED_L 12
#define YELLOW_L 14

#define _CON_DELAY 20000
#define _THRESHOLD_WEIGHT 20
#define _DETECTION_DELAY 7000

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;

Ticker wifiReconnectTimer;

HX711 scale(DOUT, CLK);

int detection_time = 0;
double lastWeight;
char publishValue[10];

double measured_weight ;
boolean break_weight_loop=true;

IPAddress ip(192, 168, 0, 114); 
IPAddress gateway_dns(192, 168, 0, 1);

void connectToWifi() {
    Serial.println("Connecting to Wi-Fi...");
    WiFi.config(ip, gateway_dns, gateway_dns); 
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
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
    Serial.println("Disconnected from MQTT.");

    if (WiFi.isConnected()) {
        mqttReconnectTimer.once(2, connectToMqtt);
    }
}

void setup() {
    scale.set_scale(CAL_FACTOR);
    scale.tare();
    scale.power_down();

    pinMode(RED_L,OUTPUT);
    pinMode(YELLOW_L,OUTPUT);

    Serial.begin(115200);
    Serial.println();
    Serial.println();

    wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
    wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

    mqttClient.onConnect(onMqttConnect);
    mqttClient.onDisconnect(onMqttDisconnect);
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);

    connectToWifi();

    //Caliberation of HX711
}

void loop() {
    scale.power_up();
    measured_weight = scale.get_units(10) * CONV_FACTOR; //If you want average weight you can pass the num of iterations in get_units()
    if (measured_weight > _THRESHOLD_WEIGHT && ((millis() - detection_time) > _CON_DELAY)) { //Checking millis to avoid repeated publish
//      mqttClient.disconnect();
        break_weight_loop=true;
        Serial.println("Weight detected.");
        digitalWrite(RED_L, HIGH);
        detection_time = millis();
        while (break_weight_loop) {
            delay(40);
            lastWeight = scale.get_units()* CONV_FACTOR;
            if (lastWeight < _THRESHOLD_WEIGHT) {
                break_weight_loop = false;
                digitalWrite(RED_L, LOW);
            }
            else
                measured_weight=lastWeight;
            if (((millis() - detection_time) > _DETECTION_DELAY)) {
                break_weight_loop = false;
                digitalWrite(RED_L, LOW);
                digitalWrite(YELLOW_L, HIGH);
            }
        }
        sprintf(publishValue, "%f", measured_weight);
        yield();
        mqttClient.publish("home/bathroom/scale/weight", 1, true, publishValue);
    }
    scale.power_down();
    delay(2000);
    analogWrite(YELLOW_L, 50);
    if(((millis() - detection_time) > _CON_DELAY)) {
        delay(500);
        digitalWrite(YELLOW_L, LOW);
    }

}

