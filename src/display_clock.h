#ifndef DISPLAY_CLOCK_H
#define DISPLAY_CLOCK_H

#include <Arduino.h>

class DisplayManager;

class DisplayClock
{
public:
    DisplayClock(DisplayManager *displayManager);
    void updateFull(int hour, int minute, int second, int dayOfWeek, int month, int day, int year);
    void updatePartial(int hour, int minute, int second);
    void drawDate(int dayOfWeek, int month, int day, int year);

private:
    DisplayManager *displayManager;
    int lastDisplayedHour = -1;
    int lastDisplayedMinute = -1;

    void drawTime(int hour, int minute);

    // Helper methods to get formatted day and date strings
    String getDayOfWeekName(int dayOfWeekIndex);
    String getFormattedDate(int month, int day, int year);
};

#endif // DISPLAY_CLOCK_H
