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

void setupPins()
{
  pinMode(P_READ_MOISTURE, INPUT);
  pinMode(P_READ_TEMP, INPUT);
  pinMode(P_READ_LIGHT, INPUT);
  pinMode(P_PUMP_ON, OUTPUT);

  digitalWrite(P_PUMP_ON, LOW);
}

// ============ JSON Utils ============

String convertSensorDataToJsonString(int lightLevel, float temperature, float humidity, bool isMoist)
{
  // Create a JSON object
  StaticJsonDocument<200> jsonDoc; // Adjust the size according to your requirements

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

bool isJsonString(const char *jsonString)
{
  // Create a DynamicJsonDocument without allocating memory
  DynamicJsonDocument jsonDoc(200); // Adjust the size according to your JSON data

  // Deserialize the JSON string into the DynamicJsonDocument
  DeserializationError error = deserializeJson(jsonDoc, jsonString);

  // Check if the parsing was successful and the JSON object was created
  return error == DeserializationError::Ok;
}

JsonObject convertJsonStringToObject(const char *jsonString)
{
  DynamicJsonDocument jsonDoc(200); // Adjust the size according to your JSON data

  // Deserialize the JSON string into a JSON object
  DeserializationError error = deserializeJson(jsonDoc, jsonString);

  if (jsonDoc.isNull())
  {
    Serial.print("Failed to parse JSON: ");
    Serial.println(error.c_str());
    return JsonObject(); // Return an empty object if parsing fails
  }
  return jsonDoc.to<JsonObject>();
}

// ============ Sensor Logic ============

void publishSensorData()
{
  int lightLevel = analogRead(P_READ_LIGHT);
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  bool isMoist = digitalRead(P_READ_MOISTURE) == LOW;
  String json = convertSensorDataToJsonString(lightLevel, temperature, humidity, isMoist);

  mqttClient.publish(MQTT_SUBJ_HUB, json.c_str());
}

void setConfiguration(JsonVariant json)
{
  mqttClient.publish(MQTT_SUBJ_HUB, "Setting config...");

  Serial.println("Setting config with: ");
  String jsonStr;
  serializeJson(json, jsonStr);
  Serial.print(jsonStr.c_str());

  int _lightLevel = json["lightLevel"].as<int>();
  float _temperature = json["temperature"].as<float>();
  float _humidity = json["humidity"].as<float>();

  if (_lightLevel != 0)
  {
    lightLevel = _lightLevel;
  }

  if (_temperature != 0.0)
  {
    temperature = _temperature;
  }

  if (_humidity != 0.0)
  {
    humidity = _humidity;
  }
}

bool shouldWater()
{
  int currentlightLevel = analogRead(P_READ_LIGHT);
  float currentTemperature = dht.readTemperature();
  float currentHumidity = dht.readHumidity();
  bool isMoist = digitalRead(P_READ_MOISTURE) == LOW;

  Serial.print("Light level: ");
  Serial.println(lightLevel);
  Serial.print("Temperature: ");
  Serial.println(temperature);
  Serial.print("Humidity: ");
  Serial.println(humidity);
  Serial.print("Is Dry: ");
  Serial.println(!isMoist);

  return currentlightLevel <= lightLevel && currentTemperature <= temperature && currentHumidity <= humidity && !isMoist;
}

void turnPumpOn()
{
  if (!pumping)
  {
    mqttClient.publish(MQTT_SUBJ_HUB, "Pumping started");

    pumping = true;
    digitalWrite(P_PUMP_ON, HIGH);
    delay(PUMP_TIMEOUT);

    turnPumpOff();

    mqttClient.publish(MQTT_SUBJ_HUB, "Pumping stopped");
  }
}

void turnPumpOff()
{
  digitalWrite(P_PUMP_ON, LOW);
  pumping = false;
}

// ============ Network ============

void onMessageReceived(char *topic, byte *payload, unsigned int length)
{
  const char *pump = "pump";
  const char *stop_pump = "stop_pump";
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
  else if (strstr(message, stop_pump) != nullptr)
  {
    turnPumpOff();
  }
  else if (isJsonString(message))
  {
    JsonObject jsonObj = convertJsonStringToObject(message);
    if (jsonObj != NULL)
    {
      setConfiguration(jsonObj);
    } else {
      mqttClient.publish(MQTT_SUBJ_HUB, "Failed to set config.");
    }
  }
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

  mqttClient.setServer("192.168.51.86", MQTT_PORT);
  mqttClient.setCallback(onMessageReceived);
  Serial.println();
}

void reconnectMqttClient()
{
  while (!mqttClient.connected())
  {
    Serial.println("Attempting MQTT connection");

    if (mqttClient.connect("WeatherHub", MQTT_USER, MQTT_PASS))
    {
      Serial.println("MQTT client connected");

      mqttClient.subscribe(MQTT_SUBJ_NODE);
    }
    else
    {
      Serial.println("Failed,");
      Serial.println();
      Serial.println(" attempting to reconnect in 5 seconds");

      delay(5000);
    }
  }
}

// ============ Main loop ============

void setup()
{
  // Serial.begin(BAUD_RATE); // When using the GPIO3 pin (RX,TX) do not enable serial output
  Serial.begin(BAUD_RATE);
  setupPins();
  connectToWifi();
  connectToMQTTBroker();
}

void loop()
{
  if (!mqttClient.connected())
  {
    reconnectMqttClient();
  }

  if (shouldWater())
  {
    turnPumpOn();
  }
  else
  {
    turnPumpOff();
  }

  publishSensorData();

  mqttClient.loop();
  delay(5000);
}