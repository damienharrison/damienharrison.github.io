#ifndef CONFIG_H
#define CONFIG_H

// Display Configuration
#define TFT_WIDTH 240
#define TFT_HEIGHT 240
#define CENTER_X (TFT_WIDTH / 2)
#define CENTER_Y (TFT_HEIGHT / 2)
#define RADIUS 110  // Inner usable radius for round display

// WiFi Configuration
#define WIFI_SSID "YOUR_SSID"
#define WIFI_PASS "YOUR_PASSWORD"

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

// Display Colors (RGB565)
#define COLOR_BLACK 0x0000
#define COLOR_WHITE 0xFFFF
#define COLOR_RADAR_BG 0x1020
#define COLOR_RADAR_GRID 0x2945
#define COLOR_PLANE_ACTIVE 0xF800  // Red
#define COLOR_PLANE_SELECTED 0xFFE0  // Yellow
#define COLOR_TEXT 0xFFFF
#define COLOR_ALTITUDE_HIGH 0xF800   // Red (high alt)
#define COLOR_ALTITUDE_MID 0xFBE0    // Yellow (mid alt)
#define COLOR_ALTITUDE_LOW 0x07E0    // Green (low alt)

// Aircraft Altitude Thresholds (feet)
#define ALTITUDE_HIGH 25000
#define ALTITUDE_MID 10000

// Zoom Levels
#define ZOOM_LEVEL_COUNT 5
#define DEFAULT_ZOOM 2  // 50km default

// UI States
enum UIState {
    UI_RADAR,
    UI_DETAILS,
    UI_MAP,
    UI_SETTINGS
};

enum TouchMode {
    TOUCH_IDLE,
    TOUCH_SINGLE,
    TOUCH_DOUBLE,
    TOUCH_ZOOM
};

#endif
