#ifndef TYPES_H
#define TYPES_H

#include <Arduino.h>

struct WeatherData
{
    float currentTemp;
    String currentCondition;
    int humidity;
    int windSpeed;
    time_t lastUpdated; // Timestamp of last weather fetch

    // Hourly forecast (next 6 hours)
    struct HourlyForecast
    {
        int hour;
        float temp;
        String condition;
    } hourly[6];

    // Daily forecast (next 4 days)
    struct DailyForecast
    {
        String day;
        float tempHigh;
        float tempLow;
        String condition;
    } daily[4];
};

#endif // TYPES_H
