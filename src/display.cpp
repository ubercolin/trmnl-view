#include "display.h"
#include "config.h"
#include <SPI.h>
#include <time.h>

DisplayManager::DisplayManager() : display(GxEPD2_750_T7(PIN_CS, PIN_DC, PIN_RST, PIN_BUSY))
{
}

void DisplayManager::init()
{
    // Initialize SPI with custom pins
    SPI.begin(PIN_CLK, PIN_MISO, PIN_MOSI, PIN_CS);

    display.init(115200);
    display.setRotation(0);
    display.fillScreen(GxEPD_WHITE);
    // display.setFont(&FreeMonoBold24pt7b);
    // display.setTextColor(GxEPD_BLACK);
    // display.setCursor(DISPLAY_WIDTH / 2 - 100, DISPLAY_HEIGHT / 2 - 20);
    // display.println("Initializing...");
    display.display(true);
}

void DisplayManager::showError(const String &errorMessage)
{
    display.fillScreen(GxEPD_WHITE);
    display.setFont(&FreeMonoBold12pt7b);
    display.setTextColor(GxEPD_BLACK);

    display.setCursor(50, 200);
    display.print("ERROR: ");
    display.println(errorMessage);

    display.display(true);
}

void DisplayManager::updateClock(int hour, int minute, int second, const String &dayOfWeek, const String &date)
{
    display.setPartialWindow(0, 0, DISPLAY_LEFT_HALF, DISPLAY_HEIGHT);
    display.firstPage();
    do
    {
        display.fillRect(0, 0, DISPLAY_LEFT_HALF, DISPLAY_HEIGHT, GxEPD_WHITE);
        drawClockSection(hour, minute, second, dayOfWeek, date);
    } while (display.nextPage());

    lastDisplayedHour = hour;
    lastDisplayedMinute = minute;
}

void DisplayManager::partialUpdateClock(int hour, int minute, int second)
{
    // Only update if time has changed
    if (hour == lastDisplayedHour && minute == lastDisplayedMinute)
    {
        return;
    }

    // Update only the time area (partial refresh for efficiency)
    display.setPartialWindow(0, 80, DISPLAY_LEFT_HALF, 230);
    display.firstPage();
    do
    {
        display.fillRect(0, 80, DISPLAY_LEFT_HALF, 230, GxEPD_WHITE);

        display.setFont(&FreeMonoBold24pt7b);
        display.setTextColor(GxEPD_BLACK);

        char timeStr[10];
        sprintf(timeStr, "%02d:%02d", hour, minute);
        display.setCursor(30, 200);
        display.setTextSize(2);
        display.println(timeStr);
        display.setTextSize(1);
    } while (display.nextPage());

    lastDisplayedHour = hour;
    lastDisplayedMinute = minute;
}

void DisplayManager::updateWeather(const WeatherData &weather)
{
    display.setPartialWindow(DISPLAY_LEFT_HALF, 0, DISPLAY_RIGHT_HALF, DISPLAY_HEIGHT);
    display.firstPage();
    do
    {
        display.fillRect(DISPLAY_LEFT_HALF, 0, DISPLAY_RIGHT_HALF, DISPLAY_HEIGHT, GxEPD_WHITE);
        drawWeatherSection(weather);
    } while (display.nextPage());
}

void DisplayManager::drawClockSection(int hour, int minute, int second, const String &dayOfWeek, const String &date)
{
    // Very large time display (roughly 90px tall)
    display.setFont(&FreeMonoBold24pt7b);
    display.setTextColor(GxEPD_BLACK);

    char timeStr[10];
    sprintf(timeStr, "%02d:%02d", hour, minute);
    display.setCursor(30, 200); // Centered vertically
    display.setTextSize(2);     // Scale up 2x for large display (~90px)
    display.println(timeStr);
    display.setTextSize(1); // Reset to normal

    // Day of week and date below time
    // display.setFont(&FreeSans24pt7b);
    display.setCursor(30, 250);
    display.println(dayOfWeek);

    display.setCursor(30, 300);
    display.println(date);
}

