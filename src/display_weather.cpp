#include "display_weather.h"
#include "display.h"
#include "config.h"
#include "digit_bitmaps.h"
#include <time.h>

DisplayWeather::DisplayWeather(DisplayManager *displayManager) : displayManager(displayManager)
{
}

void DisplayWeather::update(const WeatherData &weather)
{
    auto &display = displayManager->getDisplay();
    display.setPartialWindow(DISPLAY_LEFT_HALF, 0, DISPLAY_RIGHT_HALF, DISPLAY_HEIGHT);
    display.firstPage();
    do
    {
        display.fillRect(DISPLAY_LEFT_HALF, 0, DISPLAY_RIGHT_HALF, DISPLAY_HEIGHT, GxEPD_WHITE);
        draw(DISPLAY_LEFT_HALF, 400, weather);
    } while (display.nextPage());
}

void DisplayWeather::draw(int startX, int boxWidth, const WeatherData &weather)
{
    int startY = 30;

    drawCurrentTemperature(startX, boxWidth, startY, weather.currentTemp);
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
    displayManager->getDisplay().setCursor(startX + boxWidth - textWidth + 20, 460);
    displayManager->getDisplay().print(timeStr);
}

void DisplayWeather::drawCurrentTemperature(int startX, int boxWidth, int startY, float temp)
{
    // Large current temperature using bitmap digits with degree symbol
    char tempStr[20];
    sprintf(tempStr, "%.0f°", temp);

    // Calculate centered position for temperature (variable width depending on digits)
    // Each digit is 60px wide, estimate total width
    int numDigits = strlen(tempStr) - 1; // -1 to account for degree symbol being multi-byte UTF-8
    int totalWidth = numDigits * DIGIT_WIDTH;
    int centerX = startX + (boxWidth / 2) - (totalWidth / 2);

    // Draw temperature + degree symbol using bitmap digits
    displayManager->drawNumberBitmap(centerX, startY, tempStr);
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
        drawWeatherIcon(centerX, startY + 40, weather.hourly[i].condition);

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
        drawWeatherIcon(centerX, startY + 40, weather.daily[i].condition);

        // High / Low temps
        char tempStr[20];
        sprintf(tempStr, "%.0f/%.0f°", weather.daily[i].tempHigh, weather.daily[i].tempLow);
        displayManager->drawCenteredText(tempStr, centerX, startY + 80);
    }
}

void DisplayWeather::drawWeatherIcon(int x, int y, const String &condition)
{
    // Determine which bitmap to use based on condition
    const unsigned char *bitmap = nullptr;

    if (condition.indexOf("Clear") >= 0)
    {
        bitmap = sun_32x32;
    }
    else if (condition.indexOf("Cloudy") >= 0 || condition.indexOf("Overcast") >= 0)
    {
        bitmap = cloud_32x32;
    }
    else if (condition.indexOf("Foggy") >= 0)
    {
        bitmap = haze_32x32;
    }
    else if (condition.indexOf("Rain") >= 0)
    {
        bitmap = rain_32x32;
    }
    else if (condition.indexOf("Snow") >= 0)
    {
        bitmap = snow_32x32;
    }
    else if (condition.indexOf("Thunder") >= 0)
    {
        bitmap = lightning_bolt_32x32;
    }

    if (bitmap != nullptr)
    {
        displayManager->drawBitmapIcon(x, y, bitmap, 32);
    }
    else
    {
        // Unknown: question mark in a box
        auto &display = displayManager->getDisplay();
        display.drawRect(x - 8, y - 8, 16, 16, GxEPD_BLACK);
        display.setFont(&FreeSans9pt7b);
        display.setCursor(x - 2, y + 5);
        display.print("?");
    }
}
