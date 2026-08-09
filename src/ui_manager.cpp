#include "ui_manager.h"

UIManager::UIManager(DisplayDriver* disp, RadarEngine* rad, TouchHandler* touch_h,
                     OpenSkyAPI* api_h, LocationService* loc, WiFiManager* wifi,
                     ThemeManager* theme)
    : current_state(UI_WIFI_SETUP), display(disp), radar(rad), touch(touch_h),
      api(api_h), location(loc), wifi_mgr(wifi), theme_mgr(theme),
      last_radar_update(0), last_api_update(0), wifi_setup_start(0),
      current_radius(DEFAULT_RADIUS_KM), wifi_setup_step(0) {
    keyboard = new VirtualKeyboard();
}

void UIManager::init() {
    display->init();
    touch->init();
    theme_mgr->init();

    // Check if WiFi is already configured
    if (wifi_mgr->isConfigured()) {
        current_state = UI_RADAR;
    } else {
        current_state = UI_WIFI_SETUP;
        wifi_setup_start = millis();
        wifi_setup_step = 0;
    }
}

void UIManager::update() {
    touch->update();

    uint32_t now = millis();

    // WiFi setup timeout - go to theme select if no input
    if (current_state == UI_WIFI_SETUP && now - wifi_setup_start > WIFI_SETUP_TIMEOUT) {
        current_state = UI_THEME_SELECT;
    }

    // Update API data periodically (only if WiFi is configured and connected)
    if (current_state == UI_RADAR && now - last_api_update > OPENSKY_UPDATE_INTERVAL && api->isConnected()) {
        api->fetchAircraft(radar->getCenterLat(), radar->getCenterLon(), radar->getRadarRadius());
        radar->updateRadar(api->getAircraft());
        last_api_update = now;
    }

    // Update display periodically
    if (now - last_radar_update > RADAR_UPDATE_INTERVAL) {
        handleTouchInput();

        switch (current_state) {
            case UI_WIFI_SETUP:
                drawWiFiSetupScreen();
                break;
            case UI_THEME_SELECT:
                drawThemeSelectScreen();
                break;
            case UI_RADAR:
                drawRadarScreen();
                break;
            case UI_DETAILS:
                drawDetailsScreen();
                break;
            case UI_MAP:
                drawMapScreen();
                break;
            case UI_SETTINGS:
                drawSettingsScreen();
                break;
        }

        last_radar_update = now;
    }
}

void UIManager::setPostcode(const String& postcode) {
    if (location->lookupPostcode(postcode)) {
        Location loc = location->getLocation();
        radar->init(loc.latitude, loc.longitude);
        current_postcode = postcode;
    }
}

void UIManager::setRadiusKm(float radius) {
    current_radius = constrain(radius, MIN_RADIUS_KM, MAX_RADIUS_KM);
}

bool UIManager::isWiFiConfigured() const {
    return wifi_mgr->isConfigured();
}

void UIManager::drawButton(int x, int y, int w, int h, const char* label, bool highlighted) {
    uint16_t bg_color = highlighted ? COLOR_BUTTON_ACTIVE : COLOR_BUTTON;
    display->drawFilledRect(x, y, w, h, bg_color);
    display->drawRect(x, y, w, h, COLOR_TEXT);

    int text_x = x + (w - strlen(label) * 6) / 2;
    int text_y = y + (h - 8) / 2;
    display->drawText(text_x, text_y, label, COLOR_TEXT, 1);
}

void UIManager::drawWiFiSetupScreen() {
    display->clear();

    // Title
    display->drawFilledRect(0, 0, TFT_WIDTH, 30, COLOR_RADAR_BG);
    display->drawText(10, 8, "WiFi Setup", COLOR_TEXT, 1);

    // Step indicator
    char step_text[32];
    if (wifi_setup_step == 0) {
        snprintf(step_text, sizeof(step_text), "Step 1/2: Network Name");
    } else {
        snprintf(step_text, sizeof(step_text), "Step 2/2: Password");
    }
    display->drawText(10, 35, step_text, COLOR_TEXT, 1);

    // Instructions
    if (wifi_setup_step == 0) {
        display->drawText(10, 50, "Enter SSID:", COLOR_TEXT, 1);
        keyboard->setText(temp_ssid);
    } else {
        display->drawText(10, 50, "Enter Password:", COLOR_TEXT, 1);
        keyboard->setText(temp_password);
    }

    // Draw keyboard
    keyboard->draw(*display);

    // Draw buttons
    drawButton(10, 220, 50, 18, "NEXT", false);
    drawButton(170, 220, 50, 18, "SKIP", false);
}

