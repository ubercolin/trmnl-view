#ifndef DISPLAY_WEATHER_H
#define DISPLAY_WEATHER_H

#include <Arduino.h>
#include "types.h"

class DisplayManager;

class DisplayWeather
{
public:
    DisplayWeather(DisplayManager *displayManager);
    void draw(int startX, const WeatherData &weather);

    void drawLastUpdated(const WeatherData &weather, int startX);

private:
    DisplayManager *displayManager;

    void drawCurrentTemperature(int startX, int startY, float temp);
    void drawHourly(int startX, int startY, const WeatherData &weather);
    void drawDaily(int startX, int startY, const WeatherData &weather);
};

#endif // DISPLAY_WEATHER_H
