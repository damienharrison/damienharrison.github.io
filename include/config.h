#ifndef CONFIG_H
#define CONFIG_H

// Display Configuration
#define TFT_WIDTH 240
#define TFT_HEIGHT 240
#define CENTER_X (TFT_WIDTH / 2)
#define CENTER_Y (TFT_HEIGHT / 2)
#define RADIUS 110  // Inner usable radius for round display

// WiFi Configuration - Set in UI on device
// Leave these empty - will be loaded from SPIFFS
#define WIFI_SSID ""
#define WIFI_PASS ""
#define WIFI_CONFIG_FILE "/spiffs/wifi.json"
#define WIFI_SETUP_TIMEOUT 300000  // 5 minutes to setup WiFi

// OpenSky API Configuration
#define OPENSKY_API_URL "https://opensky-network.org/api/states/all"
#define OPENSKY_API_TIMEOUT 10000  // 10 seconds
#define OPENSKY_UPDATE_INTERVAL 5000  // Update every 5 seconds

// Radar Configuration
#define DEFAULT_RADIUS_KM 50
#define MAX_RADIUS_KM 250
#define MIN_RADIUS_KM 5
#define RADAR_UPDATE_INTERVAL 3000  // Update radar display every 3 seconds

// Touch Configuration
#define TOUCH_CS 33
#define TOUCH_INT 32

// Theme Mode
enum ThemeMode {
    THEME_LIGHT,
    THEME_DARK
};
#define DEFAULT_THEME THEME_DARK

// Display Colors (RGB565) - Light Theme
#define COLOR_BLACK 0x0000
#define COLOR_WHITE 0xFFFF
#define COLOR_RADAR_BG_LIGHT 0x1020      // Dark blue
#define COLOR_RADAR_GRID_LIGHT 0x2945    // Dim blue
#define COLOR_PLANE_ACTIVE_LIGHT 0xF800  // Red
#define COLOR_PLANE_SELECTED_LIGHT 0xFFE0 // Yellow
#define COLOR_TEXT_LIGHT 0xFFFF          // White
#define COLOR_ALTITUDE_HIGH_LIGHT 0xF800   // Red
#define COLOR_ALTITUDE_MID_LIGHT 0xFBE0    // Yellow
#define COLOR_ALTITUDE_LOW_LIGHT 0x07E0    // Green
#define COLOR_BUTTON_LIGHT 0x39E7        // Light blue
#define COLOR_BUTTON_ACTIVE_LIGHT 0x1F1F // Dark gray

// Display Colors (RGB565) - Night Theme (Dark)
#define COLOR_RADAR_BG_DARK 0x0000       // Pure black
#define COLOR_RADAR_GRID_DARK 0x1884     // Very dim blue
#define COLOR_PLANE_ACTIVE_DARK 0xFC00   // Bright red
#define COLOR_PLANE_SELECTED_DARK 0xFFF8 // Bright yellow
#define COLOR_TEXT_DARK 0xF7FF           // Bright white
#define COLOR_ALTITUDE_HIGH_DARK 0xFC00    // Bright red
#define COLOR_ALTITUDE_MID_DARK 0xFFF8    // Bright yellow
#define COLOR_ALTITUDE_LOW_DARK 0x0FF0    // Bright cyan
#define COLOR_BUTTON_DARK 0x18E3         // Dark cyan
#define COLOR_BUTTON_ACTIVE_DARK 0x7FFF  // Bright cyan

// Macro to get color based on theme
#define GET_COLOR(light, dark, theme) ((theme == THEME_LIGHT) ? (light) : (dark))

// Default color definitions (will be set by theme)
extern uint16_t COLOR_RADAR_BG;
extern uint16_t COLOR_RADAR_GRID;
extern uint16_t COLOR_PLANE_ACTIVE;
extern uint16_t COLOR_PLANE_SELECTED;
extern uint16_t COLOR_TEXT;
extern uint16_t COLOR_ALTITUDE_HIGH;
extern uint16_t COLOR_ALTITUDE_MID;
extern uint16_t COLOR_ALTITUDE_LOW;
extern uint16_t COLOR_BUTTON;
extern uint16_t COLOR_BUTTON_ACTIVE;
extern ThemeMode CURRENT_THEME;

// Aircraft Altitude Thresholds (feet)
#define ALTITUDE_HIGH 25000
#define ALTITUDE_MID 10000

// Trail Configuration
#define MAX_TRAIL_POINTS 60         // Points per aircraft
#define MAX_TRAILS 50               // Max aircraft tracked
#define POINT_INTERVAL 5000         // Add point every 5 sec
#define TRAIL_TIMEOUT 600000        // Remove after 10 min

// Zoom Levels
#define ZOOM_LEVEL_COUNT 5
#define DEFAULT_ZOOM 2  // 50km default

// UI States
enum UIState {
    UI_WIFI_SETUP,           // Setup WiFi on startup
    UI_RADAR,
    UI_DETAILS,
    UI_MAP,
    UI_SETTINGS,             // Main settings menu
    UI_THEME_SELECT,
    UI_RADAR_SETTINGS,       // Radar configuration
    UI_TRAIL_SETTINGS,       // Trail configuration
    UI_DISPLAY_SETTINGS      // Display configuration
};

enum TouchMode {
    TOUCH_IDLE,
    TOUCH_SINGLE,
    TOUCH_DOUBLE,
    TOUCH_ZOOM
};

#endif
