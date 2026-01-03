#include "network.h"
#include "config.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>
#include <ArduinoJson.h>

NetworkManager::NetworkManager()
{
}

bool NetworkManager::connectWiFi(const char *ssid, const char *password)
{
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20)
    {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("");
        Serial.print("WiFi connected! IP: ");
        Serial.println(WiFi.localIP());
        return true;
    }

    Serial.println("");
    Serial.println("WiFi connection failed!");
    return false;
}

void NetworkManager::disconnectWiFi()
{
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
}

bool NetworkManager::isConnected()
{
    return WiFi.status() == WL_CONNECTED;
}

String NetworkManager::getIPAddress()
{
    if (isConnected())
    {
        return WiFi.localIP().toString();
    }
    return "Not Connected";
}

bool NetworkManager::syncTime()
{
    Serial.println("Syncing time with NTP...");

    configTime(0, 0, NTP_SERVER);
    setenv("TZ", TZ_INFO, 1);
    tzset();

    // Wait for time to be set
    int attempts = 0;
    time_t now = 0;
    struct tm timeinfo = {};

    while (attempts < 10)
    {
        time(&now);
        localtime_r(&now, &timeinfo);
        if (timeinfo.tm_year > (2020 - 1900))
        {
            Serial.print("Time synced: ");
            Serial.println(asctime(&timeinfo));
            return true;
        }
        delay(500);
        attempts++;
    }

    Serial.println("Time sync failed!");
    return false;
}

bool NetworkManager::fetchWeather(WeatherData &weatherData)
{
    if (!isConnected())
    {
        Serial.println("WiFi not connected, cannot fetch weather");
        return false;
    }

    HTTPClient http;
    String url = String(WEATHER_API_URL) +
                 "?latitude=" + WEATHER_LATITUDE +
                 "&longitude=" + WEATHER_LONGITUDE +
                 "&current=temperature_2m,relative_humidity_2m,weather_code" +
                 "&hourly=temperature_2m,weather_code&daily=temperature_2m_max,temperature_2m_min,weather_code" +
                 "&temperature_unit=fahrenheit&timezone=auto&forecast_days=5";

    Serial.print("Fetching weather from: ");
    Serial.println(url);

    http.begin(url);
    int httpCode = http.GET();

    Serial.printf("HTTP response code: %d\n", httpCode);

    if (httpCode != 200)
    {
        Serial.printf("HTTP error: %d\n", httpCode);
        http.end();
        return false;
    }

    String payload = http.getString();
    http.end();

    Serial.printf("Payload length: %d\n", payload.length());
    if (payload.length() > 200)
    {
        Serial.println(payload.substring(0, 200));
    }
    else
    {
        Serial.println(payload);
    }

    bool parseResult = parseWeatherJson(payload, weatherData);
    Serial.printf("Parse result: %d\n", parseResult);
    return parseResult;
}