void DisplayManager::drawWeatherSection(const WeatherData &weather)
{
    int startX = DISPLAY_LEFT_HALF + 20;
    int startY = 30;

    // Large current temperature (2x size, centered, no F unit)
    display.setFont(&FreeMonoBold24pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.setTextSize(2);

    char tempStr[20];
    sprintf(tempStr, "%.0f°", weather.currentTemp);
    // Center horizontally in right panel (roughly 200px from start for center)
    display.setCursor(startX + 60, startY + 80);
    display.println(tempStr);

    display.setFont(&FreeSans12pt7b);
    display.setTextSize(1);

    // Hourly forecast (next 6 hours) - time row
    display.setCursor(startX, startY + 140);
    int colWidth = 60;
    for (int i = 0; i < 6; i++)
    {
        char timeStr[8];
        int hour = weather.hourly[i].hour;
        int displayHour = (hour % 12 == 0) ? 12 : (hour % 12);
        const char *ampm = (hour < 12) ? "a" : "p";
        sprintf(timeStr, "%d%s", displayHour, ampm);
        display.setCursor(startX + (i * colWidth), startY + 140);
        display.print(timeStr);
    }

    // Temperature row
    display.setCursor(startX, startY + 170);
    for (int i = 0; i < 6; i++)
    {
        char tempStr[8];
        sprintf(tempStr, "%.0f°", weather.hourly[i].temp);
        display.setCursor(startX + (i * colWidth), startY + 170);
        display.print(tempStr);
    }

    // Daily forecast (next 4 days) - 4 evenly spaced columns
    int dayColWidth = 100; // 400px / 4 columns
    for (int i = 0; i < 4; i++)
    {
        int boxX = startX + (i * dayColWidth);

        // Day of week
        display.setCursor(boxX, startY + 210);
        display.print(weather.daily[i].day.c_str());

        // Condition
        display.setCursor(boxX, startY + 240);
        display.print(weather.daily[i].condition.c_str());

        // High / Low temps
        char tempStr[20];
        sprintf(tempStr, "%.0f/%.0f°", weather.daily[i].tempHigh, weather.daily[i].tempLow);
        display.setCursor(boxX, startY + 270);
        display.print(tempStr);
    }

    // Last updated timestamp at bottom right
    if (weather.lastUpdated > 0)
    {
        time_t updateTime = weather.lastUpdated;
        struct tm timeinfo;
        localtime_r(&updateTime, &timeinfo);

        display.setFont(&FreeSans9pt7b);
        display.setTextSize(1);

        char timeStr[20];
        strftime(timeStr, sizeof(timeStr), "%b %d %H:%M", &timeinfo);

        int textWidth = 150;
        display.setCursor(DISPLAY_LEFT_HALF + DISPLAY_RIGHT_HALF - textWidth, DISPLAY_HEIGHT - 20);
        display.print(timeStr);
    }
}

void DisplayManager::drawForecastRow(int y, const WeatherData &weather, bool isHourly)
{
    // Forecast display simplified - not used anymore
}

void DisplayManager::drawTemperatureBox(int x, int y, int w, int h, float temp, const String &label)
{
    // Draw a box with temperature and label
    display.drawRect(x, y, w, h, GxEPD_BLACK);

    display.setFont(&FreeSans12pt7b);
    char tempStr[10];
    sprintf(tempStr, "%.0f°", temp);
    display.setCursor(x + 5, y + 20);
    display.println(tempStr);

    display.setCursor(x + 5, y + h - 5);
    display.println(label);
}

void DisplayManager::deepSleep()
{
    display.powerOff();
    // Configure timer wake-up
    esp_sleep_enable_timer_wakeup(WAKEUP_INTERVAL_SECONDS * 1000000ULL);
    esp_deep_sleep_start();
}

void DisplayManager::wakeup()
{
    // Resume from deep sleep
    display.init(115200);
}

void DisplayManager::updateBattery(float batteryPercent)
{
    currentBattery = batteryPercent;

    // Update only battery area on left panel (lower left corner)
    display.setPartialWindow(0, 400, 200, 80);
    display.firstPage();
    do
    {
        display.fillRect(0, 400, 200, 80, GxEPD_WHITE);

        display.setFont(&FreeSans9pt7b);
        display.setTextColor(GxEPD_BLACK);
        display.setTextSize(1);

        char battStr[20];
        sprintf(battStr, "Battery: %.0f%%", batteryPercent);
        display.setCursor(10, DISPLAY_HEIGHT - 20);
        display.println(battStr);
    } while (display.nextPage());
}
