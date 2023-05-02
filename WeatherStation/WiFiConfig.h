namespace WiFiConfig {
    const char SSID[15] = "WeatherHub";
    const char PASSWORD[9] = "password";
    
    const int MQTT_PORT = 1883;
    const char MQTT_USER[15] = "WeatherStation";
    const char MQTT_PASS[9] = "password";
    const char MQTT_SUBJ[13] = "weather/soil";

    const int P_READ_MOISTURE = 2;
    const int P_PUMP_ON = 0;
    const int PUMP_TIMEOUT = 3000;

    const int BAUD_RATE = 115200;
    const bool DEBUG = true;
}