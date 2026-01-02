#ifndef DISPLAY_H
#define DISPLAY_H

#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include "types.h"

class DisplayManager
{
public:
    DisplayManager();
    void init();
    void showIPAddress(const String &ipAddress);
    void showError(const String &errorMessage);
    void updateClock(int hour, int minute, int second, const String &dayOfWeek, const String &date);
    void updateWeather(const WeatherData &weather);
    void updateBattery(float batteryPercent);
    void partialUpdateClock(int hour, int minute, int second);
    void deepSleep();
    void wakeup();

private:
    GxEPD2_BW<GxEPD2_750_T7, GxEPD2_750_T7::HEIGHT> display;
    float currentBattery = 0;

    void drawClockSection(int hour, int minute, int second, const String &dayOfWeek, const String &date);
    void drawWeatherSection(const WeatherData &weather);
    void drawTemperatureBox(int x, int y, int w, int h, float temp, const String &label);
    void drawForecastRow(int y, const WeatherData &weather, bool isHourly);
    void drawWeatherIcon(int x, int y, const String &condition);

    int lastDisplayedHour = -1;
    int lastDisplayedMinute = -1;
};

#endif // DISPLAY_H
