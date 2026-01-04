#include <unity.h>
#include "../src/wake_logic.h"
#include "../src/wake_logic.cpp"  // Include implementation directly for testing

void test_shouldUpdateWeather_immediately_on_first_boot()
{
    // First boot: lastWeatherUpdate = 0, currentTime = 100
    time_t currentTime = 100;
    time_t lastWeatherUpdate = 0;
    int weatherUpdateInterval = 30 * 60; // 30 minutes

    TEST_ASSERT_TRUE(WakeLogic::shouldUpdateWeather(currentTime, lastWeatherUpdate, weatherUpdateInterval));
}

void test_shouldUpdateWeather_after_interval()
{
    // 30 minutes have passed
    time_t currentTime = 2000;
    time_t lastWeatherUpdate = 200;
    int weatherUpdateInterval = 30 * 60; // 1800 seconds

    TEST_ASSERT_TRUE(WakeLogic::shouldUpdateWeather(currentTime, lastWeatherUpdate, weatherUpdateInterval));
}

void test_shouldUpdateWeather_not_yet()
{
    // Only 10 minutes have passed, need 30
    time_t currentTime = 200 + (10 * 60);
    time_t lastWeatherUpdate = 200;
    int weatherUpdateInterval = 30 * 60;

    TEST_ASSERT_FALSE(WakeLogic::shouldUpdateWeather(currentTime, lastWeatherUpdate, weatherUpdateInterval));
}

void test_shouldUpdateWeather_exactly_at_interval()
{
    // Exactly 30 minutes have passed
    time_t currentTime = 200 + (30 * 60);
    time_t lastWeatherUpdate = 200;
    int weatherUpdateInterval = 30 * 60;

    TEST_ASSERT_TRUE(WakeLogic::shouldUpdateWeather(currentTime, lastWeatherUpdate, weatherUpdateInterval));
}

void test_shouldUpdateDate_on_day_change()
{
    int currentDay = 4;
    int lastDisplayedDay = 3;

    TEST_ASSERT_TRUE(WakeLogic::shouldUpdateDate(currentDay, lastDisplayedDay));
}

void test_shouldUpdateDate_same_day()
{
    int currentDay = 4;
    int lastDisplayedDay = 4;

    TEST_ASSERT_FALSE(WakeLogic::shouldUpdateDate(currentDay, lastDisplayedDay));
}

void test_shouldUpdateDate_first_boot()
{
    // On first boot, lastDisplayedDay = -1
    int currentDay = 3;
    int lastDisplayedDay = -1;

    TEST_ASSERT_TRUE(WakeLogic::shouldUpdateDate(currentDay, lastDisplayedDay));
}

void test_shouldUpdateDate_wrap_around_month()
{
    // Going from Jan 31 to Feb 1
    int currentDay = 1;
    int lastDisplayedDay = 31;

    TEST_ASSERT_TRUE(WakeLogic::shouldUpdateDate(currentDay, lastDisplayedDay));
}

void test_shouldUpdateClock_hour_changed()
{
    int currentHour = 14;
    int currentMinute = 30;
    int lastHour = 13;
    int lastMinute = 30;

    TEST_ASSERT_TRUE(WakeLogic::shouldUpdateClock(currentHour, currentMinute, lastHour, lastMinute));
}

void test_shouldUpdateClock_minute_changed()
{
    int currentHour = 14;
    int currentMinute = 31;
    int lastHour = 14;
    int lastMinute = 30;

    TEST_ASSERT_TRUE(WakeLogic::shouldUpdateClock(currentHour, currentMinute, lastHour, lastMinute));
}

void test_shouldUpdateClock_no_change()
{
    int currentHour = 14;
    int currentMinute = 30;
    int lastHour = 14;
    int lastMinute = 30;

    TEST_ASSERT_FALSE(WakeLogic::shouldUpdateClock(currentHour, currentMinute, lastHour, lastMinute));
}

