#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"

struct Settings {
    // Radar settings
    float radar_radius_km = DEFAULT_RADIUS_KM;
    float center_latitude = 51.5074;
    float center_longitude = -0.1278;

    // Display settings
    int brightness = 255;
    ThemeMode theme = THEME_DARK;
    int grid_opacity = 255;

    // API settings
    int api_update_interval_ms = OPENSKY_UPDATE_INTERVAL;
    int radar_update_interval_ms = RADAR_UPDATE_INTERVAL;

    // Trail settings
    bool trails_enabled = true;
    int max_trail_points = MAX_TRAIL_POINTS;
    int max_trails = MAX_TRAILS;

    // WiFi settings (stored separately)
    String wifi_ssid;
    String wifi_password;
};

class SettingsManager {
private:
    Settings current_settings;
    const char* SETTINGS_FILE = "/spiffs/settings.json";
    const char* WIFI_FILE = "/spiffs/wifi.json";

public:
    SettingsManager();

    // Load/Save settings
    bool loadSettings();
    bool saveSettings();
    void resetToDefaults();

    // Radar settings
    float getRadarRadius() const { return current_settings.radar_radius_km; }
    void setRadarRadius(float radius) {
        current_settings.radar_radius_km = constrain(radius, MIN_RADIUS_KM, MAX_RADIUS_KM);
        saveSettings();
    }

    float getCenterLatitude() const { return current_settings.center_latitude; }
    float getCenterLongitude() const { return current_settings.center_longitude; }
    void setCenter(float lat, float lon) {
        current_settings.center_latitude = lat;
        current_settings.center_longitude = lon;
        saveSettings();
    }

    // Display settings
    int getBrightness() const { return current_settings.brightness; }
    void setBrightness(int b) {
        current_settings.brightness = constrain(b, 0, 255);
        saveSettings();
    }

    ThemeMode getTheme() const { return current_settings.theme; }
    void setTheme(ThemeMode t) {
        current_settings.theme = t;
        saveSettings();
    }

    int getGridOpacity() const { return current_settings.grid_opacity; }
    void setGridOpacity(int opacity) {
        current_settings.grid_opacity = constrain(opacity, 0, 255);
        saveSettings();
    }

    // API settings
    int getApiUpdateInterval() const { return current_settings.api_update_interval_ms; }
    void setApiUpdateInterval(int interval) {
        current_settings.api_update_interval_ms = constrain(interval, 1000, 60000);
        saveSettings();
    }

    int getRadarUpdateInterval() const { return current_settings.radar_update_interval_ms; }
    void setRadarUpdateInterval(int interval) {
        current_settings.radar_update_interval_ms = constrain(interval, 500, 10000);
        saveSettings();
    }

    // Trail settings
    bool areTrailsEnabled() const { return current_settings.trails_enabled; }
    void setTrailsEnabled(bool enabled) {
        current_settings.trails_enabled = enabled;
        saveSettings();
    }

    int getMaxTrailPoints() const { return current_settings.max_trail_points; }
    void setMaxTrailPoints(int points) {
        current_settings.max_trail_points = constrain(points, 10, 120);
        saveSettings();
    }

    int getMaxTrails() const { return current_settings.max_trails; }
    void setMaxTrails(int count) {
        current_settings.max_trails = constrain(count, 10, 100);
        saveSettings();
    }

    // WiFi settings
    String getWiFiSSID() const { return current_settings.wifi_ssid; }
    String getWiFiPassword() const { return current_settings.wifi_password; }
    void setWiFiCredentials(const String& ssid, const String& password) {
        current_settings.wifi_ssid = ssid;
        current_settings.wifi_password = password;
        saveWiFiSettings();
    }

    // Get all settings
    const Settings& getSettings() const { return current_settings; }

private:
    bool saveWiFiSettings();
};

#endif
