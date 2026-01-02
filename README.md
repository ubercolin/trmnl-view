# TRMNL View - ESP32 E-Ink Weather & Clock Display

A battery-efficient ESP32-C3 application that displays the current time on the left half of a 7.5" Waveshare e-ink screen and weather forecast on the right half.

## Features

- **Time Display**: Large, easy-to-read time on left half with day and date
- **Weather Display**:
  - Current temperature in large font
  - 6-hour hourly forecast with icons and temperatures
  - 4-day forecast with high/low temperatures and conditions
- **Battery Efficient**:
  - Deep sleep between updates
  - Partial screen refresh for time updates (no full refresh every minute)
  - WiFi only enabled for weather updates (every 30 minutes)
- **OTA Firmware Updates**: Wireless firmware updates in development
- **Open-Meteo API**: Uses free, no-key-required weather API

## Hardware

- **Microcontroller**: ESP32-C3 DevKit
- **Display**: Waveshare 7.5" e-ink (800x480 pixels)
- **Power**: Battery powered with deep sleep optimization

## Pin Configuration

```
ESP32-C3 → Waveshare E-Ink Display
CLK (10)  → Clock
MOSI (7)  → MOSI
MISO (6)  → MISO
CS (8)    → Chip Select
DC (20)   → Data/Command
RST (21)  → Reset
BUSY (19) → Busy
```

## Configuration

Edit `src/config.h` with your settings:

```cpp
#define WIFI_SSID "your-ssid"
#define WIFI_PASSWORD "your-password"
#define WEATHER_LATITUDE "37.7749"      // Your latitude
#define WEATHER_LONGITUDE "-122.4194"   // Your longitude
#define TZ_INFO "PST8PDT,M3.2.0,M11.1.0" // Your timezone
```

## Build & Flash

```bash
# Build the project
pio run

# Flash to device
pio run -t upload

# Monitor serial output
pio device monitor
```

## OTA Updates (Development)

1. Set the device's IP in `platformio.ini`:

   ```
   upload_port = 192.168.5.99
   ```

2. Upload firmware over-the-air:
   ```bash
   pio run -t upload --environment esp32c3
   ```

## Power Consumption

- **Active Mode** (WiFi on): ~120mA
- **Display Update**: ~50mA for 2-3 seconds
- **Deep Sleep**: ~10µA
- **Typical 24-hour usage**: < 100mAh with 30-minute weather updates

## Display Layout

```
┌─────────────────────┬─────────────────────┐
│                     │                     │
│    TIME             │   CURRENT TEMP      │
│   HH:MM             │      72°F           │
│                     │    Sunny            │
│  Day, Date          │                     │
│                     │  6-Hour Forecast:   │
│                     │  🌞70° 🌞68° ...    │
│                     │                     │
│                     │  4-Day Forecast:    │
│                     │  Mon 75°/60°        │
│                     │  Tue 68°/55°        │
│                     │  Wed 72°/58°        │
│                     │  Thu 70°/57°        │
└─────────────────────┴─────────────────────┘
```

## Development Notes

### Display Library

Uses `bb_epaper` (via GxEPD2) for efficient rendering:

- Supports partial screen updates
- No full-screen flashes during time updates
- Low power consumption

### Weather API

Uses [Open-Meteo](https://open-meteo.com/) for weather data:

- No API key required
- Free for non-commercial use
- Comprehensive hourly and daily forecasts
- WMO weather codes for condition mapping

### Sleep Strategy

1. **Time Update** (1 minute interval): Partial screen refresh, stays in light sleep
2. **Weather Update** (30 minute interval): Full screen refresh, WiFi reconnect
3. **Deep Sleep**: Between updates to minimize battery drain

## Troubleshooting

### Weather Not Updating

- Check WiFi credentials in `config.h`
- Verify latitude/longitude are correct
- Check internet connectivity

### Display Not Showing

- Verify GPIO pins in `config.h`
- Check SPI connections
- Try `DEBUG_DISPLAY=1` in `config.h`

### OTA Not Working

- Ensure device is on same network as development machine
- Set correct IP in `platformio.ini`
- Check firewall allows UDP port 3232

## Future Enhancements

- [ ] Weather icons (not just text)
- [ ] Sunrise/sunset times
- [ ] Air quality index
- [ ] Multiple location support
- [ ] Custom display layouts
- [ ] Battery voltage monitoring

## License

MIT License - See LICENSE file for details
