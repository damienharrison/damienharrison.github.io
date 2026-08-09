# ESP32-S3 Flight Radar Application

A real-time aviation radar application for the ESP32-S3 microcontroller with a 1.28" round IPS LCD display. Displays live flight data via OpenSky API with interactive radar visualization.

## Features

### Core Functionality
- ✅ **Live Flight Tracking**: Real-time ADS-B data from OpenSky Network API
- ✅ **Radar Visualization**: Circular radar display showing aircraft in your area
- ✅ **Interactive UI**: Tap to view aircraft details
- ✅ **Zoom Control**: 5 zoom levels (10km - 250km radius)
- ✅ **Pan Support**: Move around the radar view
- ✅ **Postcode Lookup**: Enter postcode to center on location
- ✅ **Aircraft Details**: Callsign, ICAO, altitude, speed, heading

### Display Features
- Color-coded altitude indicators (green/yellow/red)
- Aircraft heading indicators (triangle pointing direction)
- Radar grid with distance markers
- Real-time aircraft count
- Zoom level and radius display
- WiFi status indicator

### UI Screens
1. **Radar View** (default): Main radar visualization with interactive controls
2. **Aircraft Details**: Full information on selected aircraft
3. **Map View**: Map representation and statistics
4. **Settings**: Configuration options

## Hardware Requirements

### Essential
- **ESP32-S3 Development Board** (with PSRAM recommended)
- **1.28" GC9A01 Round IPS LCD Display**
  - Resolution: 240x240 pixels
  - Interface: SPI
  - Touch: Capacitive or resistive (CST816S, GT911, FT6236, or similar)

### Wiring

```
GC9A01 Display:
  VCC     → 3.3V
  GND     → GND
  CLK     → GPIO 12 (SPI2_SCLK)
  MOSI    → GPIO 11 (SPI2_MOSI)
  MISO    → GPIO 13 (SPI2_MISO) - optional if display doesn't need readback
  CS      → GPIO 10
  DC      → GPIO 8
  RST     → GPIO 9
  BL      → GPIO 46 (backlight PWM)

Touch Controller (CST816S example):
  VCC     → 3.3V
  GND     → GND
  SDA     → GPIO 3 (I2C_SDA)
  SCL     → GPIO 46 (I2C_SCL) - adjust if conflicts with BL
  INT     → GPIO 32
  RST     → GPIO 33 (optional, can tie to VCC)
```

**Note**: Pin assignments can be modified in `include/config.h` and `src/display_driver.cpp`

### Software Requirements
- **PlatformIO** (recommended) or Arduino IDE
- **ESP-IDF 5.0+** or Arduino-ESP32 2.0+
- **Dependencies** (auto-installed via platformio.ini):
  - LovyanGFX 1.1.12+
  - ArduinoJson 7.0+
  - HTTPClient (included with ESP32 core)

## Setup Instructions

### 1. Initial Hardware Setup

1. Connect your GC9A01 display and touch controller to the ESP32-S3 following the wiring diagram above
2. Verify all power connections (3.3V, GND)
3. Double-check SPI and I2C pin connections

### 2. Software Installation

#### Using PlatformIO (Recommended)

```bash
# Install PlatformIO (if not already installed)
pip install platformio

# Clone/open this project
cd ~/path/to/esp32-flight-radar

# Install dependencies and build
platformio run --target build --environment esp32-s3-devkit

# Upload to board
platformio run --target upload --environment esp32-s3-devkit

# Monitor serial output
platformio device monitor -b 115200
```

#### Using Arduino IDE

1. Install ESP32 Board Support: `Tools → Board Manager → Search "esp32" → Install`
2. Install required libraries:
   - Sketch → Include Library → Manage Libraries
   - Search and install: **LovyanGFX** by LOVYAN32
   - Search and install: **ArduinoJson** by Benoit Blanchon
3. Copy all files from `src/`, `include/` to new Arduino sketch
4. Configure board: `Tools → Board → esp32 → esp32-s3-devkit`
5. Adjust pin definitions in `config.h` if needed
6. Upload and monitor serial output

