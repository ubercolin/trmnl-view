#ifndef DISPLAY_CLOCK_H
#define DISPLAY_CLOCK_H

#include <Arduino.h>

class DisplayManager;

class DisplayClock
{
public:
    DisplayClock(DisplayManager *displayManager);
    void updateFull(int hour, int minute, int second, const String &dayOfWeek, const String &date);
    void updatePartial(int hour, int minute, int second);
    void drawDate(const String &dayOfWeek, const String &date);

private:
    DisplayManager *displayManager;
    int lastDisplayedHour = -1;
    int lastDisplayedMinute = -1;

    void drawTime(int hour, int minute);
};

#endif // DISPLAY_CLOCK_H
