#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "config.h"
#include "display_driver.h"
#include "radar_engine.h"
#include "touch_handler.h"
#include "opensky_api.h"
#include "location_service.h"
#include "wifi_manager.h"
#include "theme_manager.h"
#include "virtual_keyboard.h"
#include "trail_manager.h"

class UIManager {
private:
    UIState current_state;
    DisplayDriver* display;
    RadarEngine* radar;
    TouchHandler* touch;
    OpenSkyAPI* api;
    LocationService* location;
    WiFiManager* wifi_mgr;
    ThemeManager* theme_mgr;
    VirtualKeyboard* keyboard;
    TrailManager* trail_mgr;

    // UI timing
    uint32_t last_radar_update;
    uint32_t last_api_update;
    uint32_t wifi_setup_start;

    // State management
    String current_postcode;
    float current_radius;
    int wifi_setup_step;  // 0=SSID, 1=PASSWORD
    String temp_ssid;
    String temp_password;
    bool show_trails;

public:
    UIManager(DisplayDriver* disp, RadarEngine* rad, TouchHandler* touch_h,
              OpenSkyAPI* api_h, LocationService* loc, WiFiManager* wifi,
              ThemeManager* theme, TrailManager* trails);

    void init();
    void update();
    void setPostcode(const String& postcode);
    void setRadiusKm(float radius);
    bool isWiFiConfigured() const;

    // Screen rendering
    void drawWiFiSetupScreen();
    void drawThemeSelectScreen();
    void drawRadarScreen();
    void drawDetailsScreen();
    void drawMapScreen();
    void drawSettingsScreen();

    // Input handling
    void handleTouchInput();

    // Getters
    UIState getCurrentState() { return current_state; }

private:
    void handleRadarTouch();
    void handleDetailsTouch();
    void handleMapTouch();
    void handleWiFiSetupTouch();
    void handleThemeSelectTouch();
    void drawButton(int x, int y, int w, int h, const char* label, bool highlighted = false);
};

#endif