### 3. Configuration

#### WiFi Credentials
Edit `include/config.h`:
```cpp
#define WIFI_SSID "YOUR_SSID"
#define WIFI_PASS "YOUR_PASSWORD"
```

#### Default Location
In `src/main.cpp`, change the default location:
```cpp
// Default to London - change coordinates
location_service.setManualLocation(51.5074, -0.1278);
```

#### Display Pins
If using different pins, update in `config.h` and `src/display_driver.cpp`:
```cpp
cfg.pin_cs = 10;      // Chip Select
cfg.pin_mosi = 11;    // Data
cfg.pin_sclk = 12;    // Clock
cfg.pin_dc = 8;       // Data/Command
cfg.pin_rst = 9;      // Reset
```

#### Touch Controller
The touch support is framework-dependent. For your specific touch IC:
- **CST816S**: I2C address 0x5D
- **GT911**: I2C address 0x5D or 0x14
- **FT6236**: I2C address 0x38

Uncomment and configure the appropriate touch driver in LovyanGFX.

### 4. First Run

1. Open Serial Monitor (115200 baud)
2. Power on the ESP32-S3
3. Watch initialization messages:
   ```
   === ESP32-S3 Flight Radar ===
   Initializing...
   1. Initializing display...
   2. Initializing touch...
   3. Connecting to WiFi...
   4. Setting default location...
   5. Initialization complete!
   ```
4. The radar should display with aircraft appearing in your area

## Operation

### Radar Screen
- **Default View**: Circular radar centered on your location
- **Ring Spacing**: Each circle = 1/4 of the selected radius
- **Aircraft Symbols**: Triangles pointing in flight direction
- **Colors**: 
  - 🟢 Green (altitude < 10,000 ft)
  - 🟡 Yellow (10,000 - 25,000 ft)
  - 🔴 Red (altitude > 25,000 ft)
  - 🟨 Yellow + Circle (selected aircraft)

### Controls

#### Touch Screen
- **Single Tap**: Select aircraft to view details
- **Double Tap**: Zoom in one level
- **Drag**: Pan the radar view (if not on an aircraft)

#### Serial Commands (for testing/setup)
```
postcode:SW1A1AA          # Set location by UK postcode
radius:100                # Change radar radius (km)
status                    # Print current status
```

### Zoom Levels
| Level | Radius | Typical Use |
|-------|--------|-------------|
| 0 | 250 km | Regional overview |
| 1 | 100 km | Large area |
| 2 | 50 km | Standard (default) |
| 3 | 25 km | Local area |
| 4 | 10 km | Detailed view |

## Settings System

The application includes a **comprehensive settings system** that eliminates all hardcoded configuration. All parameters can be adjusted directly from the device UI:

### Accessing Settings
1. **From Radar Screen**: Tap anywhere (not on aircraft) to open Settings menu
2. **Via Serial**: Use serial commands for quick adjustments
3. **Persistent Storage**: All changes saved to SPIFFS and restored on power-up

### Main Categories
- **Radar Settings**: Radius (5-250 km), center location
- **Display Settings**: Theme (light/dark), brightness (0-255), grid opacity
- **Trail Settings**: Enable/disable, trail point count, max aircraft

### Example Serial Commands
```
settings:show             # Display all current settings
settings:reset            # Reset to factory defaults
settings:radar 100        # Set radius to 100 km
settings:theme dark       # Switch to dark theme
settings:trails on        # Enable aircraft trails
settings:brightness 128   # Set brightness to 50%
```

**📖 For detailed settings documentation**, see [SETTINGS_SYSTEM.md](SETTINGS_SYSTEM.md)

## Architecture

### Core Components

#### Display Driver (`display_driver.h/cpp`)
- LovyanGFX wrapper for GC9A01 display
- Radar rendering primitives
- Color management

#### Radar Engine (`radar_engine.h/cpp`)
- Aircraft projection (lat/lon → screen coordinates)
- Zoom and pan logic
- Aircraft sorting and selection

