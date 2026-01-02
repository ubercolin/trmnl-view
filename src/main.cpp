#include <Arduino.h>
#include "config.h"
#include "display.h"
#include "network.h"

// Disable watchdog timers to prevent early resets during init
extern "C"
{
    void esp_task_wdt_delete(void *);
}

DisplayManager display;
NetworkManager network;

void setup()
{
    // Disable watchdog immediately
    esp_task_wdt_delete(NULL);

    Serial.begin(115200);
    delay(100);

    Serial.println("=== SERIAL STARTED ===");
    Serial.flush();

    delay(1000);

    Serial.println("Initializing display...");
    Serial.flush();

    display.init();

    Serial.println("Display initialized!");

    // Connect to WiFi
    Serial.println("Connecting to WiFi...");
    if (network.connectWiFi(WIFI_SSID, WIFI_PASSWORD))
    {
        String ipAddress = network.getIPAddress();
        Serial.print("IP Address: ");
        Serial.println(ipAddress);

        // Sync time
        Serial.println("Syncing time...");
        if (network.syncTime())
        {
            Serial.println("Time synced successfully!");
        }
        else
        {
            Serial.println("Time sync failed!");
        }

        // Show initial clock
        time_t now;
        struct tm timeinfo;
        time(&now);
        localtime_r(&now, &timeinfo);

        const char *daysOfWeek[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
        const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

        String dayOfWeek = daysOfWeek[timeinfo.tm_wday];
        char dateStr[32];
        snprintf(dateStr, sizeof(dateStr), "%s %d, %d", months[timeinfo.tm_mon], timeinfo.tm_mday, timeinfo.tm_year + 1900);

        display.updateClock(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, dayOfWeek, String(dateStr));

        // Read battery (before WiFi uses power)
        float batteryPercent = network.readDeviceBattery();

        // Fetch initial weather
        Serial.println("Fetching initial weather...");
        WeatherData weather = {};
        if (network.fetchWeather(weather))
        {
            Serial.println("Weather fetched successfully!");
            display.updateWeather(weather);
        }
        else
        {
            Serial.println("Weather fetch failed!");
        }

        // Display battery on clock
        display.updateBattery(batteryPercent);
    }
    else
    {
        Serial.println("WiFi connection failed!");
        display.showError("WiFi Failed");
    }

    Serial.println("Setup complete!");
    Serial.flush();
}

void loop()
{
    static unsigned long lastClockUpdate = 0;
    static unsigned long lastWeatherUpdate = 0;
    unsigned long now = millis();

    // Update clock every minute
    if (now - lastClockUpdate >= CLOCK_UPDATE_INTERVAL * 1000)
    {
        time_t currentTime;
        struct tm timeinfo;
        time(&currentTime);
        localtime_r(&currentTime, &timeinfo);

        // Use partial update for efficiency
        display.partialUpdateClock(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

        Serial.printf("Clock updated: %02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min);
        lastClockUpdate = now;
    }

    // Update weather every 30 minutes
    if (now - lastWeatherUpdate >= WEATHER_UPDATE_INTERVAL * 1000)
    {
        Serial.println("Fetching weather...");
        WeatherData weather = {};
        if (network.fetchWeather(weather))
        {
            Serial.println("Weather updated!");
            display.updateWeather(weather);
            
            // Update battery display along with weather
            float batteryPercent = network.readDeviceBattery();
            display.updateBattery(batteryPercent);
        }
        else
        {
            Serial.println("Weather fetch failed");
        }
        lastWeatherUpdate = now;
    }

    delay(1000);
}
