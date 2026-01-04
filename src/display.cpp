#include "display.h"
#include "config.h"
#include "digit_bitmaps.h"
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

void DisplayManager::updateClock(int hour, int minute, int second, int dayOfWeek, int month, int day, int year)
{
    clockDisplay.updateFull(hour, minute, second, dayOfWeek, month, day, year);
}

void DisplayManager::partialUpdateClock(int hour, int minute, int second)
{
    clockDisplay.updatePartial(hour, minute, second);
}

void DisplayManager::partialUpdateDate(int dayOfWeek, int month, int day, int year)
{
    // Update only the date area (top of left half)
    display.setPartialWindow(0, 0, DISPLAY_LEFT_HALF, 80);
    display.firstPage();
    do
    {
        display.fillRect(0, 0, DISPLAY_LEFT_HALF, 80, GxEPD_WHITE);
        clockDisplay.drawDate(dayOfWeek, month, day, year);
    } while (display.nextPage());
}

void DisplayManager::updateWeather(const WeatherData &weather)
{
    weatherDisplay.update(weather);
}

void DisplayManager::deepSleep(uint32_t sleepSeconds)
{
    display.powerOff();
    // Configure timer wake-up
    esp_sleep_enable_timer_wakeup(sleepSeconds * 1000000ULL);
    esp_deep_sleep_start();
}

void DisplayManager::wakeup()
{
    // Light initialization after deep sleep - just reinit SPI and display controller
    // Does NOT clear the screen (preserves existing content)
    SPI.begin(PIN_CLK, PIN_MISO, PIN_MOSI, PIN_CS);
    display.init(115200, false); // false = don't reset, preserves display content
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

void DisplayManager::drawNumberBitmap(int x, int y, const char *numberString)
{
    // Draw a string of numbers using bitmap digits
    // x, y: top-left position for first digit
    // numberString: string containing digits, colon, and degree symbol (e.g., "72°")

    int currentX = x;

    // Draw each character
    for (size_t i = 0; numberString[i] != '\0'; i++)
    {
        unsigned char c = (unsigned char)numberString[i];

        // Handle UTF-8 degree symbol (0xC2 0xB0) - skip the first byte
        if (c == 0xC2 && (unsigned char)numberString[i + 1] == 0xB0)
        {
            i++;      // Skip next byte
            c = 0xB0; // Use the second byte for lookup
        }

        const unsigned char *bitmap = getDigitBitmap((char)c);

        // Draw bitmap row by row
        int bytes_per_row = (DIGIT_WIDTH + 7) / 8;
        for (int row = 0; row < DIGIT_HEIGHT; row++)
        {
            for (int col = 0; col < bytes_per_row; col++)
            {
                byte b = pgm_read_byte(bitmap + row * bytes_per_row + col);

                // Process each bit in the byte
                for (int bit = 0; bit < 8; bit++)
                {
                    if ((b & (0x80 >> bit)) != 0) // Black pixel
                    {
                        int pixel_x = currentX + col * 8 + bit;
                        int pixel_y = y + row;

                        // Bounds check
                        if (pixel_x >= 0 && pixel_x < 800 &&
                            pixel_y >= 0 && pixel_y < 480)
                        {
                            display.drawPixel(pixel_x, pixel_y, GxEPD_BLACK);
                        }
                    }
                }
            }
        }

        // Move to next digit position
        currentX += DIGIT_WIDTH;
    }
}