void test_shouldUpdateClock_second_changed_only()
{
    // Seconds change but we only care about hour/minute
    int currentHour = 14;
    int currentMinute = 30;
    int lastHour = 14;
    int lastMinute = 30;

    TEST_ASSERT_FALSE(WakeLogic::shouldUpdateClock(currentHour, currentMinute, lastHour, lastMinute));
}

void test_isFirstBoot_on_first_boot()
{
    time_t lastWeatherUpdate = 0;

    TEST_ASSERT_TRUE(WakeLogic::isFirstBoot(lastWeatherUpdate));
}

void test_isFirstBoot_after_update()
{
    time_t lastWeatherUpdate = 1234567890;

    TEST_ASSERT_FALSE(WakeLogic::isFirstBoot(lastWeatherUpdate));
}

// Integrated scenario tests

void test_scenario_regular_minute_wake_no_weather_update()
{
    // Device wakes every minute, but weather only updates every 30 minutes
    time_t firstWake = 1000;
    time_t secondWake = 1000 + 60;  // 1 minute later
    time_t lastWeatherUpdate = 1000;
    int weatherUpdateInterval = 30 * 60;

    bool shouldUpdate = WakeLogic::shouldUpdateWeather(secondWake, lastWeatherUpdate, weatherUpdateInterval);
    TEST_ASSERT_FALSE(shouldUpdate);
}

void test_scenario_30_minute_weather_update()
{
    // At 30 minute mark, weather should update
    time_t wakeTime = 1000 + (30 * 60);
    time_t lastWeatherUpdate = 1000;
    int weatherUpdateInterval = 30 * 60;

    bool shouldUpdate = WakeLogic::shouldUpdateWeather(wakeTime, lastWeatherUpdate, weatherUpdateInterval);
    TEST_ASSERT_TRUE(shouldUpdate);
}

void test_scenario_midnight_day_change()
{
    // Clock rolls over from 23:59 to 00:00, day changes from 3 to 4
    int currentDay = 4;
    int lastDisplayedDay = 3;

    bool shouldUpdateDate = WakeLogic::shouldUpdateDate(currentDay, lastDisplayedDay);
    TEST_ASSERT_TRUE(shouldUpdateDate);
}

void test_scenario_multiple_wake_cycles_same_day()
{
    // Multiple minute wakes throughout the day, day never changes
    int currentDay = 4;
    int lastDisplayedDay = 4;

    for (int i = 0; i < 10; i++)
    {
        bool shouldUpdateDate = WakeLogic::shouldUpdateDate(currentDay, lastDisplayedDay);
        TEST_ASSERT_FALSE(shouldUpdateDate);
    }
}

int main()
{
    UNITY_BEGIN();

    // shouldUpdateWeather tests
    RUN_TEST(test_shouldUpdateWeather_immediately_on_first_boot);
    RUN_TEST(test_shouldUpdateWeather_after_interval);
    RUN_TEST(test_shouldUpdateWeather_not_yet);
    RUN_TEST(test_shouldUpdateWeather_exactly_at_interval);

    // shouldUpdateDate tests
    RUN_TEST(test_shouldUpdateDate_on_day_change);
    RUN_TEST(test_shouldUpdateDate_same_day);
    RUN_TEST(test_shouldUpdateDate_first_boot);
    RUN_TEST(test_shouldUpdateDate_wrap_around_month);

    // shouldUpdateClock tests
    RUN_TEST(test_shouldUpdateClock_hour_changed);
    RUN_TEST(test_shouldUpdateClock_minute_changed);
    RUN_TEST(test_shouldUpdateClock_no_change);
    RUN_TEST(test_shouldUpdateClock_second_changed_only);

    // isFirstBoot tests
    RUN_TEST(test_isFirstBoot_on_first_boot);
    RUN_TEST(test_isFirstBoot_after_update);

    // Scenario tests
    RUN_TEST(test_scenario_regular_minute_wake_no_weather_update);
    RUN_TEST(test_scenario_30_minute_weather_update);
    RUN_TEST(test_scenario_midnight_day_change);
    RUN_TEST(test_scenario_multiple_wake_cycles_same_day);

    return UNITY_END();
}


