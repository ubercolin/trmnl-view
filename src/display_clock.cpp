#include "display_clock.h"
#include "display.h"
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include "config.h"

DisplayClock::DisplayClock(DisplayManager *displayManager) : displayManager(displayManager)
{
}

void DisplayClock::updateFull(int hour, int minute, int second, const String &dayOfWeek, const String &date)
{
    auto &display = displayManager->getDisplay();
    display.setPartialWindow(0, 0, DISPLAY_LEFT_HALF, DISPLAY_HEIGHT);
    display.firstPage();
    do
    {
        display.fillRect(0, 0, DISPLAY_LEFT_HALF, DISPLAY_HEIGHT, GxEPD_WHITE);
        drawTime(hour, minute);
        drawDate(dayOfWeek, date);
    } while (display.nextPage());

    lastDisplayedHour = hour;
    lastDisplayedMinute = minute;
}

void DisplayClock::updatePartial(int hour, int minute, int second)
{
    // Only update if time has changed
    if (hour == lastDisplayedHour && minute == lastDisplayedMinute)
    {
        return;
    }

    auto &display = displayManager->getDisplay();
    // Update only the time area (partial refresh for efficiency)
    // Window from y=80 to y=230 covers just the time, not day/date below
    display.setPartialWindow(0, 80, DISPLAY_LEFT_HALF, 140);
    display.firstPage();
    do
    {
        display.fillRect(0, 80, DISPLAY_LEFT_HALF, 140, GxEPD_WHITE);
        drawTime(hour, minute);
    } while (display.nextPage());

    lastDisplayedHour = hour;
    lastDisplayedMinute = minute;
}

void DisplayClock::drawTime(int hour, int minute)
{
    // Very large time display (roughly 90px tall)
    displayManager->getDisplay().setFont(&FreeMonoBold18pt7b);
    displayManager->getDisplay().setTextColor(GxEPD_BLACK);
    displayManager->getDisplay().setTextSize(3);

    char timeStr[10];
    sprintf(timeStr, "%02d:%02d", hour, minute);

    // Center time horizontally in left panel
    int centerX = DISPLAY_LEFT_HALF / 2;
    displayManager->drawCenteredText(timeStr, centerX, 200);

    displayManager->getDisplay().setTextSize(1); // Reset to normal
}

void DisplayClock::drawDate(const String &dayOfWeek, const String &date)
{
    displayManager->getDisplay().setFont(&FreeMonoBold24pt7b);
    displayManager->getDisplay().setTextColor(GxEPD_BLACK);
    displayManager->getDisplay().setTextSize(1);

    // Center day of week and date below time
    int centerX = DISPLAY_LEFT_HALF / 2;
    displayManager->drawCenteredText(dayOfWeek.c_str(), centerX, 250);
    displayManager->drawCenteredText(date.c_str(), centerX, 300);
}
