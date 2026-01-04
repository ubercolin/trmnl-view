#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
#define WIFI_SSID "house"
#define WIFI_PASSWORD "superJ4ckson"

// Time synchronization
#define NTP_SERVER "pool.ntp.org"
#define TZ_INFO "PST8PDT,M3.2.0,M11.1.0" // Adjust to your timezone

// Weather API (using Open-Meteo free API - no key required)
#define WEATHER_API_URL "https://api.open-meteo.com/v1/forecast"
#define WEATHER_LATITUDE "45.5152" // Portland, OR
#define WEATHER_LONGITUDE "-122.6784"
#define WEATHER_UPDATE_INTERVAL 30 * 60 // 30 minutes in seconds

// Time display update
#define CLOCK_UPDATE_INTERVAL 60 // 1 minute in seconds

// Display configuration
#define DISPLAY_WIDTH 800
#define DISPLAY_HEIGHT 480
#define DISPLAY_LEFT_HALF 400  // Left half for clock
#define DISPLAY_RIGHT_HALF 400 // Right half for weather

// Pin configuration (Waveshare e-ink for ESP32-C3)
// Match TRMNL OG hardware
#define PIN_CLK 7     // EPD_SCK
#define PIN_MOSI 8    // EPD_MOSI
#define PIN_MISO 9    // Not used by e-ink but needed for SPI init
#define PIN_CS 6      // EPD_CS
#define PIN_DC 5      // EPD_DC
#define PIN_RST 10    // EPD_RST
#define PIN_BUSY 4    // EPD_BUSY
#define PIN_BATTERY 3 // Battery ADC pin

// OTA Configuration
#define OTA_ENABLED 1
#define OTA_WAIT_SECONDS 30

// Battery configuration
#define BATTERY_SAVE_MODE 1       // Enable deep sleep
#define WAKEUP_INTERVAL_MINUTES 1 // Wake up every minute for time updates
#define WAKEUP_INTERVAL_SECONDS (WAKEUP_INTERVAL_MINUTES * 60)

// Debug options (set to 1 to enable)
#define DEBUG_TIME_SYNC 0
#define DEBUG_WEATHER_API 0
#define DEBUG_DISPLAY 0
#define DEBUG_NO_SLEEP 0 // Set to 1 to disable deep sleep (use delay) for monitoring

#endif // CONFIG_H
