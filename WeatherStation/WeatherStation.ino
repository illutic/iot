#include <DHT_U.h>
#include <DHT.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Custom libraries
#include "WiFiConfig.h"

using namespace WiFiConfig;

#define DHT_TYPE DHT11
bool pumping = false;

DHT dht(P_READ_TEMP, DHT_TYPE);
WiFiClient espClient;
PubSubClient mqttClient(espClient);

void setupPins() {
  pinMode(P_READ_MOISTURE, INPUT);
  pinMode(P_READ_TEMP, INPUT);
  pinMode(P_READ_LIGHT, INPUT);
  pinMode(P_PUMP_ON, OUTPUT);

  digitalWrite(P_PUMP_ON, LOW);
}

// ============ JSON Utils ============

// Converts the sensor data provided to a json string for transport
String convertSensorDataToJsonString(int lightLevel, float temperature, float humidity, bool isMoist) {
  // Create a JSON object
  StaticJsonDocument<200> jsonDoc;

  // Assign values to the JSON object
  jsonDoc["lightLevel"] = lightLevel;
  jsonDoc["temperature"] = temperature;
  jsonDoc["humidity"] = humidity;
  jsonDoc["isMoist"] = isMoist;

  // Serialize the JSON object to a string
  String jsonString;
  serializeJson(jsonDoc, jsonString);

  return jsonString;
}

// ============ Sensor Logic ============

// Publishes the provided sensor data. This is a background task.
void publishSensorData(int lightLeve, float temperature, float humidity, bool isMoist) {
  String json = convertSensorDataToJsonString(lightLevel, temperature, humidity, isMoist);

  mqttClient.publish(MQTT_SUBJ_HUB, json.c_str());
  yield();
}

// Sets the sensors thresholds. This is a background task.
void setConfiguration(int _lightLevel, float _temperature, float _humidity) {
  mqttClient.publish(MQTT_SUBJ_HUB, "Setting config...");
  yield();

  if (_lightLevel != 0) {
    lightLevel = _lightLevel;
  }

  if (_temperature != 0.0) {
    temperature = _temperature;
  }

  if (_humidity != 0.0) {
    humidity = _humidity;
  }

}

// Turns the pump on and wait a timeout (3 secs) and turns it off.
void turnPumpOn() {
  if (!pumping) {
    mqttClient.publish(MQTT_SUBJ_HUB, "Pumping started");

    pumping = true;
    digitalWrite(P_PUMP_ON, HIGH);
    delay(PUMP_TIMEOUT);
    turnPumpOff();

    mqttClient.publish(MQTT_SUBJ_HUB, "Pumping stopped");
    yield();
  }
}

void turnPumpOff() {
  if (pumping) {
    digitalWrite(P_PUMP_ON, LOW);
    pumping = false;
  }
}

// ============ Network ============

void onMessageReceived(char *topic, byte *payload, unsigned int length) {
  const char *pump = "pump";
  const char *stop_pump = "stop_pump";
  const char *config = "config";
  char message[length + 1];

  // Copy payload into message array
  for (int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }

  // Null-terminate the message array
  message[length] = '\0';

  Serial.println("Received message: ");
  Serial.println(message);

  if (strstr(message, pump) != nullptr) {
    // Search for "pump" in the message and force turn the pump on
    turnPumpOn();
  } else if (strstr(message, stop_pump) != nullptr) {
    // Search for "stop_pump" in the message and force turn the pump on
    turnPumpOff();
  } else if (strstr(topic, config) != nullptr) {
    // If the topic is set to config then assume we are expecting a JSON message

    DynamicJsonDocument doc(300);
    deserializeJson(doc, message);
    JsonObject jsonObj = doc.as<JsonObject>();

    if (!jsonObj.isNull()) {
      // if JSON is nice, then set the configuration.
      setConfiguration(jsonObj["lightLevel"], jsonObj["temperature"], jsonObj["humidity"]);
    } else {
      // otherwise fail it.
      mqttClient.publish(MQTT_SUBJ_HUB, "Failed to set config.");
      yield();
    }
  }
}

void connectToWifi() {
  while (!WiFi.isConnected()) {
    Serial.print(".");
    WiFi.begin(SSID, PASSWORD);
    WiFi.waitForConnectResult();
  }
}

void connectToMQTTBroker() {
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCallback(onMessageReceived);
}

void reconnectMqttClient() {
  while (!mqttClient.connected()) {
    Serial.println("Attempting MQTT connection");

    if (mqttClient.connect("WeatherHub", MQTT_USER, MQTT_PASS)) {
      Serial.println("MQTT client connected");

      // attempt connection, if succesful, subscribe to the "node" and "config" topics.
      mqttClient.subscribe(MQTT_SUBJ_NODE);
      mqttClient.subscribe(MQTT_SUBJ_CONFIG);
    } else {
      Serial.println("Failed,");
      Serial.println("attempting to reconnect in 5 seconds");

      delay(5000);
    }
  }
}

// ============ Main ============

void setup() {
  if (DEBUG) Serial.begin(BAUD_RATE);
  setupPins();
  connectToWifi();
  connectToMQTTBroker();
}

// Be very careful with how long each iteration takes to complete. The Watchdog timer of the ESP will get triggered if we surpass a certain amount and the module will reset!
void loop() {
  if (!mqttClient.connected()) {
    reconnectMqttClient();
  }

  int currentLightLevel = analogRead(P_READ_LIGHT);
  float currentTemperature = dht.readTemperature();
  float currentHumidity = dht.readHumidity();
  bool isMoist = digitalRead(P_READ_MOISTURE) == LOW;

  bool shouldWater = currentLightLevel <= lightLevel && currentTemperature <= temperature && currentHumidity <= humidity && !isMoist;

  if (shouldWater) {
    turnPumpOn();
  } else {
    turnPumpOff();
  }

  publishSensorData(currentLightLevel, currentTemperature, currentHumidity, isMoist);

  mqttClient.loop();
  delay(1500);
}