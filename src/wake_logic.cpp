#include "wake_logic.h"
#include <Arduino.h>

bool WakeLogic::shouldUpdateWeather(time_t currentTime, time_t lastWeatherUpdate, int weatherUpdateInterval)
{
    // On first boot, lastWeatherUpdate is 0, so update immediately
    if (lastWeatherUpdate == 0)
    {
        return true;
    }

    bool should_update = (currentTime - lastWeatherUpdate >= weatherUpdateInterval);
    Serial.printf("Weather check: current=%ld, lastUpdate=%ld, diff=%ld, interval=%d, result=%s\n",
                  currentTime, lastWeatherUpdate, currentTime - lastWeatherUpdate, weatherUpdateInterval,
                  should_update ? "YES" : "NO");
    Serial.flush();
    return should_update;
}

bool WakeLogic::shouldUpdateDate(int currentDay, int lastDisplayedDay)
{
    return (currentDay != lastDisplayedDay);
}

bool WakeLogic::shouldUpdateClock(int currentHour, int currentMinute, int lastHour, int lastMinute)
{
    return (currentHour != lastHour || currentMinute != lastMinute);
}

bool WakeLogic::isFirstBoot(time_t lastWeatherUpdate)
{
    return (lastWeatherUpdate == 0);
}
