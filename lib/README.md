# Libraries Directory

This directory will contain local library copies if needed.

## Library Dependencies

The project uses the following external libraries (managed via `platformio.ini`):

### Graphics & Display
- **LovyanGFX** (v1.1.12+)
  - Handles GC9A01 display driver
  - Provides touch panel support
  - Optimized for ESP32-S3
  - GitHub: https://github.com/lovyan03/LovyanGFX

### JSON Processing
- **ArduinoJson** (v7.0+)
  - Parse OpenSky API responses
  - Lightweight and memory-efficient
  - GitHub: https://github.com/bblanchon/ArduinoJson

### Networking
- **HTTPClient** (built-in with ESP32 Arduino Core)
  - HTTP/HTTPS requests to OpenSky API
  - Included with esp32 board package

## Manual Library Installation (if needed)

If PlatformIO auto-install fails:

1. Download LovyanGFX from GitHub
2. Place in `lib/LovyanGFX/`
3. Do the same for ArduinoJson

Most users won't need to do this - PlatformIO handles it automatically.
