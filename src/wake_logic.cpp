#include "wake_logic.h"

bool WakeLogic::shouldUpdateWeather(time_t currentTime, time_t lastWeatherUpdate, int weatherUpdateInterval)
{
    // On first boot, lastWeatherUpdate is 0, so update immediately
    if (lastWeatherUpdate == 0)
    {
        return true;
    }
    return (currentTime - lastWeatherUpdate >= weatherUpdateInterval);
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
