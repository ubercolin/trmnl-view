#ifndef DISPLAY_H
#define DISPLAY_H

#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include "types.h"
#include "weather_bitmaps.h"
#include "display_clock.h"
#include "display_weather.h"

// Bounding box info for rendered text
struct TextBounds
{
    int16_t x, y, w, h;
};

class DisplayManager
{
public:
    DisplayManager();
    void init();
    void showError(const String &errorMessage);
    void updateClock(int hour, int minute, int second, const String &dayOfWeek, const String &date);
    void updateWeather(const WeatherData &weather);
    void updateBattery(float batteryPercent);
    void partialUpdateClock(int hour, int minute, int second);
    void deepSleep(uint32_t sleepSeconds);
    void wakeup();

    // Exposed for helper classes
    GxEPD2_BW<GxEPD2_750_T7, GxEPD2_750_T7::HEIGHT> &getDisplay() { return display; }
    TextBounds drawCenteredText(const char *text, int16_t centerX, int16_t y);
    void drawBitmapIcon(int x, int y, const unsigned char *bitmap, int size);

private:
    GxEPD2_BW<GxEPD2_750_T7, GxEPD2_750_T7::HEIGHT> display;
    float currentBattery = 0;
    DisplayClock clockDisplay;
    DisplayWeather weatherDisplay;
};

#endif // DISPLAY_H
