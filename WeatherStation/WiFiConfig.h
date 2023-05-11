namespace WiFiConfig {
    const char SSID[] = "WeatherHub";
    const char PASSWORD[] = "password";
    
    const char MQTT_HOST[] = "192.168.51.86";
    const int MQTT_PORT = 1883;
    const char MQTT_USER[] = "WeatherStation";
    const char MQTT_PASS[] = "password";

    const char MQTT_SUBJ_HUB[] = "hub";
    const char MQTT_SUBJ_CONFIG[] = "config";
    const char MQTT_SUBJ_NODE[] = "node";

    const int P_READ_LIGHT = (uint8_t)17U;//A0
    const int P_READ_MOISTURE = 5; //D1
    const int P_PUMP_ON = 4; //D2
    const int P_READ_TEMP = 0; // D3
    const int PUMP_TIMEOUT = 3000; // Be very careful, we can't set this too high, otherwise the Watchdog timer will reset the board!

    int lightLevel = 400;
    float temperature = 30;
    float humidity = 40;

    const int BAUD_RATE = 115200;
    const bool DEBUG = true;
}