void UIManager::drawThemeSelectScreen() {
    display->clear();

    display->drawFilledRect(0, 0, TFT_WIDTH, 30, COLOR_RADAR_BG);
    display->drawText(10, 8, "Theme Select", COLOR_TEXT, 1);

    display->drawText(10, 50, "Choose theme:", COLOR_TEXT, 1);

    // Light theme button
    uint16_t light_color = (theme_mgr->getTheme() == THEME_LIGHT) ? COLOR_BUTTON_ACTIVE : COLOR_BUTTON;
    display->drawFilledRect(20, 80, 90, 40, light_color);
    display->drawRect(20, 80, 90, 40, COLOR_TEXT);
    display->drawText(35, 95, "Light", COLOR_TEXT, 1);

    // Dark theme button
    uint16_t dark_color = (theme_mgr->getTheme() == THEME_DARK) ? COLOR_BUTTON_ACTIVE : COLOR_BUTTON;
    display->drawFilledRect(130, 80, 90, 40, dark_color);
    display->drawRect(130, 80, 90, 40, COLOR_TEXT);
    display->drawText(145, 95, "Dark", COLOR_TEXT, 1);

    display->drawText(10, 140, "Current:", COLOR_TEXT, 1);
    const char* theme_name = (theme_mgr->getTheme() == THEME_LIGHT) ? "Light Mode" : "Dark Mode";
    display->drawText(10, 155, theme_name, COLOR_RADAR_GRID, 1);

    // Button
    drawButton(70, 180, 100, 20, "Start Radar", false);
}

void UIManager::drawRadarScreen() {
    radar->drawRadar(*display);

    // WiFi status
    char status_text[32];
    if (api->isConnected()) {
        snprintf(status_text, sizeof(status_text), "WiFi:OK");
    } else {
        snprintf(status_text, sizeof(status_text), "WiFi:OFF");
    }
    display->drawText(TFT_WIDTH - 60, 5, status_text, COLOR_TEXT, 1);

    // Theme indicator
    const char* theme_char = (theme_mgr->getTheme() == THEME_LIGHT) ? "L" : "D";
    display->drawText(TFT_WIDTH - 15, 5, theme_char, COLOR_TEXT, 1);

    display->drawText(5, TFT_HEIGHT - 20, "TAP:Info  DBL:Zoom", COLOR_TEXT, 1);
}

void UIManager::drawDetailsScreen() {
    auto* selected = radar->getSelectedAircraft();
    if (!selected || !selected->aircraft) {
        current_state = UI_RADAR;
        return;
    }

    Aircraft* ac = selected->aircraft;

    display->clear();

    // Draw title bar
    display->drawFilledRect(0, 0, TFT_WIDTH, 25, COLOR_RADAR_BG);
    display->drawText(10, 5, "Aircraft Details", COLOR_TEXT, 1);

    // Draw aircraft info
    char buf[64];
    int y = 35;

    snprintf(buf, sizeof(buf), "Call: %s", ac->callsign.c_str());
    display->drawText(10, y, buf, COLOR_TEXT, 1);
    y += 20;

    snprintf(buf, sizeof(buf), "ICAO: %s", ac->icao24.c_str());
    display->drawText(10, y, buf, COLOR_TEXT, 1);
    y += 20;

    snprintf(buf, sizeof(buf), "Alt: %.0f ft", ac->altitude);
    display->drawText(10, y, buf, COLOR_TEXT, 1);
    y += 20;

    snprintf(buf, sizeof(buf), "Speed: %.1f m/s", ac->velocity);
    display->drawText(10, y, buf, COLOR_TEXT, 1);
    y += 20;

    snprintf(buf, sizeof(buf), "Track: %.0f°", ac->track);
    display->drawText(10, y, buf, COLOR_TEXT, 1);
    y += 20;

    snprintf(buf, sizeof(buf), "Lat: %.4f", ac->latitude);
    display->drawText(10, y, buf, COLOR_TEXT, 1);
    y += 20;

    snprintf(buf, sizeof(buf), "Lon: %.4f", ac->longitude);
    display->drawText(10, y, buf, COLOR_TEXT, 1);
    y += 30;

    display->drawText(10, TFT_HEIGHT - 20, "TAP to return", COLOR_RADAR_GRID, 1);
}

void UIManager::drawMapScreen() {
    display->clear();

    display->drawFilledRect(0, 0, TFT_WIDTH, 25, COLOR_RADAR_BG);
    display->drawText(10, 5, "Map View", COLOR_TEXT, 1);

    char buf[64];
    snprintf(buf, sizeof(buf), "Center: %.2f, %.2f", radar->getCenterLat(), radar->getCenterLon());
    display->drawText(10, 40, buf, COLOR_TEXT, 1);

    snprintf(buf, sizeof(buf), "Radius: %.0f km", radar->getRadarRadius());
    display->drawText(10, 60, buf, COLOR_TEXT, 1);

    snprintf(buf, sizeof(buf), "Aircraft: %d", radar->getProjectedAircraft().size());
    display->drawText(10, 80, buf, COLOR_TEXT, 1);

    const char* zoom_names[] = {"250km", "100km", "50km", "25km", "10km"};
    snprintf(buf, sizeof(buf), "Zoom: %s", zoom_names[radar->getZoomLevel()]);
    display->drawText(10, 100, buf, COLOR_TEXT, 1);

    display->drawText(10, TFT_HEIGHT - 20, "TAP to return", COLOR_RADAR_GRID, 1);
}