#### OpenSky API (`opensky_api.h/cpp`)
- WiFi connection management
- API data fetching and parsing
- Distance filtering

#### Touch Handler (`touch_handler.h/cpp`)
- Touch input detection
- Gesture recognition (tap, double-tap, drag)

#### Location Service (`location_service.h/cpp`)
- Postcode → coordinate lookup (via Nominatim/OSM)
- Location caching

#### UI Manager (`ui_manager.h/cpp`)
- Screen state management
- Event routing
- Display updates

## API Integration

### OpenSky API
- **Endpoint**: `https://opensky-network.org/api/states/all`
- **Rate Limit**: 1 request/sec (free tier)
- **Update Interval**: 5 seconds default
- **Data**: Real-time aircraft positions, altitude, speed, heading

**Note**: OpenSky's free API has rate limiting and may throttle requests. For higher update rates, consider:
- OpenSky's paid API tier
- ADS-B Exchange API
- Local rtl-sdr setup with dump1090

### Nominatim API (Postcode Lookup)
- **Endpoint**: `https://nominatim.openstreetmap.org/search`
- **Rate Limit**: 1 request/sec
- **Usage**: Postcode → latitude/longitude

## Performance Notes

### Memory Usage
- **RAM**: ~80-120 KB used (typical, varies with aircraft count)
- **SPIFFS**: ~20 KB available for future logging/caching
- **PSRAM**: Not currently used, but available for future optimization

### Update Rates
- **API Fetch**: Every 5 seconds (configurable)
- **Radar Display**: Every 3 seconds (configurable)
- **Touch Input**: Continuous polling
- **Serial Debug**: Every 10 seconds

### Optimization Tips
1. Use Settings menu to increase API update interval if WiFi bandwidth is limited
2. Reduce radar display interval for smoother animation (increases CPU load)
3. Adjust radar radius to limit aircraft tracking range
4. Reduce trail point count or max aircraft count to save memory
5. Use PSRAM-enabled build for larger aircraft databases

## Troubleshooting

### Display Not Showing
1. Check SPI pins and connections
2. Verify backlight pin is functional (GPIO 46)
3. Check LovyanGFX is properly configured for your display variant
4. Try different pin assignments if using non-standard wiring

### No Aircraft Appearing
1. Check WiFi connection (serial monitor should confirm)
2. Verify location is correct (change with `postcode:` command)
3. Check OpenSky API response (might be rate-limited)
4. Ensure default location has aircraft in range

### Touch Not Responding
1. Verify I2C connections (SDA/SCL)
2. Check touch controller address in config (0x5D vs 0x14)
3. Confirm touch INT pin is properly connected
4. Try calibrating touch (if your display supports it)

### WiFi Connection Issues
1. Use Settings → WiFi screen to configure WiFi credentials on device
2. Check if network requires WPA2/WPA3 (device supports both)
3. Try 2.4GHz network (5GHz may have connectivity issues on some boards)
4. Monitor signal strength with `status` or `wifi:status` serial command
5. Use `wifi:reset` to clear stored credentials and reconfigure

## Future Enhancements

- [ ] Touch calibration utility
- [ ] Aircraft route history/trail
- [ ] Alerts for specific aircraft
- [ ] Custom location bookmarks
- [ ] Battery level indicator
- [ ] Offline map support
- [ ] ADS-B receiver mode (with rtl-sdr)
- [ ] Aircraft photo/info display
- [ ] Speed/altitude graphs
- [ ] Multi-language support

## License

MIT License - See LICENSE file

## References

- [OpenSky Network API](https://openskynetwork.github.io/opensky-api/)
- [LovyanGFX Documentation](https://github.com/lovyan03/LovyanGFX)
- [ESP32-S3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)
- [GC9A01 Display Driver](https://www.datasheethub.com/GC9A01.html)

## Support & Contributing

For issues or suggestions:
1. Check troubleshooting section above
2. Review serial debug output
3. Open an issue with:
   - Hardware configuration
   - Error messages from serial monitor
   - Steps to reproduce

---

**Happy flying! 🛩️**
