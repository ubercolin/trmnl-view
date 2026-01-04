#include "display_clock.h"
#include "display.h"

DisplayClock::DisplayClock(DisplayManager *displayManager) : displayManager(displayManager)
{
}

void DisplayClock::draw(int x, int hour, int minute, int second, const String &dayOfWeek, const String &date)
{
    // Very large time display (roughly 90px tall)
    displayManager->getDisplay().setFont(&FreeMonoBold24pt7b);
    displayManager->getDisplay().setTextColor(GxEPD_BLACK);

    char timeStr[10];
    sprintf(timeStr, "%02d:%02d", hour, minute);
    displayManager->getDisplay().setCursor(x + 30, 200); // Centered vertically
    displayManager->getDisplay().setTextSize(2);         // Scale up 2x for large display (~90px)
    displayManager->getDisplay().println(timeStr);
    displayManager->getDisplay().setTextSize(1); // Reset to normal

    // Day of week and date below time
    displayManager->getDisplay().setCursor(x + 30, 250);
    displayManager->getDisplay().println(dayOfWeek);

    displayManager->getDisplay().setCursor(x + 30, 300);
    displayManager->getDisplay().println(date);
}
