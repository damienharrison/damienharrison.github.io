#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "config.h"
#include "display_driver.h"
#include "radar_engine.h"
#include "touch_handler.h"
#include "opensky_api.h"
#include "location_service.h"

class UIManager {
private:
    UIState current_state;
    DisplayDriver* display;
    RadarEngine* radar;
    TouchHandler* touch;
    OpenSkyAPI* api;
    LocationService* location;

    // UI timing
    uint32_t last_radar_update;
    uint32_t last_api_update;

    // State management
    String current_postcode;
    float current_radius;

public:
    UIManager(DisplayDriver* disp, RadarEngine* rad, TouchHandler* touch_h,
              OpenSkyAPI* api_h, LocationService* loc);

    void init();
    void update();
    void setPostcode(const String& postcode);
    void setRadiusKm(float radius);

    // Screen rendering
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
};

#endif