bool NetworkManager::parseWeatherJson(const String &jsonResponse, WeatherData &weatherData)
{
    // Initialize with defaults
    weatherData.currentTemp = 0;
    weatherData.currentCondition = "Unknown";
    weatherData.humidity = 0;
    weatherData.windSpeed = 0;

    // Use static DynamicJsonDocument to avoid stack overflow
    // Static means it's allocated once on heap, not on stack
    static DynamicJsonDocument doc(20480);
    doc.clear(); // Clear previous data

    DeserializationError error = deserializeJson(doc, jsonResponse);

    if (error)
    {
        Serial.print("JSON parse error: ");
        Serial.println(error.c_str());
        return false;
    }

    try
    {
        // Current weather
        if (!doc["current"].isNull())
        {
            JsonObject current = doc["current"];
            weatherData.currentTemp = current["temperature_2m"] | 0.0f;
            weatherData.humidity = current["relative_humidity_2m"] | 0;

            // Simple WMO weather code to condition mapping
            int weatherCode = current["weather_code"] | 0;
            weatherData.currentCondition = getWeatherCondition(weatherCode);

            Serial.printf("Current: %.1f°F, %d%%, %s\n", weatherData.currentTemp, weatherData.humidity, weatherData.currentCondition.c_str());
        }
        else
        {
            Serial.println("No current data in response");
            return false;
        }

        // Hourly forecast (next 6 hours)
        if (doc["hourly"].isNull())
        {
            Serial.println("No hourly data in response");
            return false;
        }

        JsonArray hourlyTimes = doc["hourly"]["time"];
        JsonArray hourlyTemps = doc["hourly"]["temperature_2m"];
        JsonArray hourlyWeatherCodes = doc["hourly"]["weather_code"];

        // Find the current hour to determine starting index
        time_t now;
        struct tm timeinfo;
        time(&now);
        localtime_r(&now, &timeinfo);
        int currentHour = timeinfo.tm_hour;

        // Find the first hourly entry that matches or exceeds current hour
        int startIndex = currentHour + 1; // Start from next hour
        if (startIndex >= hourlyTemps.size())
            startIndex = 0;

        for (int i = 0; i < 6 && (startIndex + i) < hourlyTemps.size(); i++)
        {
            // Extract hour from ISO timestamp (e.g., "2026-01-02T14:00" → 14)
            String timeStr = hourlyTimes[startIndex + i].as<String>();
            int hourStart = timeStr.indexOf('T') + 1;
            weatherData.hourly[i].hour = timeStr.substring(hourStart, hourStart + 2).toInt();

            weatherData.hourly[i].temp = hourlyTemps[startIndex + i] | 0.0f;
            weatherData.hourly[i].condition = getWeatherCondition((int)(hourlyWeatherCodes[startIndex + i] | 0));
        }

        // Daily forecast (next 4 days)
        if (doc["daily"].isNull())
        {
            Serial.println("No daily data in response");
            return false;
        }

        JsonArray dailyTimes = doc["daily"]["time"];
        JsonArray dailyTempMax = doc["daily"]["temperature_2m_max"];
        JsonArray dailyTempMin = doc["daily"]["temperature_2m_min"];
        JsonArray dailyWeatherCodes = doc["daily"]["weather_code"];

        const char *daysOfWeek[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

        for (int i = 0; i < 4 && i < dailyTempMax.size(); i++)
        {
            // Extract date from ISO timestamp (e.g., "2026-01-03" → day of week)
            String dateStr = dailyTimes[i].as<String>();
            // Parse year, month, day from YYYY-MM-DD format
            int year = dateStr.substring(0, 4).toInt();
            int month = dateStr.substring(5, 7).toInt();
            int day = dateStr.substring(8, 10).toInt();

            // Zeller's congruence to get day of week
            int q = day;
            int m = month;
            int k = year % 100;
            int j = year / 100;

            if (m < 3)
            {
                m += 12;
                k--;
            }

            int h = (q + (13 * (m + 1)) / 5 + k + k / 4 + j / 4 - 2 * j) % 7;
            int dayOfWeek = (h + 5) % 7; // Convert to 0=Sun, 1=Mon, etc.

            weatherData.daily[i].day = daysOfWeek[dayOfWeek];
            weatherData.daily[i].tempHigh = dailyTempMax[i] | 0.0f;
            weatherData.daily[i].tempLow = dailyTempMin[i] | 0.0f;
            weatherData.daily[i].condition = getWeatherCondition((int)(dailyWeatherCodes[i] | 0));
        }

        Serial.println("Weather data parsed successfully!");

        // Timestamp the weather data with current time
        time(&now);
        weatherData.lastUpdated = now;

        return true;
    }
    catch (const std::exception &e)
    {
        Serial.print("Exception during parsing: ");
        Serial.println(e.what());
        return false;
    }
    catch (...)
    {
        Serial.println("Unknown error parsing weather data");
        return false;
    }
}

String NetworkManager::getWeatherCondition(int wmoCode)
{
    // Simplified WMO weather code to text mapping
    if (wmoCode == 0 || wmoCode == 1)
        return "Clear";
    if (wmoCode == 2)
        return "Cloudy";
    if (wmoCode == 3)
        return "Overcast";
    if (wmoCode == 45 || wmoCode == 48)
        return "Foggy";
    if (wmoCode >= 51 && wmoCode <= 67)
        return "Rain";
    if (wmoCode >= 71 && wmoCode <= 87)
        return "Snow";
    if (wmoCode >= 80 && wmoCode <= 82)
        return "Showers";
    if (wmoCode == 85 || wmoCode == 86)
        return "Snow";
    if (wmoCode >= 90 && wmoCode <= 99)
        return "Thunder";
    return "Unknown";
}

float NetworkManager::readDeviceBattery()
{
    // Take 8 samples and average
    int32_t adc = 0;
    analogRead(PIN_BATTERY); // Initialize ADC
    for (int i = 0; i < 8; i++)
    {
        adc += analogReadMilliVolts(PIN_BATTERY);
    }
    // Voltage divider: multiply by 2, convert mV to V
    float voltage = (adc / 8.0f) * 2.0f / 1000.0f;

    // Convert to percentage (3.0V = 0%, 4.2V = 100%)
    float percent = (voltage - 3.0) / (4.2 - 3.0) * 100.0;
    percent = constrain(percent, 0, 100);

    Serial.printf("Device battery: %.2fV (%.0f%%)\n", voltage, percent);
    return percent;
}
