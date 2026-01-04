#include "display_weather.h"
#include "display.h"
#include <time.h>

DisplayWeather::DisplayWeather(DisplayManager *displayManager) : displayManager(displayManager)
{
}

void DisplayWeather::draw(int startX, int boxWidth, const WeatherData &weather)
{
    int startY = 30;

    drawCurrentTemperature(startX, boxWidth, startY + 20, weather.currentTemp);
    drawHourly(startX, boxWidth, startY + 140, weather);
    drawDaily(startX, boxWidth, startY + 250, weather);

    // Last updated timestamp at bottom right
    if (weather.lastUpdated > 0)
    {
        drawLastUpdated(weather, startX, boxWidth);
    }
}

void DisplayWeather::drawLastUpdated(const WeatherData &weather, int startX, int boxWidth)
{

    time_t updateTime = weather.lastUpdated;
    struct tm timeinfo;
    localtime_r(&updateTime, &timeinfo);

    displayManager->getDisplay().setFont(&FreeSans9pt7b);
    displayManager->getDisplay().setTextSize(1);

    char timeStr[20];
    strftime(timeStr, sizeof(timeStr), "%b %d %H:%M", &timeinfo);

    int textWidth = 150;
    displayManager->getDisplay().setCursor(startX + boxWidth - textWidth, 460);
    displayManager->getDisplay().print(timeStr);
}

void DisplayWeather::drawCurrentTemperature(int startX, int boxWidth, int startY, float temp)
{
    // Large current temperature (3x size, centered)
    displayManager->getDisplay().setFont(&FreeMonoBold24pt7b);
    displayManager->getDisplay().setTextColor(GxEPD_BLACK);
    displayManager->getDisplay().setTextSize(3);

    char tempStr[20];
    sprintf(tempStr, "%.0f", temp);

    // Center horizontally
    int centerX = startX + (boxWidth / 2);
    TextBounds tempBounds = displayManager->drawCenteredText(tempStr, centerX, startY + 80);

    // Draw small "o" to upper right as degree symbol
    // Position it at the top-right of the temperature text
    displayManager->getDisplay().setTextSize(1);
    int degreeX = tempBounds.x + tempBounds.w + 10;
    int degreeY = tempBounds.y + 24;
    displayManager->getDisplay().setCursor(degreeX, degreeY);
    displayManager->getDisplay().print("o");

    displayManager->getDisplay().setFont(&FreeSans12pt7b);
    displayManager->getDisplay().setTextSize(1);
}

void DisplayWeather::drawHourly(int startX, int boxWidth, int startY, const WeatherData &weather)
{
    // Hourly forecast (next 5 hours)
    displayManager->getDisplay().setFont(&FreeSans12pt7b);
    displayManager->getDisplay().setTextSize(1);

    int colWidth = boxWidth / 5; // 5 columns across the box width
    for (int i = 0; i < 5; i++)
    {
        char timeStr[8];
        int hour = weather.hourly[i].hour;
        int displayHour = (hour % 12 == 0) ? 12 : (hour % 12);
        const char *ampm = (hour < 12) ? "a" : "p";
        int colX = startX + (i * colWidth);
        int centerX = colX + (colWidth / 2);

        sprintf(timeStr, "%d%s", displayHour, ampm);
        displayManager->drawCenteredText(timeStr, centerX, startY + 20);

        // Draw weather icon
        displayManager->drawWeatherIcon(centerX, startY + 40, weather.hourly[i].condition);

        char tempStr[8];
        sprintf(tempStr, "%.0f°", weather.hourly[i].temp);
        displayManager->drawCenteredText(tempStr, centerX, startY + 80);
    }
}

void DisplayWeather::drawDaily(int startX, int boxWidth, int startY, const WeatherData &weather)
{
    // Daily forecast (next 4 days) - 4 evenly spaced columns
    displayManager->getDisplay().setFont(&FreeSans12pt7b);
    displayManager->getDisplay().setTextSize(1);

    int dayColWidth = boxWidth / 4; // 4 columns across the box width
    for (int i = 0; i < 4; i++)
    {
        int boxX = startX + (i * dayColWidth);
        int centerX = boxX + (dayColWidth / 2);

        // Day of week
        displayManager->drawCenteredText(weather.daily[i].day.c_str(), centerX, startY + 20);

        // Draw weather icon
        displayManager->drawWeatherIcon(centerX, startY + 40, weather.daily[i].condition);

        // High / Low temps
        char tempStr[20];
        sprintf(tempStr, "%.0f/%.0f°", weather.daily[i].tempHigh, weather.daily[i].tempLow);
        displayManager->drawCenteredText(tempStr, centerX, startY + 80);
    }
}
