
#include <ArduinoJson.h>

String convertSensorDataToJsonString(int lightLevel, float temperature, float humidity, bool isMoist)
{
    // Create a JSON object
    StaticJsonDocument<1024> jsonDoc; // Adjust the size according to your requirements

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

bool isJsonString(String& jsonString)
{
    // Create a DynamicJsonDocument without allocating memory
    DynamicJsonDocument jsonDoc(1024); // Adjust the size according to your JSON data

    // Deserialize the JSON string into the DynamicJsonDocument
    DeserializationError error = deserializeJson(jsonDoc, jsonString);

    // Check if the parsing was successful and the JSON object was created
    return error == DeserializationError::Ok;
}

inline JsonObject convertJsonStringToObject(String &jsonString)
{
    DynamicJsonDocument jsonDoc(1024); // Adjust the size according to your JSON data

    // Deserialize the JSON string into a JSON object
    DeserializationError error = deserializeJson(jsonDoc, jsonString);

    if (error)
    {
        Serial.print("Failed to parse JSON: ");
        Serial.println(error.c_str());
        return JsonObject(); // Return an empty object if parsing fails
    }

    JsonObject jsonObject = jsonDoc.as<JsonObject>();
    return jsonObject;
}

void setup() {
    Serial.begin(9600);
}

void loop() {
    if (Serial.available())
    {
        String data = Serial.readString();

        Serial.print("Received Data: ");
        Serial.println(data);

        Serial.print("Is it JSON? ");
        Serial.println(isJsonString(data));

        Serial.println("JSON content: ");

        DynamicJsonDocument jsonDoc(1024); // Adjust the size according to your JSON data
        deserializeJson(jsonDoc, data);

        String jsonString;
        JsonObject jsonObject = jsonDoc.as<JsonObject>();
        String hello = jsonObject["hello"];
        Serial.println(hello);

        serializeJson(jsonObject, jsonString);
        Serial.println(jsonString);
    }
}