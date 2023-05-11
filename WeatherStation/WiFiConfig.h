namespace WiFiConfig {
    const char SSID[15] = "WeatherHub";
    const char PASSWORD[9] = "password";
    
    const int MQTT_PORT = 1883;
    const char MQTT_USER[15] = "WeatherStation";
    const char MQTT_PASS[9] = "password";
    const char MQTT_SUBJ_HUB[13] = "hub";
    const char MQTT_SUBJ_NODE[13] = "node";

    const int P_READ_TEMP = 0;
    const int P_READ_LIGHT = (uint8_t)17U;
    const int P_READ_MOISTURE = 5; //This can't be the regular IO pins (0 and 2) as they are pullup input pins
    const int P_PUMP_ON = 4; // Boot fails if pulled low!!
    const int PUMP_TIMEOUT = 3000;

    int lightLevel = 400;
    float temperature = 30;
    float humidity = 40;

    const int BAUD_RATE = 115200;
    const bool DEBUG = true;
}