#include "settings_manager.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>

SettingsManager::SettingsManager() {
    loadSettings();
}

bool SettingsManager::loadSettings() {
    if (!SPIFFS.exists(SETTINGS_FILE)) {
        Serial.println("Settings file not found, using defaults");
        resetToDefaults();
        return true;
    }

    File file = SPIFFS.open(SETTINGS_FILE, "r");
    if (!file) {
        Serial.println("Failed to open settings file");
        return false;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.printf("JSON parse error: %s\n", error.c_str());
        resetToDefaults();
        return false;
    }

    // Radar settings
    if (doc.containsKey("radar_radius_km")) {
        current_settings.radar_radius_km = doc["radar_radius_km"];
    }
    if (doc.containsKey("center_latitude")) {
        current_settings.center_latitude = doc["center_latitude"];
    }
    if (doc.containsKey("center_longitude")) {
        current_settings.center_longitude = doc["center_longitude"];
    }

    // Display settings
    if (doc.containsKey("brightness")) {
        current_settings.brightness = doc["brightness"];
    }
    if (doc.containsKey("theme")) {
        current_settings.theme = (doc["theme"] == "light") ? THEME_LIGHT : THEME_DARK;
    }
    if (doc.containsKey("grid_opacity")) {
        current_settings.grid_opacity = doc["grid_opacity"];
    }

    // API settings
    if (doc.containsKey("api_update_interval_ms")) {
        current_settings.api_update_interval_ms = doc["api_update_interval_ms"];
    }
    if (doc.containsKey("radar_update_interval_ms")) {
        current_settings.radar_update_interval_ms = doc["radar_update_interval_ms"];
    }

    // Trail settings
    if (doc.containsKey("trails_enabled")) {
        current_settings.trails_enabled = doc["trails_enabled"];
    }
    if (doc.containsKey("max_trail_points")) {
        current_settings.max_trail_points = doc["max_trail_points"];
    }
    if (doc.containsKey("max_trails")) {
        current_settings.max_trails = doc["max_trails"];
    }

    Serial.println("Settings loaded successfully");
    return true;
}

bool SettingsManager::saveSettings() {
    JsonDocument doc;

    // Radar settings
    doc["radar_radius_km"] = current_settings.radar_radius_km;
    doc["center_latitude"] = current_settings.center_latitude;
    doc["center_longitude"] = current_settings.center_longitude;

    // Display settings
    doc["brightness"] = current_settings.brightness;
    doc["theme"] = (current_settings.theme == THEME_LIGHT) ? "light" : "dark";
    doc["grid_opacity"] = current_settings.grid_opacity;

    // API settings
    doc["api_update_interval_ms"] = current_settings.api_update_interval_ms;
    doc["radar_update_interval_ms"] = current_settings.radar_update_interval_ms;

    // Trail settings
    doc["trails_enabled"] = current_settings.trails_enabled;
    doc["max_trail_points"] = current_settings.max_trail_points;
    doc["max_trails"] = current_settings.max_trails;

    File file = SPIFFS.open(SETTINGS_FILE, "w");
    if (!file) {
        Serial.println("Failed to open settings file for writing");
        return false;
    }

    if (serializeJson(doc, file) == 0) {
        Serial.println("Failed to write settings to file");
        file.close();
        return false;
    }

    file.close();
    Serial.println("Settings saved successfully");
    return true;
}

bool SettingsManager::saveWiFiSettings() {
    JsonDocument doc;
    doc["ssid"] = current_settings.wifi_ssid;
    doc["password"] = current_settings.wifi_password;

    File file = SPIFFS.open(WIFI_FILE, "w");
    if (!file) {
        return false;
    }

    serializeJson(doc, file);
    file.close();
    return true;
}

void SettingsManager::resetToDefaults() {
    current_settings.radar_radius_km = DEFAULT_RADIUS_KM;
    current_settings.center_latitude = 51.5074;
    current_settings.center_longitude = -0.1278;
    current_settings.brightness = 255;
    current_settings.theme = THEME_DARK;
    current_settings.grid_opacity = 255;
    current_settings.api_update_interval_ms = OPENSKY_UPDATE_INTERVAL;
    current_settings.radar_update_interval_ms = RADAR_UPDATE_INTERVAL;
    current_settings.trails_enabled = true;
    current_settings.max_trail_points = MAX_TRAIL_POINTS;
    current_settings.max_trails = MAX_TRAILS;
    saveSettings();
}
