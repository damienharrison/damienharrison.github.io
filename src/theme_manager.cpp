#include "theme_manager.h"

// Global color variables
uint16_t COLOR_RADAR_BG = COLOR_RADAR_BG_DARK;
uint16_t COLOR_RADAR_GRID = COLOR_RADAR_GRID_DARK;
uint16_t COLOR_PLANE_ACTIVE = COLOR_PLANE_ACTIVE_DARK;
uint16_t COLOR_PLANE_SELECTED = COLOR_PLANE_SELECTED_DARK;
uint16_t COLOR_TEXT = COLOR_TEXT_DARK;
uint16_t COLOR_ALTITUDE_HIGH = COLOR_ALTITUDE_HIGH_DARK;
uint16_t COLOR_ALTITUDE_MID = COLOR_ALTITUDE_MID_DARK;
uint16_t COLOR_ALTITUDE_LOW = COLOR_ALTITUDE_LOW_DARK;
uint16_t COLOR_BUTTON = COLOR_BUTTON_DARK;
uint16_t COLOR_BUTTON_ACTIVE = COLOR_BUTTON_ACTIVE_DARK;
ThemeMode CURRENT_THEME = THEME_DARK;

ThemeManager::ThemeManager() : current_theme(DEFAULT_THEME) {}

void ThemeManager::init() {
    loadTheme();
    applyTheme();
}

void ThemeManager::loadTheme() {
    if (!SPIFFS.begin(true)) {
        current_theme = DEFAULT_THEME;
        return;
    }

    if (!SPIFFS.exists(THEME_FILE)) {
        current_theme = DEFAULT_THEME;
        return;
    }

    File file = SPIFFS.open(THEME_FILE, "r");
    if (!file) {
        current_theme = DEFAULT_THEME;
        return;
    }

    String content = "";
    while (file.available()) {
        content += (char)file.read();
    }
    file.close();

    DynamicJsonDocument doc(128);
    if (deserializeJson(doc, content) == DeserializationError::Ok) {
        int theme = doc["theme"].as<int>();
        current_theme = (theme == THEME_LIGHT) ? THEME_LIGHT : THEME_DARK;
    } else {
        current_theme = DEFAULT_THEME;
    }
}

void ThemeManager::saveTheme() {
    if (!SPIFFS.begin(true)) {
        return;
    }

    DynamicJsonDocument doc(128);
    doc["theme"] = (int)current_theme;

    File file = SPIFFS.open(THEME_FILE, "w");
    if (file) {
        serializeJson(doc, file);
        file.close();
        Serial.printf("Theme saved: %s\n", current_theme == THEME_LIGHT ? "LIGHT" : "DARK");
    }
}

void ThemeManager::setTheme(ThemeMode theme) {
    if (current_theme != theme) {
        current_theme = theme;
        applyTheme();
        saveTheme();
    }
}

void ThemeManager::toggleTheme() {
    current_theme = (current_theme == THEME_LIGHT) ? THEME_DARK : THEME_LIGHT;
    applyTheme();
    saveTheme();
}

void ThemeManager::applyTheme() {
    if (current_theme == THEME_LIGHT) {
        applyLightTheme();
    } else {
        applyDarkTheme();
    }
    CURRENT_THEME = current_theme;
}

void ThemeManager::applyLightTheme() {
    COLOR_RADAR_BG = COLOR_RADAR_BG_LIGHT;
    COLOR_RADAR_GRID = COLOR_RADAR_GRID_LIGHT;
    COLOR_PLANE_ACTIVE = COLOR_PLANE_ACTIVE_LIGHT;
    COLOR_PLANE_SELECTED = COLOR_PLANE_SELECTED_LIGHT;
    COLOR_TEXT = COLOR_TEXT_LIGHT;
    COLOR_ALTITUDE_HIGH = COLOR_ALTITUDE_HIGH_LIGHT;
    COLOR_ALTITUDE_MID = COLOR_ALTITUDE_MID_LIGHT;
    COLOR_ALTITUDE_LOW = COLOR_ALTITUDE_LOW_LIGHT;
    COLOR_BUTTON = COLOR_BUTTON_LIGHT;
    COLOR_BUTTON_ACTIVE = COLOR_BUTTON_ACTIVE_LIGHT;
}

void ThemeManager::applyDarkTheme() {
    COLOR_RADAR_BG = COLOR_RADAR_BG_DARK;
    COLOR_RADAR_GRID = COLOR_RADAR_GRID_DARK;
    COLOR_PLANE_ACTIVE = COLOR_PLANE_ACTIVE_DARK;
    COLOR_PLANE_SELECTED = COLOR_PLANE_SELECTED_DARK;
    COLOR_TEXT = COLOR_TEXT_DARK;
    COLOR_ALTITUDE_HIGH = COLOR_ALTITUDE_HIGH_DARK;
    COLOR_ALTITUDE_MID = COLOR_ALTITUDE_MID_DARK;
    COLOR_ALTITUDE_LOW = COLOR_ALTITUDE_LOW_DARK;
    COLOR_BUTTON = COLOR_BUTTON_DARK;
    COLOR_BUTTON_ACTIVE = COLOR_BUTTON_ACTIVE_DARK;
}
