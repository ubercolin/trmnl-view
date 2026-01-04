#ifndef DISPLAY_CLOCK_H
#define DISPLAY_CLOCK_H

#include <Arduino.h>

class DisplayManager;

class DisplayClock
{
public:
    DisplayClock(DisplayManager *displayManager);
    void draw(int x, int hour, int minute, int second, const String &dayOfWeek, const String &date);

private:
    DisplayManager *displayManager;
};

#endif // DISPLAY_CLOCK_H
