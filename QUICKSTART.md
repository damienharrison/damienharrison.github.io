# Quick Start Guide - ESP32-S3 Flight Radar

Get your flight radar running in 5 minutes!

## ⚡ Super Quick (if you know what you're doing)

1. **Wire it up** according to pins in `include/config.h`
2. **Edit WiFi**: `#define WIFI_SSID "YOUR_SSID"` in `include/config.h`
3. **Upload**: `platformio run --target upload`
4. **Done!** Open serial monitor to see it boot

## 📋 Step-by-Step Setup

### Step 1: Verify Hardware

- [ ] ESP32-S3 development board (with USB cable)
- [ ] 1.28" round GC9A01 LCD display with touch
- [ ] Jumper wires and breadboard (if needed)
- [ ] 3.3V power source available on board

### Step 2: Make Connections

**Display (GC9A01):**
```
VCC   → ESP32 3.3V
GND   → ESP32 GND
CLK   → ESP32 GPIO 12
MOSI  → ESP32 GPIO 11
MISO  → ESP32 GPIO 13
CS    → ESP32 GPIO 10
DC    → ESP32 GPIO 8
RST   → ESP32 GPIO 9
BL    → ESP32 GPIO 46
```

**Touch (I2C):**
```
VCC   → ESP32 3.3V
GND   → ESP32 GND
SDA   → ESP32 GPIO 3
SCL   → ESP32 GPIO 46*
INT   → ESP32 GPIO 32
RST   → ESP32 GPIO 33 (optional)
```

*If GPIO 46 conflicts, adjust both touch SCL and backlight BL pins

### Step 3: Install PlatformIO

**Option A: VS Code (Easy)**
1. Install VS Code
2. Install PlatformIO extension
3. Open this project folder

**Option B: Command Line**
```bash
pip install platformio
```

### Step 4: Configure WiFi

Edit `include/config.h`:
```cpp
#define WIFI_SSID "MyWiFiNetwork"
#define WIFI_PASS "MyPassword123"
```

### Step 5: Set Default Location

In `src/main.cpp`, find this line (~92):
```cpp
location_service.setManualLocation(51.5074, -0.1278);  // London
```

Change to your location (get coordinates from Google Maps or similar):
```cpp
location_service.setManualLocation(YOUR_LAT, YOUR_LON);
```

### Step 6: Build & Upload

```bash
# One command to build and upload
platformio run --target upload --environment esp32-s3-devkit

# OR via VS Code: 
# Click the checkmark icon (Build) then arrow icon (Upload)
```

### Step 7: Monitor & Verify

```bash
platformio device monitor -b 115200
```

You should see:
```
=== ESP32-S3 Flight Radar ===
Initializing...
1. Initializing display...
2. Initializing touch...
3. Connecting to WiFi...
   WiFi connected!
4. Setting default location...
5. Initialization complete!
```

**Success!** The radar should display with aircraft showing up!

## 🎮 Using the App

### Main Radar Screen
- **White triangles** = Aircraft (pointing in flight direction)
- **Circles** = Radar range markers
- **Cardinal markings** = N, S, E, W

### Colors (Altitude)
- 🟢 **Green** = Low altitude (< 10,000 ft)
- 🟡 **Yellow** = Medium (10,000 - 25,000 ft)  
- 🔴 **Red** = High altitude (> 25,000 ft)

### Interactions
- **Single tap aircraft** → See details (callsign, speed, etc.)
- **Double tap** → Zoom in one level
- **Drag/swipe** → Pan around
- **Tap aircraft detail screen** → Return to radar

## 🔧 Troubleshooting Quick Fixes

### No display output?
```cpp
// In config.h, check these are correct for YOUR display:
#define TFT_MOSI 11
#define TFT_SCLK 12
#define TFT_CS   10
#define TFT_DC   8
#define TFT_RST  9
```

### Can't connect to WiFi?
1. Verify SSID/password are correct (case-sensitive!)
2. Try using 2.4GHz network (5GHz may not work)
3. Check ESP32 is in WiFi range

### No aircraft appearing?
1. Check WiFi connection (serial shows "WiFi: OK")
2. Verify location is correct (change to somewhere busier like London)
3. Wait 10 seconds for first API update

### Touch not working?
1. Verify I2C connections (GPIO 3 & 46)
2. Check touch controller I2C address (usually 0x5D)
3. Try without touch first - radar still works!

## 📚 Next Steps

1. **Customize colors**: Edit `#define COLOR_*` in `config.h`
2. **Adjust update speed**: Change `OPENSKY_UPDATE_INTERVAL` (milliseconds)
3. **Change zoom default**: Edit `#define DEFAULT_ZOOM` (0-4)
4. **Improve touch support**: Calibrate your touch panel for better accuracy
5. **Add logging**: Enable serial output for debugging

## 📖 Full Documentation

See `ESP32_FLIGHT_RADAR.md` for:
- Complete pin definitions
- Architecture explanation
- API integration details
- Performance optimization
- Advanced troubleshooting

## 🆘 Still Stuck?

1. **Check serial output** at 115200 baud for error messages
2. **Try the command line**:
   ```bash
   platformio run --target monitor  # Shows live serial
   ```
3. **Common issues checklist**:
   - [ ] Display connected to correct SPI pins
   - [ ] Touch connected to I2C (GPIO 3 & 46)
   - [ ] WiFi SSID/password correct in config.h
   - [ ] Board selected: esp32-s3-devkit
   - [ ] Power supply adequate (USB usually fine)

---

**You got this! 🚀** Once you see aircraft on the radar, you're done!

Enjoy tracking flights on your ESP32! ✈️
