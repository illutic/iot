#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "WiFiConfig.h"

using namespace WiFiConfig;

bool pumping = false;
WiFiClient espClient;
PubSubClient mqttClient(espClient);

void setupPins()
{
    pinMode(P_READ_MOISTURE, INPUT);
    pinMode(P_PUMP_ON, OUTPUT);
}

void turnPumpOn()
{
    if (!pumping)
    {
    Serial.println("Pumping...");

        pumping = true;

        digitalWrite(P_PUMP_ON, HIGH);
        delay(PUMP_TIMEOUT);
        digitalWrite(P_PUMP_ON, LOW);

        pumping = false;
    }
}

void onMessageReceived(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message received [");
    Serial.print(topic);
    Serial.println("]");

    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }

    Serial.println();
}

void connectToWifi()
{
    Serial.println();

    while (!WiFi.isConnected())
    {
        Serial.print(".");
        WiFi.begin(SSID, PASSWORD);
        WiFi.waitForConnectResult();
    }

    Serial.println();
}

void connectToMQTTBroker()
{
    Serial.println();

    mqttClient.setServer(WiFi.gatewayIP(), MQTT_PORT);
    mqttClient.setCallback(onMessageReceived);
    Serial.println();
}

void reconnectMqttClient()
{
    while (!mqttClient.connected())
    {
        Serial.print("Attempting MQTT connection");

        if (mqttClient.connect("WeatherHub", MQTT_USER, MQTT_PASS))
        {
            Serial.println("MQTT client connected");

            mqttClient.subscribe(MQTT_SUBJ);
        }
        else
        {
            Serial.print("Failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" attempting to reconnect in 5 seconds");

            delay(5000);
        }
    }
}

void setup()
{
    Serial.begin(BAUD_RATE);
    connectToWifi();
    connectToMQTTBroker();
    setupPins();
}

void loop()
{
    if (!mqttClient.connected())
    {
        reconnectMqttClient();
    }
    mqttClient.loop();
    Serial.println(digitalRead(P_READ_MOISTURE));
    delay(5000);
}