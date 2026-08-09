#include <Arduino.h>
#include "config.h"
#include "display_driver.h"
#include "radar_engine.h"
#include "touch_handler.h"
#include "opensky_api.h"
#include "location_service.h"
#include "wifi_manager.h"
#include "theme_manager.h"
#include "trail_manager.h"
#include "ui_manager.h"

// Global objects
DisplayDriver display_driver;
RadarEngine radar_engine;
TouchHandler touch_handler;
OpenSkyAPI opensky_api;
LocationService location_service;
WiFiManager wifi_manager;
ThemeManager theme_manager;
TrailManager trail_manager;
UIManager ui_manager(&display_driver, &radar_engine, &touch_handler, &opensky_api,
                     &location_service, &wifi_manager, &theme_manager, &trail_manager);

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n\n=== ESP32-S3 Flight Radar ===");
    Serial.println("Initializing...");

    // Initialize display
    Serial.println("1. Initializing display...");
    display_driver.init();

    // Initialize WiFi manager and load credentials
    Serial.println("2. Initializing WiFi manager...");
    wifi_manager.initSPIFFS();
    wifi_manager.loadCredentials();

    if (wifi_manager.isConfigured()) {
        Serial.println("   WiFi credentials found");
        display_driver.drawText(10, 30, "WiFi: Connecting...", COLOR_TEXT, 1);

        if (wifi_manager.connect()) {
            Serial.println("   WiFi connected!");
            display_driver.drawText(10, 50, "WiFi: Connected", COLOR_TEXT, 1);
        } else {
            Serial.println("   WiFi connection failed");
            display_driver.drawText(10, 50, "WiFi: Failed", COLOR_PLANE_ACTIVE, 1);
        }
    } else {
        Serial.println("   No WiFi credentials configured");
        display_driver.drawText(10, 30, "WiFi: Setup needed", COLOR_PLANE_ACTIVE, 1);
    }

    // Initialize theme
    Serial.println("3. Initializing theme...");
    theme_manager.init();

    // Initialize touch
    Serial.println("4. Initializing touch...");
    touch_handler.init();

    // Initialize trail manager
    Serial.println("5. Initializing trail manager...");
    radar_engine.setTrailManager(&trail_manager);

    // Initialize location
    Serial.println("6. Setting default location...");
    location_service.setManualLocation(51.5074, -0.1278);
    Location loc = location_service.getLocation();
    radar_engine.init(loc.latitude, loc.longitude);

    // Initialize UI
    Serial.println("7. Initializing UI...");
    ui_manager.init();

    Serial.println("8. Initialization complete!");
    Serial.println("\nSerial Commands:");
    Serial.println("  postcode:<code>  - Set location by UK postcode");
    Serial.println("  radius:<km>       - Change radar radius (km)");
    Serial.println("  status            - Print current status");
    Serial.println("  theme             - Print current theme");
    Serial.println("  theme:light       - Switch to light theme");
    Serial.println("  theme:dark        - Switch to dark theme");
    Serial.println("  trails:on         - Enable aircraft trails");
    Serial.println("  trails:off        - Disable aircraft trails");
    Serial.println("  trails:clear      - Clear all trail data");
    Serial.println("  wifi:status       - Print WiFi status");
    Serial.println("  wifi:reset        - Reset WiFi credentials");

    delay(1000);
}

void loop() {
    // Main application loop
    ui_manager.update();

    // Handle serial commands
    handleSerialInput();

    // Debug output every 15 seconds
    static uint32_t last_debug = 0;
    if (millis() - last_debug > 15000) {
        Serial.printf("[Status] Aircraft: %d | WiFi: %s | Theme: %s | Zoom: %d\n",
                     radar_engine.getProjectedAircraft().size(),
                     opensky_api.isConnected() ? "ON" : "OFF",
                     (theme_manager.getTheme() == THEME_LIGHT) ? "LIGHT" : "DARK",
                     radar_engine.getZoomLevel());
        last_debug = millis();
    }

    delay(10);
}

void handleSerialInput() {
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        input.trim();

        if (input.startsWith("postcode:")) {
            String postcode = input.substring(9);
            Serial.printf("Setting postcode: %s\n", postcode.c_str());
            ui_manager.setPostcode(postcode);
        }
        else if (input.startsWith("radius:")) {
            float radius = input.substring(7).toFloat();
            Serial.printf("Setting radius: %.1f km\n", radius);
            ui_manager.setRadiusKm(radius);
        }
        else if (input == "status") {
            Location loc = location_service.getLocation();
            Serial.printf("Current location: %.4f, %.4f\n", loc.latitude, loc.longitude);
            Serial.printf("Aircraft detected: %d\n", radar_engine.getProjectedAircraft().size());
            Serial.printf("Zoom level: %d (%.0f km)\n", radar_engine.getZoomLevel(), radar_engine.getRadarRadius());
            Serial.printf("WiFi: %s\n", opensky_api.isConnected() ? "Connected" : "Disconnected");
        }
        else if (input == "theme") {
            const char* theme_name = (theme_manager.getTheme() == THEME_LIGHT) ? "LIGHT" : "DARK";
            Serial.printf("Current theme: %s\n", theme_name);
        }
        else if (input == "theme:light") {
            theme_manager.setTheme(THEME_LIGHT);
            Serial.println("Switched to LIGHT theme");
        }
        else if (input == "theme:dark") {
            theme_manager.setTheme(THEME_DARK);
            Serial.println("Switched to DARK theme");
        }
        else if (input == "wifi:status") {
            if (wifi_manager.isConfigured()) {
                Serial.printf("WiFi SSID: %s\n", wifi_manager.getSSID().c_str());
                Serial.printf("WiFi Connected: %s\n", wifi_manager.isConnected() ? "Yes" : "No");
                Serial.printf("Signal Strength: %d%%\n", wifi_manager.getSignalStrength());
            } else {
                Serial.println("WiFi not configured");
            }
        }
        else if (input == "wifi:reset") {
            wifi_manager.deleteCredentials();
            Serial.println("WiFi credentials deleted. Restart device to reconfigure.");
        }
        else if (input == "trails:on") {
            trail_manager.setEnabled(true);
            Serial.println("Aircraft trails enabled");
        }
        else if (input == "trails:off") {
            trail_manager.setEnabled(false);
            Serial.println("Aircraft trails disabled");
        }
        else if (input == "trails:clear") {
            trail_manager.clearAllTrails();
            Serial.println("All trails cleared");
        }
        else if (input == "help" || input == "?") {
            Serial.println("Available commands:");
            Serial.println("  postcode:<code>  - Set location by UK postcode");
            Serial.println("  radius:<km>       - Change radar radius");
            Serial.println("  status            - Print status");
            Serial.println("  theme             - Print current theme");
            Serial.println("  theme:light       - Switch to light theme");
            Serial.println("  theme:dark        - Switch to dark theme");
            Serial.println("  trails:on         - Enable aircraft trails");
            Serial.println("  trails:off        - Disable aircraft trails");
            Serial.println("  trails:clear      - Clear all trail data");
            Serial.println("  wifi:status       - Print WiFi status");
            Serial.println("  wifi:reset        - Reset WiFi credentials");
        }
        else {
            Serial.println("Unknown command. Type 'help' for available commands.");
        }
    }
}
