#include <Arduino.h>
#include <cstdlib>
#include "config.h"
#include "display.h"
#include "network.h"
#include "wake_logic.h"

// Disable watchdog timers to prevent early resets during init
extern "C"
{
    void esp_task_wdt_delete(void *);
}

// RTC memory survives deep sleep
RTC_DATA_ATTR time_t lastWeatherUpdate = 0;
RTC_DATA_ATTR bool isFirstBoot = true;
RTC_DATA_ATTR int lastDisplayedDay = -1; // Track last displayed day to detect midnight transitions

DisplayManager display;
NetworkManager network;

// Common update logic used by both first boot and regular wakes
void performUpdates()
{
    // Get current time
    time_t currentTime;
    struct tm timeinfo;
    time(&currentTime);
    localtime_r(&currentTime, &timeinfo);

    // On fresh boot, do full display update; otherwise partial updates
    if (isFirstBoot)
    {
        Serial.println("Fresh boot - doing initial full display...");
        display.updateClock(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec,
                            timeinfo.tm_wday, timeinfo.tm_mon, timeinfo.tm_mday, timeinfo.tm_year + 1900);
        lastDisplayedDay = timeinfo.tm_mday;
        isFirstBoot = false;
    }
    else
    {
        // Regular partial updates on wake from sleep
        display.partialUpdateClock(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        Serial.printf("Clock updated: %02d:%02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

        // Update date display if day changed
        if (WakeLogic::shouldUpdateDate(timeinfo.tm_mday, lastDisplayedDay))
        {
            Serial.printf("Day changed from %d to %d - updating date display\n", lastDisplayedDay, timeinfo.tm_mday);
            display.partialUpdateDate(timeinfo.tm_wday, timeinfo.tm_mon, timeinfo.tm_mday, timeinfo.tm_year + 1900);
            lastDisplayedDay = timeinfo.tm_mday;
        }
    }

    // Battery and weather updates happen every cycle regardless of boot state
    float batteryPercent = network.readDeviceBattery();
    display.updateBattery(batteryPercent);

    // Update weather if needed (every 30 minutes)
    if (WakeLogic::shouldUpdateWeather(currentTime, lastWeatherUpdate, WEATHER_UPDATE_INTERVAL))
    {
        Serial.println("Weather update needed - connecting WiFi...");

        // Connect WiFi and sync time for accurate weather fetch and future cycles
        if (!network.isConnected())
        {
            network.connectWiFi(WIFI_SSID, WIFI_PASSWORD);
        }

        if (network.isConnected())
        {
            Serial.println("Syncing time for accurate weather fetch...");
            network.syncTime();
            // Update currentTime after sync for weather timestamp
            time(&currentTime);
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
}

void setup()
{
    // Disable watchdog immediately
    esp_task_wdt_delete(NULL);

    Serial.begin(115200);
    delay(100);

    // Set timezone early so time conversions are correct
    setenv("TZ", TZ_INFO, 1);
    tzset();

    // Check wake reason
    esp_sleep_wakeup_cause_t wakeupReason = esp_sleep_get_wakeup_cause();
    bool wokeFromSleep = (wakeupReason == ESP_SLEEP_WAKEUP_TIMER);

    if (wokeFromSleep)
    {
        Serial.println("=== WOKE FROM DEEP SLEEP ===");
        display.wakeup();
    }
    else
    {
        // Fresh boot - hardware initialization only
        Serial.println("=== FRESH BOOT ===");

        Serial.println("Initializing display...");
        display.init();
        Serial.println("Display initialized!");

        // WiFi and time sync on fresh boot
        Serial.println("Connecting to WiFi...");
        if (!network.connectWiFi(WIFI_SSID, WIFI_PASSWORD))
        {
            Serial.println("WiFi connection failed!");
            display.showError("WiFi Failed");
            return;
        }

        Serial.print("IP Address: ");
        Serial.println(network.getIPAddress());

        Serial.println("Syncing time...");
        if (!network.syncTime())
        {
            Serial.println("Time sync failed!");
        }

        // Initialize RTC tracking for next cycles
        lastWeatherUpdate = 0; // Force weather update in loop
        lastDisplayedDay = -1; // Force date update in loop
        // Keep isFirstBoot = true so performUpdates knows to do full initial display
    }

    Serial.println("Setup complete!");
    Serial.flush();
}

void loop()
{
    performUpdates();

    // Get current time for sleep calculation
    time_t currentTime;
    struct tm timeinfo;
    time(&currentTime);
    localtime_r(&currentTime, &timeinfo);

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
