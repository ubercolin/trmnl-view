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
    // Window from y=80 to y=230 covers just the time, not day/date below
    display.setPartialWindow(0, 80, DISPLAY_LEFT_HALF, 140);
    display.firstPage();
    do
    {
        display.fillRect(0, 80, DISPLAY_LEFT_HALF, 140, GxEPD_WHITE);

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
        int colX = startX + (i * colWidth);
        sprintf(timeStr, "%d%s", displayHour, ampm);
        display.setCursor(colX, startY + 200);
        display.print(timeStr);

        // Draw weather icon instead of text
        drawWeatherIcon(colX + 20, startY + 220, weather.hourly[i].condition);

        char tempStr[8];
        sprintf(tempStr, "%.0f°", weather.hourly[i].temp);
        display.setCursor(colX, startY + 260);
        display.print(tempStr);
    }

    // Daily forecast (next 4 days) - 4 evenly spaced columns
    int dayColWidth = 100; // 400px / 4 columns
    for (int i = 0; i < 4; i++)
    {
        int boxX = startX + (i * dayColWidth);

        // Day of week
        display.setCursor(boxX, startY + 310);
        display.print(weather.daily[i].day.c_str());

        // Draw weather icon instead of condition text
        drawWeatherIcon(boxX + 40, startY + 330, weather.daily[i].condition);

        // High / Low temps
        char tempStr[20];
        sprintf(tempStr, "%.0f/%.0f°", weather.daily[i].tempHigh, weather.daily[i].tempLow);
        display.setCursor(boxX, startY + 370);
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

void DisplayManager::drawBitmapIcon(int x, int y, const unsigned char *bitmap, int size)
{
    // Draw a monochrome bitmap icon centered at (x, y)
    // bitmap: pointer to PROGMEM bitmap array
    // size: 32 for 32x32 bitmap

    int bytes_per_row = size / 8;
    int offset_x = size / 2; // Center horizontally
    int offset_y = size / 2; // Center vertically

    for (int row = 0; row < size; row++)
    {
        for (int col = 0; col < bytes_per_row; col++)
        {
            byte b = pgm_read_byte(bitmap + row * bytes_per_row + col);

            for (int bit = 0; bit < 8; bit++)
            {
                if ((b & (0x80 >> bit)) != 0) // Black pixel
                {
                    int pixel_x = x - offset_x + col * 8 + bit;
                    int pixel_y = y - offset_y + row;
                    display.drawPixel(pixel_x, pixel_y, GxEPD_BLACK);
                }
            }
        }
    }
}

void DisplayManager::drawWeatherIcon(int x, int y, const String &condition)
{
    // Draw weather icons using bitmaps (32x32)

    if (condition.indexOf("Clear") >= 0)
    {
        drawBitmapIcon(x, y, sun_32x32, 32);
    }
    else if (condition.indexOf("Cloudy") >= 0 || condition.indexOf("Overcast") >= 0)
    {
        drawBitmapIcon(x, y, cloud_32x32, 32);
    }
    else if (condition.indexOf("Foggy") >= 0)
    {
        drawBitmapIcon(x, y, haze_32x32, 32);
    }
    else if (condition.indexOf("Rain") >= 0)
    {
        drawBitmapIcon(x, y, rain_32x32, 32);
    }
    else if (condition.indexOf("Snow") >= 0)
    {
        drawBitmapIcon(x, y, snow_32x32, 32);
    }
    else if (condition.indexOf("Thunder") >= 0)
    {
        drawBitmapIcon(x, y, lightning_bolt_32x32, 32);
    }
    else
    {
        // Unknown: question mark in a box
        display.drawRect(x - 8, y - 8, 16, 16, GxEPD_BLACK);
        display.setFont(&FreeSans9pt7b);
        display.setCursor(x - 2, y + 5);
        display.print("?");
    }
}
