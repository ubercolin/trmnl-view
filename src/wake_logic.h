#ifndef WAKE_LOGIC_H
#define WAKE_LOGIC_H

#include <ctime>

// Pure decision logic functions for wake scenarios
// These have no side effects and can be easily tested

class WakeLogic
{
public:
    /**
     * Determine if weather should be updated based on time elapsed
     * @param currentTime Current epoch time
     * @param lastWeatherUpdate Last time weather was updated (epoch)
     * @param weatherUpdateInterval Interval in seconds (e.g., 30 * 60)
     * @return true if weather should be updated
     */
    static bool shouldUpdateWeather(time_t currentTime, time_t lastWeatherUpdate, int weatherUpdateInterval);

    /**
     * Determine if day/date display should be updated (midnight transition)
     * @param currentDay Current day of month (tm_mday)
     * @param lastDisplayedDay Last day that was displayed
     * @return true if day has changed
     */
    static bool shouldUpdateDate(int currentDay, int lastDisplayedDay);

    /**
     * Determine if clock should be updated (hour or minute changed)
     * @param currentHour Current hour
     * @param currentMinute Current minute
     * @param lastHour Last displayed hour
     * @param lastMinute Last displayed minute
     * @return true if hour or minute has changed
     */
    static bool shouldUpdateClock(int currentHour, int currentMinute, int lastHour, int lastMinute);

    /**
     * Determine if this is first boot (no weather update recorded)
     * @param lastWeatherUpdate Last weather update time
     * @return true if lastWeatherUpdate is 0 (never updated)
     */
    static bool isFirstBoot(time_t lastWeatherUpdate);
};

#endif // WAKE_LOGIC_H
