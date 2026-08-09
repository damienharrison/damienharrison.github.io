# ESP32-S3 Flight Radar - New Features Update

## ✨ Night Mode (Dark/Light Theme)

### Features
- **Automatic Theme Application**: Colors adapt throughout the entire UI
- **Easy Switching**: Toggle between themes from Settings screen or serial command
- **Persistent Storage**: Theme preference saved to SPIFFS
- **Eye-Friendly**: Dark theme with optimized colors for nighttime use
- **Light Theme**: High contrast colors for daytime operation

### Color Schemes

#### Dark Theme (Optimized for Night)
- Background: Pure black (0x0000) for zero power glow
- Grid: Very dim blue (0x1884) for subtle reference
- Aircraft: Bright red (0xFC00), Bright yellow (0xFFF8)
- Text: Bright white (0xF7FF)
- Altitude Low: Bright cyan (0x0FF0)
- Altitude Mid: Bright yellow (0xFFF8)
- Altitude High: Bright red (0xFC00)

#### Light Theme (Optimized for Day)
- Background: Dark blue (0x1020) for clarity
- Grid: Dim blue (0x2945) for visibility
- Aircraft: Red (0xF800), Yellow (0xFFE0)
- Text: White (0xFFFF)
- Altitude Low: Green (0x07E0)
- Altitude Mid: Yellow (0xFBE0)
- Altitude High: Red (0xF800)

### How to Use

**On Device:**
1. Open Settings screen (tap at bottom of radar)
2. Tap "Toggle Theme" button to switch
3. Current theme shown below buttons

**Via Serial:**
```
theme              # Check current theme
theme:light        # Switch to light theme
theme:dark         # Switch to dark theme
```

### Implementation Details

- **Theme Manager** (`theme_manager.h/cpp`): Manages theme state and persistence
- **Dynamic Colors**: All display colors are global variables set by theme
- **Persistent Storage**: Theme preference saved to `/spiffs/theme.json`
- **On Startup**: Loads saved theme automatically

---

## 🔐 WiFi Configuration on Device

### Features
- **No Hardcoded Credentials**: Set WiFi SSID and password directly on the device
- **Virtual Keyboard**: Easy text input via touchscreen
- **Step-by-Step Setup**: Clear instructions for configuration
- **Persistent Storage**: Credentials saved securely to SPIFFS
- **Skip Option**: Can skip WiFi setup if needed
- **Timeout**: Auto-proceeds after 5 minutes of inactivity

### How to Use

#### First-Time Setup
1. Power on the ESP32-S3
2. "WiFi Setup" screen appears automatically
3. **Step 1**: Enter network name (SSID)
   - Use on-screen keyboard to type SSID
   - Tap NEXT when done
4. **Step 2**: Enter password
   - Use on-screen keyboard to type password
   - Tap NEXT to save and continue
5. **Theme Select**: Choose your preferred theme
   - Tap "Light" or "Dark"
   - Tap "Start Radar" to begin

#### If WiFi Already Configured
- Device connects automatically on startup
- Proceeds directly to radar view
- Can reconfigure from Settings screen if needed

#### Via Serial Commands
```
wifi:status        # Check current WiFi config and connection status
wifi:reset         # Delete saved credentials (will show setup screen on restart)
```

### Virtual Keyboard Features
- **Full QWERTY layout** (lowercase letters)
- **Spacebar** for adding spaces
- **Backspace** (<) for deleting characters
- **Max 32 characters** for SSID/password
- **Real-time feedback** as you type

### Storage & Security

**Storage Location**: `/spiffs/wifi.json`
```json
{
  "ssid": "YourNetworkName",
  "password": "YourPassword"
}
```

**Notes:**
- Credentials stored in plain text on device SPIFFS
- Consider this when using on shared devices
- Can reset credentials via serial command: `wifi:reset`
- Device must have SPIFFS initialized (automatic)

### Implementation Details

- **WiFi Manager** (`wifi_manager.h/cpp`):
  - Loads/saves credentials to SPIFFS
  - Handles WiFi connection logic
  - Manages connection attempts and timeouts
  - Reports signal strength

- **Virtual Keyboard** (`virtual_keyboard.h/cpp`):
  - Touchscreen-based text input
  - Visual feedback on key selection
  - Efficient character handling
  - Support for multiple input fields

- **Storage**: Uses SPIFFS (SPI Flash File System)
  - Included in partition scheme (`min_spiffs.csv`)
  - Automatic initialization on first run

---

## 🔄 Updated Startup Flow

```
1. Display Initialization
   ↓
2. WiFi Manager & SPIFFS Init
   ↓
3. Theme Initialization
   ├─ If no theme saved: Use default (Dark)
   └─ Apply theme colors globally
   ↓
4. Check WiFi Credentials
   ├─ If configured: Show "WiFi: Connecting..."
   └─ If not: Show "WiFi: Setup needed"
   ↓
5. Touch & UI Initialization
   ↓
6. Enter Main Loop
   ├─ If WiFi not configured: Show WiFi Setup Screen
   └─ If WiFi configured: Proceed to Theme Select or Radar
```

