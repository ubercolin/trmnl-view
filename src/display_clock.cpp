#include "display_clock.h"
#include "display.h"
#include <Fonts/FreeMonoBold24pt7b.h>
#include "config.h"
#include "digit_bitmaps.h"

DisplayClock::DisplayClock(DisplayManager *displayManager) : displayManager(displayManager)
{
}

void DisplayClock::updateFull(int hour, int minute, int second, int dayOfWeek, int month, int day, int year)
{
    auto &display = displayManager->getDisplay();
    display.setPartialWindow(0, 0, DISPLAY_LEFT_HALF, DISPLAY_HEIGHT);
    display.firstPage();
    do
    {
        display.fillRect(0, 0, DISPLAY_LEFT_HALF, DISPLAY_HEIGHT, GxEPD_WHITE);
        drawTime(hour, minute);
        drawDate(dayOfWeek, month, day, year);
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
    // Use custom bitmap rendering for crisp, pixelation-free display
    drawTimeBitmap(hour, minute);
}

void DisplayClock::drawDate(int dayOfWeek, int month, int day, int year)
{
    String dayOfWeekStr = getDayOfWeekName(dayOfWeek);
    String dateStr = getFormattedDate(month, day, year);

    displayManager->getDisplay().setFont(&FreeMonoBold24pt7b);
    displayManager->getDisplay().setTextColor(GxEPD_BLACK);
    displayManager->getDisplay().setTextSize(1);

    // Center day of week and date below time
    int centerX = DISPLAY_LEFT_HALF / 2;
    displayManager->drawCenteredText(dayOfWeekStr.c_str(), centerX, 250);
    displayManager->drawCenteredText(dateStr.c_str(), centerX, 300);
}

String DisplayClock::getDayOfWeekName(int dayOfWeekIndex)
{
    static const char *daysOfWeek[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    return String(daysOfWeek[dayOfWeekIndex % 7]);
}

String DisplayClock::getFormattedDate(int month, int day, int year)
{
    static const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%s %d, %d", months[month % 12], day, year);
    return String(buffer);
}

void DisplayClock::drawTimeBitmap(int hour, int minute)
{
    // Draw time using custom digit bitmaps for crisp rendering
    char timeStr[6];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", hour, minute);

    // Render centered in left half: x=50, y=80
    displayManager->drawNumberBitmap(30, 80, timeStr);
}
