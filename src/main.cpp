#include <Arduino.h>
#include "config.h"
#include "display_driver.h"
#include "radar_engine.h"
#include "touch_handler.h"
#include "opensky_api.h"
#include "location_service.h"
#include "ui_manager.h"

// Global objects
DisplayDriver display_driver;
RadarEngine radar_engine;
TouchHandler touch_handler;
OpenSkyAPI opensky_api;
LocationService location_service;
UIManager ui_manager(&display_driver, &radar_engine, &touch_handler, &opensky_api, &location_service);

// Startup state
enum StartupState {
    STARTUP_DISPLAY,
    STARTUP_WIFI,
    STARTUP_LOCATION,
    STARTUP_COMPLETE
};

StartupState startup_state = STARTUP_DISPLAY;
uint32_t startup_time = 0;

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n\n=== ESP32-S3 Flight Radar ===");
    Serial.println("Initializing...");

    // Initialize display
    Serial.println("1. Initializing display...");
    display_driver.init();
    display_driver.getDisplay().setTextColor(COLOR_TEXT);
    display_driver.getDisplay().setCursor(10, 10);
    display_driver.getDisplay().print("Initializing...");

    // Initialize other components
    Serial.println("2. Initializing touch...");
    touch_handler.init();

    Serial.println("3. Connecting to WiFi...");
    display_driver.drawText(10, 30, "WiFi: Connecting...", COLOR_TEXT, 1);
    if (opensky_api.connect()) {
        Serial.println("   WiFi connected!");
        display_driver.drawText(10, 50, "WiFi: OK", COLOR_TEXT, 1);
    } else {
        Serial.println("   WiFi connection failed!");
        display_driver.drawText(10, 50, "WiFi: Failed", COLOR_PLANE_ACTIVE, 1);
    }

    Serial.println("4. Setting default location...");
    // Default to London for testing - change this based on your preference
    location_service.setManualLocation(51.5074, -0.1278);
    Location loc = location_service.getLocation();
    radar_engine.init(loc.latitude, loc.longitude);

    Serial.println("5. Initialization complete!");
    display_driver.drawText(10, 70, "Ready!", COLOR_TEXT, 1);
    delay(1000);

    ui_manager.init();
    startup_time = millis();
}

void loop() {
    // Main application loop
    ui_manager.update();

    // Debug output every 10 seconds
    static uint32_t last_debug = 0;
    if (millis() - last_debug > 10000) {
        Serial.printf("Status: Aircraft: %d, WiFi: %s, Zoom: %d\n",
                     radar_engine.getProjectedAircraft().size(),
                     opensky_api.isConnected() ? "OK" : "FAIL",
                     radar_engine.getZoomLevel());
        last_debug = millis();
    }

    // Keep some CPU breathing room
    delay(10);
}

// Helper function to update location via Serial (for testing)
void handleSerialInput() {
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        input.trim();

        if (input.startsWith("postcode:")) {
            String postcode = input.substring(9);
            Serial.printf("Setting postcode: %s\n", postcode.c_str());
            ui_manager.setPostcode(postcode);
        } else if (input.startsWith("radius:")) {
            float radius = input.substring(7).toFloat();
            Serial.printf("Setting radius: %.1f km\n", radius);
            ui_manager.setRadiusKm(radius);
        } else if (input == "status") {
            Location loc = location_service.getLocation();
            Serial.printf("Current location: %.4f, %.4f\n", loc.latitude, loc.longitude);
            Serial.printf("Aircraft detected: %d\n", radar_engine.getProjectedAircraft().size());
            Serial.printf("Zoom level: %d (%.0f km)\n", radar_engine.getZoomLevel(), radar_engine.getRadarRadius());
        } else {
            Serial.println("Commands: postcode:<code>, radius:<km>, status");
        }
    }
}