---

## 📱 Settings Screen

New Settings screen accessible from any radar screen:

**Features:**
- View current postcode
- Check WiFi status
- See current theme
- **Toggle Theme** button: Switch between Light/Dark
- **Reconfigure WiFi** button: Re-enter WiFi setup
- Tap anywhere to return to radar

---

## ⌨️ Serial Commands (Expanded)

```
postcode:<code>     # Set location by UK postcode
                    # Example: postcode:SW1A1AA

radius:<km>         # Change radar radius (5-250 km)
                    # Example: radius:100

status              # Print current status
                    # Shows: location, aircraft count, zoom, WiFi status

theme               # Check current theme
                    # Output: LIGHT or DARK

theme:light         # Switch to light theme

theme:dark          # Switch to dark theme

wifi:status         # Print WiFi information
                    # Shows: SSID, connection status, signal strength

wifi:reset          # Delete WiFi credentials
                    # Restart device to reconfigure

help or ?           # Show all available commands
```

---

## 🎨 Color Customization

To customize colors further, edit the color definitions in `config.h`:

```cpp
// Light Theme Colors
#define COLOR_RADAR_BG_LIGHT 0x1020      // Change to your color
#define COLOR_RADAR_GRID_LIGHT 0x2945
#define COLOR_PLANE_ACTIVE_LIGHT 0xF800  // Red aircraft
// ... etc
```

**RGB565 Color Format:**
- 5 bits Red, 6 bits Green, 5 bits Blue
- Use online RGB565 converter or this formula:
  ```
  RGB565 = ((R >> 3) << 11) | ((G >> 2) << 5) | (B >> 3)
  ```

---

## 📊 SPIFFS Storage Usage

**Files Created:**
- `/spiffs/wifi.json` - WiFi credentials (~100 bytes)
- `/spiffs/theme.json` - Theme preference (~50 bytes)

**Total Partition**: 64 KB (defined in `min_spiffs.csv`)
**Remaining**: ~63.85 KB available for future features (logs, caching, etc.)

---

## 🔧 Troubleshooting

### WiFi Setup Screen Doesn't Appear
- Device boots directly to radar?
  → WiFi already configured
  → Use `wifi:reset` via serial to reconfigure

### Keyboard Input Not Working
- Taps not registering?
  → Check touch calibration
  → Verify touch pin connections (GPIO 32/33)
  → Try double-tapping slowly on each key

### Theme Not Persisting
- Reverts to dark on restart?
  → Check SPIFFS is mounted: `wifi:status` shows errors?
  → Verify `min_spiffs.csv` partition scheme is active
  → Use: `esp32-hal-psram.c` build flag

### Can't Connect to WiFi
- Shows WiFi setup but can't connect?
  → Verify SSID and password are correct (case-sensitive)
  → Try 2.4GHz network (ESP32-S3 may struggle with 5GHz)
  → Check network doesn't require WPA3 enterprise
  → Use `wifi:status` to check signal strength

---

## 🎯 Default Behavior

**First Boot:**
1. Shows WiFi Setup screen
2. Waits for WiFi configuration
3. After 5 minutes, proceeds to Theme Select
4. Default theme: Dark
5. Default location: London (51.5074°N, 0.1278°W)

**Subsequent Boots:**
1. Loads WiFi credentials from SPIFFS
2. Loads theme preference from SPIFFS
3. Connects to WiFi automatically
4. Proceeds directly to Radar view

---

## 🚀 What's Next?

Future enhancement ideas:
- [ ] Password masking on virtual keyboard
- [ ] Multiple WiFi network support
- [ ] WiFi signal strength visual indicator
- [ ] Auto-detect daylight and switch themes
- [ ] Save multiple locations as bookmarks
- [ ] Backup/restore settings to cloud
- [ ] QR code WiFi setup option

---

## 📝 Developer Notes

### Adding New Settings

1. Create new screen: `void drawNewSettingScreen()`
2. Add to UI state enum: `UI_NEW_SETTING`
3. Handle touches: `handleNewSettingTouch()`
4. Store persistent data to SPIFFS (like WiFi/theme)

### Modifying Theme Colors

All global color variables are defined in `config.h` and implemented in `theme_manager.cpp`. Update both locations for consistency.

### Memory Impact

- WiFi Manager: ~2 KB
- Theme Manager: ~1 KB
- Virtual Keyboard: ~3 KB
- Globals for colors: ~256 bytes

**Total**: ~6.3 KB additional RAM usage

---

**Enjoy your enhanced Flight Radar with customizable themes and easy WiFi setup!** 🎨✈️
