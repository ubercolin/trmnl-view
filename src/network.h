#ifndef NETWORK_H
#define NETWORK_H

#include <ArduinoJson.h>
#include "types.h"

class NetworkManager
{
public:
    NetworkManager();
    bool connectWiFi(const char *ssid, const char *password);
    void disconnectWiFi();
    bool isConnected();
    String getIPAddress();

    // Time synchronization
    bool syncTime();

    // Weather API
    bool fetchWeather(WeatherData &weatherData);

private:
    bool parseWeatherJson(const String &jsonResponse, WeatherData &weatherData);
    String getWeatherCondition(int wmoCode);
};

#endif // NETWORK_H
