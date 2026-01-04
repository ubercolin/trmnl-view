#include "display.h"
#include "config.h"
#include <SPI.h>
#include <time.h>

DisplayManager::DisplayManager() : display(GxEPD2_750_T7(PIN_CS, PIN_DC, PIN_RST, PIN_BUSY)), clockDisplay(this), weatherDisplay(this)
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

TextBounds DisplayManager::drawCenteredText(const char *text, int16_t centerX, int16_t y)
{
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

    // Calculate x position to center the text
    int16_t x = centerX - (w / 2);

    display.setCursor(x, y);
    display.print(text);

    // Return the bounding box of the drawn text
    return {x, (int16_t)(y - h), (int16_t)w, (int16_t)h};
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
    clockDisplay.draw(0, DISPLAY_LEFT_HALF, hour, minute, second, dayOfWeek, date);
}

void DisplayManager::drawWeatherSection(const WeatherData &weather)
{
    weatherDisplay.draw(DISPLAY_LEFT_HALF, DISPLAY_RIGHT_HALF, weather);
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
                    int pixel_x = x - offset_x + 1 + col * 8 + bit;
                    int pixel_y = y - offset_y + row;
                    display.drawPixel(pixel_x, pixel_y, GxEPD_BLACK);
                }
            }
        }
    }
}

void DisplayManager::drawWeatherIcon(int x, int y, const String &condition)
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
        drawBitmapIcon(x, y, bitmap, 32);
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
