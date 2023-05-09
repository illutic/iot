#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Custom libraries
#include "WiFiConfig.h"

using namespace WiFiConfig;

bool pumping = false;
WiFiClient espClient;
PubSubClient mqttClient(espClient);

void setupPins() {
  pinMode(P_READ_MOISTURE, INPUT);
  pinMode(P_PUMP_ON, OUTPUT);
}

void turnPumpOn() {
  if (!pumping) {
    Serial.println("Pumping...");

    pumping = true;

    digitalWrite(P_PUMP_ON, HIGH);
    delay(PUMP_TIMEOUT);
    digitalWrite(P_PUMP_ON, LOW);

    pumping = false;
    mqttClient.publish(MQTT_SUBJ, "Pumping complete");
  }
}

void onMessageReceived(char *topic, byte *payload, unsigned int length) {
  const char *pump = "pump";
  char message[length + 1];

  // Copy payload into message array
  for (int i = 0; i < length; i++)
  {
    message[i] = (char)payload[i];
  }

  // Null-terminate the message array
  message[length] = '\0';

  Serial.println("Received message: ");
  Serial.println(message);

  // Search for "pump" in the message
  if (strstr(message, pump) != nullptr)
  {
    turnPumpOn();
  }
}

void connectToWifi() {
  Serial.println();

  while (!WiFi.isConnected()) {
    Serial.print(".");
    WiFi.begin(SSID, PASSWORD);
    WiFi.waitForConnectResult();
  }

  Serial.println();
}

void connectToMQTTBroker() {
  Serial.println();

  mqttClient.setServer("192.168.147.86", MQTT_PORT);
  mqttClient.setCallback(onMessageReceived);
  Serial.println();
}

void reconnectMqttClient() {
  while (!mqttClient.connected()) {
    Serial.println("Attempting MQTT connection");

    if (mqttClient.connect("WeatherHub", MQTT_USER, MQTT_PASS)) {
      Serial.println("MQTT client connected");

      mqttClient.subscribe(MQTT_SUBJ);
    } else {
      Serial.println("Failed,");
      Serial.print(mqttClient.state());
      Serial.println(" attempting to reconnect in 5 seconds");

      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(BAUD_RATE);
  connectToWifi();
  connectToMQTTBroker();
  setupPins();
}

void loop() {
  if (!mqttClient.connected()) {
    reconnectMqttClient();
  }

  mqttClient.loop();

  if (digitalRead(P_READ_MOISTURE) == HIGH) {
    mqttClient.publish(MQTT_SUBJ, "pump");
  }

  delay(10000);
}