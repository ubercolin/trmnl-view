#include "display_weather.h"
#include "display.h"
#include <time.h>

DisplayWeather::DisplayWeather(DisplayManager *displayManager) : displayManager(displayManager)
{
}

void DisplayWeather::draw(int startX, const WeatherData &weather)
{
    int startY = 30;

    drawCurrentTemperature(startX, startY + 20, weather.currentTemp);
    drawHourly(startX, startY + 140, weather);
    drawDaily(startX, startY + 250, weather);

    // Last updated timestamp at bottom right
    if (weather.lastUpdated > 0)
    {
        drawLastUpdated(weather, startX);
    }
}

void DisplayWeather::drawLastUpdated(const WeatherData &weather, int startX)
{

    time_t updateTime = weather.lastUpdated;
    struct tm timeinfo;
    localtime_r(&updateTime, &timeinfo);

    displayManager->getDisplay().setFont(&FreeSans9pt7b);
    displayManager->getDisplay().setTextSize(1);

    char timeStr[20];
    strftime(timeStr, sizeof(timeStr), "%b %d %H:%M", &timeinfo);

    int textWidth = 150;
    displayManager->getDisplay().setCursor(startX + 400 - textWidth, 460);
    displayManager->getDisplay().print(timeStr);
}

void DisplayWeather::drawCurrentTemperature(int startX, int startY, float temp)
{
    // Large current temperature (3x size, centered, no F unit)
    displayManager->getDisplay().setFont(&FreeMonoBold24pt7b);
    displayManager->getDisplay().setTextColor(GxEPD_BLACK);
    displayManager->getDisplay().setTextSize(3);

    char tempStr[20];
    sprintf(tempStr, "%.0f", temp);
    // Center horizontally in right panel (roughly 200px from start for center)
    displayManager->getDisplay().setCursor(startX + 120, startY + 80);
    displayManager->getDisplay().println(tempStr);

    // Draw small "o" to upper right as degree symbol
    // displayManager->getDisplay().setFont(&FreeMonoBold18pt7b);
    displayManager->getDisplay().setTextSize(1);
    displayManager->getDisplay().setCursor(startX + 280, startY);
    displayManager->getDisplay().print("o");

    displayManager->getDisplay().setFont(&FreeSans12pt7b);
    displayManager->getDisplay().setTextSize(1);
}

void DisplayWeather::drawHourly(int startX, int startY, const WeatherData &weather)
{
    // Hourly forecast (next 5 hours)
    int colWidth = 80;
    for (int i = 0; i < 5; i++)
    {
        char timeStr[8];
        int hour = weather.hourly[i].hour;
        int displayHour = (hour % 12 == 0) ? 12 : (hour % 12);
        const char *ampm = (hour < 12) ? "a" : "p";
        int colX = startX + (i * colWidth);
        sprintf(timeStr, "%d%s", displayHour, ampm);
        displayManager->getDisplay().setCursor(colX + 25, startY + 20);
        displayManager->getDisplay().print(timeStr);

        // Draw weather icon
        displayManager->drawWeatherIcon(colX + (colWidth / 2), startY + 40, weather.hourly[i].condition);

        char tempStr[8];
        sprintf(tempStr, "%.0f°", weather.hourly[i].temp);
        displayManager->getDisplay().setCursor(colX + 25, startY + 80);
        displayManager->getDisplay().print(tempStr);
    }
}

void DisplayWeather::drawDaily(int startX, int startY, const WeatherData &weather)
{
    // Daily forecast (next 4 days) - 4 evenly spaced columns
    int dayColWidth = 100; // 400px / 4 columns
    for (int i = 0; i < 4; i++)
    {
        int boxX = startX + (i * dayColWidth);

        // Day of week
        displayManager->getDisplay().setCursor(boxX + 35, startY + 20);
        displayManager->getDisplay().print(weather.daily[i].day.c_str());

        // Draw weather icon
        displayManager->drawWeatherIcon(boxX + (dayColWidth / 2), startY + 40, weather.daily[i].condition);

        // High / Low temps
        char tempStr[20];
        sprintf(tempStr, "%.0f/%.0f°", weather.daily[i].tempHigh, weather.daily[i].tempLow);
        displayManager->getDisplay().setCursor(boxX + 20, startY + 80);
        displayManager->getDisplay().print(tempStr);
    }
}
