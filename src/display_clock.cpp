#include "display_clock.h"
#include "display.h"

DisplayClock::DisplayClock(DisplayManager *displayManager) : displayManager(displayManager)
{
}

void DisplayClock::draw(int x, int boxWidth, int hour, int minute, int second, const String &dayOfWeek, const String &date)
{
    // Very large time display (roughly 90px tall)
    displayManager->getDisplay().setFont(&FreeMonoBold24pt7b);
    displayManager->getDisplay().setTextColor(GxEPD_BLACK);
    displayManager->getDisplay().setTextSize(2);

    char timeStr[10];
    sprintf(timeStr, "%02d:%02d", hour, minute);

    // Center time horizontally
    int centerX = x + (boxWidth / 2);
    displayManager->drawCenteredText(timeStr, centerX, 200);

    displayManager->getDisplay().setTextSize(1); // Reset to normal

    // Day of week and date below time
    displayManager->drawCenteredText(dayOfWeek.c_str(), centerX, 250);
    displayManager->drawCenteredText(date.c_str(), centerX, 300);
}
