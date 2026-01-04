#include <Arduino.h>
#include "config.h"
#include "display.h"
#include "network.h"

// Disable watchdog timers to prevent early resets during init
extern "C"
{
    void esp_task_wdt_delete(void *);
}

// RTC memory survives deep sleep
RTC_DATA_ATTR time_t lastWeatherUpdate = 0;
RTC_DATA_ATTR bool isFirstBoot = true;

DisplayManager display;
NetworkManager network;

void setup()
{
    // Disable watchdog immediately
    esp_task_wdt_delete(NULL);

    Serial.begin(115200);
    delay(100);

    // Check wake reason
    esp_sleep_wakeup_cause_t wakeupReason = esp_sleep_get_wakeup_cause();
    bool wokeFromSleep = (wakeupReason == ESP_SLEEP_WAKEUP_TIMER);

    // Get current time from RTC (preserved during deep sleep)
    time_t currentTime;
    struct tm timeinfo;
    time(&currentTime);
    localtime_r(&currentTime, &timeinfo);

    // Check if weather needs updating
    bool needsWeatherUpdate = (currentTime - lastWeatherUpdate >= WEATHER_UPDATE_INTERVAL);

    if (wokeFromSleep)
    {
        Serial.println("=== WOKE FROM DEEP SLEEP ===");
        Serial.printf("Current RTC time: %02d:%02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

        // Wake display from sleep (light init, no full refresh)
        display.wakeup();

        // Always connect WiFi on wake to sync time (keeps RTC accurate)
        Serial.println("Connecting WiFi to sync time...");
        if (network.connectWiFi(WIFI_SSID, WIFI_PASSWORD))
        {
            // Sync time while we have WiFi
            network.syncTime();
            time(&currentTime);
            localtime_r(&currentTime, &timeinfo);
            Serial.printf("Time synced: %02d:%02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        }
        else
        {
            Serial.println("WiFi connection for time sync failed - using RTC (may drift)");
        }
    }
    else
    {
        // Fresh boot - full initialization
        Serial.println("=== FRESH BOOT ===");
        isFirstBoot = true;

        Serial.println("Initializing display...");
        display.init();
        Serial.println("Display initialized!");

        // Connect to WiFi
        Serial.println("Connecting to WiFi...");
        if (!network.connectWiFi(WIFI_SSID, WIFI_PASSWORD))
        {
            Serial.println("WiFi connection failed!");
            display.showError("WiFi Failed");
            return;
        }

        Serial.print("IP Address: ");
        Serial.println(network.getIPAddress());

        // Sync time
        Serial.println("Syncing time...");
        if (network.syncTime())
        {
            Serial.println("Time synced successfully!");
            time(&currentTime);
            localtime_r(&currentTime, &timeinfo);
        }
        else
        {
            Serial.println("Time sync failed!");
        }

        // Full display initialization
        const char *daysOfWeek[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
        const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

        String dayOfWeek = daysOfWeek[timeinfo.tm_wday];
        char dateStr[32];
        snprintf(dateStr, sizeof(dateStr), "%s %d, %d", months[timeinfo.tm_mon], timeinfo.tm_mday, timeinfo.tm_year + 1900);

        display.updateClock(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, dayOfWeek, String(dateStr));

        // Fetch initial weather
        Serial.println("Fetching initial weather...");
        WeatherData weather = {};
        if (network.fetchWeather(weather))
        {
            Serial.println("Weather fetched successfully!");
            display.updateWeather(weather);
            lastWeatherUpdate = currentTime;
        }

        // Update battery display
        float batteryPercent = network.readDeviceBattery();
        display.updateBattery(batteryPercent);

        isFirstBoot = false;
        needsWeatherUpdate = false; // Already done
    }

    Serial.println("Setup complete!");
    Serial.flush();
}

void loop()
{
    // Get current time
    time_t currentTime;
    struct tm timeinfo;
    time(&currentTime);
    localtime_r(&currentTime, &timeinfo);

    // Always update clock (partial update for efficiency)
    display.partialUpdateClock(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    Serial.printf("Clock updated: %02d:%02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

    // Update battery display every minute
    float batteryPercent = network.readDeviceBattery();
    display.updateBattery(batteryPercent);

    // Check if weather needs updating (every 30 minutes)
    if (currentTime - lastWeatherUpdate >= WEATHER_UPDATE_INTERVAL)
    {
        // Connect WiFi if not already connected
        if (!network.isConnected())
        {
            Serial.println("Connecting WiFi for weather update...");
            network.connectWiFi(WIFI_SSID, WIFI_PASSWORD);
        }

        Serial.println("Fetching weather...");
        WeatherData weather = {};
        if (network.fetchWeather(weather))
        {
            Serial.println("Weather updated!");
            display.updateWeather(weather);
        }
        else
        {
            Serial.println("Weather fetch failed");
        }
        lastWeatherUpdate = currentTime;
    }

    // Calculate sleep duration to wake at the top of the next minute
    int secondsInMinute = timeinfo.tm_sec;
    uint32_t sleepSeconds = 60 - secondsInMinute;

    // Add a small buffer to ensure we're past the minute boundary
    if (sleepSeconds < 3)
    {
        sleepSeconds += 60; // Sleep to the next minute instead
    }

    Serial.printf("Sleeping for %u seconds (current: %02d:%02d:%02d)\n",
                  sleepSeconds, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    Serial.flush();

    // Disconnect WiFi to save power
    network.disconnectWiFi();

#if DEBUG_NO_SLEEP
    // Debug mode: use delay instead of deep sleep to keep serial monitor active
    Serial.println("DEBUG_NO_SLEEP enabled - using delay instead of deep sleep");
    delay(sleepSeconds * 1000);
#else
    // Put display into low power mode and enter deep sleep
    display.deepSleep(sleepSeconds);

    // This line is never reached - deep sleep restarts from setup()
#endif
}
