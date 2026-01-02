# Copilot Instructions for TRMNL View

## Project Overview

ESP32-C3 firmware for TRMNL View - a battery-efficient e-ink weather & clock display using a Waveshare 7.5" screen.

## Architecture

- **Microcontroller**: ESP32-C3 (built-in WiFi, low power)
- **Display**: Waveshare 7.5" e-ink (800x480 pixels), updated via GxEPD2/bb_epaper
- **Weather Data**: Open-Meteo API (free, no key required)
- **Power**: Battery with deep sleep optimization
- **Updates**: Time every 1 minute (partial screen), Weather every 30 minutes (full refresh)

## Key Components

- `src/main.cpp` - Main firmware loop with deep sleep logic
- `src/display.cpp` - E-ink rendering and partial updates
- `src/network.cpp` - WiFi and weather API integration
- `src/config.h` - Configuration (WiFi credentials, coordinates, pins)

## Display Layout

Left half: Clock (large time + day/date)
Right half: Weather (current temp + 6-hour forecast + 4-day forecast)

## Build & Flash

```bash
pio run                    # Build
pio run -t upload          # Flash to device (OTA or serial)
pio device monitor         # Serial output
```

## OTA Firmware Updates

Set device IP in `platformio.ini`:

```
upload_port = 192.168.5.99
```

Then upload wirelessly.

## Power Optimization

- WiFi disabled between updates
- Partial screen refreshes for time (no flash)
- Deep sleep with 1-minute timer wakeup
- Expected battery life: > 48 hours on typical battery

## Configuration

Edit `src/config.h`:

- WiFi SSID/password
- Latitude/longitude for weather
- Timezone
- GPIO pins for display

## Weather API

Uses Open-Meteo free API:

- WMO weather codes for conditions
- Hourly forecast (next 6 hours)
- Daily forecast (next 4 days)
- No authentication required

## Debugging

Enable debug flags in `config.h`:

```cpp
#define DEBUG_WEATHER_API 1
#define DEBUG_TIME_SYNC 1
#define DEBUG_DISPLAY 1
```

Set `DEBUG_NO_SLEEP=1` to disable deep sleep during development.