void UIManager::drawSettingsScreen() {
    display->clear();

    display->drawFilledRect(0, 0, TFT_WIDTH, 25, COLOR_RADAR_BG);
    display->drawText(10, 5, "Settings", COLOR_TEXT, 1);

    char buf[64];
    int y = 40;

    snprintf(buf, sizeof(buf), "Postcode: %s", current_postcode.c_str());
    display->drawText(10, y, buf, COLOR_TEXT, 1);
    y += 25;

    snprintf(buf, sizeof(buf), "WiFi: %s", api->isConnected() ? "ON" : "OFF");
    display->drawText(10, y, buf, COLOR_TEXT, 1);
    y += 25;

    const char* theme = (theme_mgr->getTheme() == THEME_LIGHT) ? "Light" : "Dark";
    snprintf(buf, sizeof(buf), "Theme: %s", theme);
    display->drawText(10, y, buf, COLOR_TEXT, 1);
    y += 25;

    // Buttons
    drawButton(10, 180, 100, 20, "Toggle Theme", false);
    drawButton(120, 180, 100, 20, "Reconfigure", false);

    display->drawText(10, TFT_HEIGHT - 20, "TAP to return", COLOR_RADAR_GRID, 1);
}

void UIManager::handleTouchInput() {
    if (!touch->isTouched()) {
        return;
    }

    TouchPoint point = touch->getTouchPoint();

    switch (current_state) {
        case UI_WIFI_SETUP:
            handleWiFiSetupTouch();
            break;
        case UI_THEME_SELECT:
            handleThemeSelectTouch();
            break;
        case UI_RADAR:
            handleRadarTouch();
            break;
        case UI_DETAILS:
            handleDetailsTouch();
            break;
        case UI_MAP:
            handleMapTouch();
            break;
        case UI_SETTINGS:
            // Handle settings touches
            if (point.x > 10 && point.x < 110 && point.y > 180 && point.y < 200) {
                // Toggle theme
                theme_mgr->toggleTheme();
            } else if (point.x > 120 && point.x < 220 && point.y > 180 && point.y < 200) {
                // Reconfigure WiFi
                current_state = UI_WIFI_SETUP;
                wifi_setup_step = 0;
                temp_ssid = "";
                temp_password = "";
                wifi_setup_start = millis();
            } else {
                current_state = UI_RADAR;
            }
            break;
    }
}

void UIManager::handleRadarTouch() {
    TouchPoint point = touch->getTouchPoint();

    // Check if double-tap (zoom)
    if (touch->isDoubleTap()) {
        radar->zoomIn();
        return;
    }

    // Check if tapping on aircraft
    radar->selectAircraft(point.x, point.y);
    if (radar->getSelectedIdx() >= 0) {
        current_state = UI_DETAILS;
    }

    // Check for long press or swipe
    int dx, dy;
    int delta = touch->getTouchDelta(dx, dy);
    if (delta > 30) {
        radar->panRadar(dx, dy);
    }

    // Check for bottom menu taps
    if (point.y > 210) {
        current_state = UI_SETTINGS;
    }
}

void UIManager::handleDetailsTouch() {
    current_state = UI_RADAR;
    radar->selectAircraft(-1000, -1000);
}

void UIManager::handleMapTouch() {
    current_state = UI_RADAR;
}

void UIManager::handleWiFiSetupTouch() {
    TouchPoint point = touch->getTouchPoint();

    // Keyboard input
    if (keyboard->handleInput(point)) {
        return;
    }

    // Next button
    if (point.x > 10 && point.x < 60 && point.y > 220 && point.y < 238) {
        if (wifi_setup_step == 0) {
            temp_ssid = keyboard->getText();
            if (temp_ssid.length() > 0) {
                wifi_setup_step = 1;
                keyboard->clear();
            }
        } else {
            temp_password = keyboard->getText();
            wifi_mgr->saveCredentials(temp_ssid, temp_password);
            current_state = UI_THEME_SELECT;
        }
    }

    // Skip button
    if (point.x > 170 && point.x < 220 && point.y > 220 && point.y < 238) {
        if (wifi_setup_step == 0) {
            wifi_setup_step = 1;
            keyboard->clear();
        } else {
            current_state = UI_THEME_SELECT;
        }
    }
}

void UIManager::handleThemeSelectTouch() {
    TouchPoint point = touch->getTouchPoint();

    // Light theme button
    if (point.x > 20 && point.x < 110 && point.y > 80 && point.y < 120) {
        theme_mgr->setTheme(THEME_LIGHT);
    }
    // Dark theme button
    else if (point.x > 130 && point.x < 220 && point.y > 80 && point.y < 120) {
        theme_mgr->setTheme(THEME_DARK);
    }
    // Start button
    else if (point.x > 70 && point.x < 170 && point.y > 180 && point.y < 200) {
        // Try to connect to WiFi if configured
        if (wifi_mgr->isConfigured()) {
            wifi_mgr->connect();
        }
        current_state = UI_RADAR;

        // Set default location
        location->setManualLocation(51.5074, -0.1278);  // London default
        radar->init(51.5074, -0.1278);
    }
}
