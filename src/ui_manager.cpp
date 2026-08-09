#include "ui_manager.h"

UIManager::UIManager(DisplayDriver* disp, RadarEngine* rad, TouchHandler* touch_h,
                     OpenSkyAPI* api_h, LocationService* loc, WiFiManager* wifi,
                     ThemeManager* theme, TrailManager* trails, SettingsManager* settings)
    : current_state(UI_WIFI_SETUP), display(disp), radar(rad), touch(touch_h),
      api(api_h), location(loc), wifi_mgr(wifi), theme_mgr(theme), trail_mgr(trails),
      settings_mgr(settings), last_radar_update(0), last_api_update(0), wifi_setup_start(0),
      current_radius(DEFAULT_RADIUS_KM), wifi_setup_step(0), show_trails(true) {
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
        auto& aircraft_list = api->getAircraft();
        radar->updateRadar(aircraft_list);

        // Update trail positions
        if (trail_mgr) {
            for (auto& ac : aircraft_list) {
                trail_mgr->addTrailPoint(ac.icao24, ac.latitude, ac.longitude, ac.altitude);
            }
        }
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
            case UI_RADAR_SETTINGS:
                drawRadarSettingsScreen();
                break;
            case UI_TRAIL_SETTINGS:
                drawTrailSettingsScreen();
                break;
            case UI_DISPLAY_SETTINGS:
                drawDisplaySettingsScreen();
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

    // Trail indicator
    if (trail_mgr && trail_mgr->isEnabled()) {
        display->drawText(TFT_WIDTH - 40, TFT_HEIGHT - 20, "Tr:On", COLOR_TEXT, 1);
    } else {
        display->drawText(TFT_WIDTH - 40, TFT_HEIGHT - 20, "Tr:Off", COLOR_RADAR_GRID, 1);
    }

    display->drawText(5, TFT_HEIGHT - 20, "TAP:Info DBL:Zoom", COLOR_TEXT, 1);
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

    // Settings category buttons
    drawButton(10, 50, 50, 15, "Radar", false);
    drawButton(75, 50, 50, 15, "Display", false);
    drawButton(140, 50, 55, 15, "Trails", false);

    char buf[64];
    int y = 75;

    // Quick status
    snprintf(buf, sizeof(buf), "WiFi: %s", api->isConnected() ? "ON" : "OFF");
    display->drawText(10, y, buf, COLOR_TEXT, 1);
    y += 15;

    const char* theme = (theme_mgr->getTheme() == THEME_LIGHT) ? "Light" : "Dark";
    snprintf(buf, sizeof(buf), "Theme: %s", theme);
    display->drawText(10, y, buf, COLOR_TEXT, 1);
    y += 15;

    // Quick buttons
    drawButton(10, 90, 50, 15, "WiFi", false);
    drawButton(70, 90, 50, 15, "Theme", false);

    y = 125;
    snprintf(buf, sizeof(buf), "Radius: %.0f km", settings_mgr->getRadarRadius());
    display->drawText(10, y, buf, COLOR_TEXT, 1);
    y += 15;

    const char* trails_status = (trail_mgr && trail_mgr->isEnabled()) ? "On" : "Off";
    snprintf(buf, sizeof(buf), "Trails: %s", trails_status);
    display->drawText(10, y, buf, COLOR_TEXT, 1);
    y += 30;

    drawButton(10, 200, 170, 18, "Back to Radar", false);
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
            handleSettingsTouch();
            break;
        case UI_RADAR_SETTINGS:
            handleRadarSettingsTouch();
            break;
        case UI_TRAIL_SETTINGS:
            handleTrailSettingsTouch();
            break;
        case UI_DISPLAY_SETTINGS:
            handleDisplaySettingsTouch();
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

void UIManager::handleSettingsTouch() {
    TouchPoint point = touch->getTouchPoint();

    // Radar button
    if (point.x > 10 && point.x < 65 && point.y > 50 && point.y < 70) {
        current_state = UI_RADAR_SETTINGS;
    }
    // Display button
    else if (point.x > 75 && point.x < 130 && point.y > 50 && point.y < 70) {
        current_state = UI_DISPLAY_SETTINGS;
    }
    // Trails button
    else if (point.x > 140 && point.x < 195 && point.y > 50 && point.y < 70) {
        current_state = UI_TRAIL_SETTINGS;
    }
    // WiFi button
    else if (point.x > 10 && point.x < 60 && point.y > 90 && point.y < 108) {
        current_state = UI_WIFI_SETUP;
        wifi_setup_step = 0;
        temp_ssid = "";
        temp_password = "";
        wifi_setup_start = millis();
    }
    // Theme button
    else if (point.x > 70 && point.x < 120 && point.y > 90 && point.y < 108) {
        theme_mgr->toggleTheme();
    }
    // Back button
    else if (point.x > 10 && point.x < 190 && point.y > 200 && point.y < 218) {
        current_state = UI_RADAR;
    }
}

void UIManager::drawRadarSettingsScreen() {
    display->clear();

    display->drawFilledRect(0, 0, TFT_WIDTH, 25, COLOR_RADAR_BG);
    display->drawText(10, 5, "Radar Settings", COLOR_TEXT, 1);

    char buf[64];
    int y = 40;

    snprintf(buf, sizeof(buf), "Radius: %.0f km", settings_mgr->getRadarRadius());
    display->drawText(10, y, buf, COLOR_TEXT, 1);
    y += 20;

    // Decrease/Increase radius buttons
    if (settings_mgr->getRadarRadius() > MIN_RADIUS_KM) {
        drawButton(10, y, 30, 15, "-", false);
    }
    drawButton(50, y, 30, 15, "+", false);
    y += 25;

    snprintf(buf, sizeof(buf), "Lat: %.4f", settings_mgr->getCenterLatitude());
    display->drawText(10, y, buf, COLOR_TEXT, 1);
    y += 15;

    snprintf(buf, sizeof(buf), "Lon: %.4f", settings_mgr->getCenterLongitude());
    display->drawText(10, y, buf, COLOR_TEXT, 1);
    y += 30;

    drawButton(10, TFT_HEIGHT - 25, 170, 18, "Back to Settings", false);
    display->drawText(10, TFT_HEIGHT - 5, "Tap +/- to adjust radius", COLOR_RADAR_GRID, 1);
}

void UIManager::drawTrailSettingsScreen() {
    display->clear();

    display->drawFilledRect(0, 0, TFT_WIDTH, 25, COLOR_RADAR_BG);
    display->drawText(10, 5, "Trail Settings", COLOR_TEXT, 1);

    char buf[64];
    int y = 40;

    const char* trails_status = settings_mgr->areTrailsEnabled() ? "On" : "Off";
    snprintf(buf, sizeof(buf), "Trails: %s", trails_status);
    display->drawText(10, y, buf, COLOR_TEXT, 1);
    y += 20;

    drawButton(10, y, 60, 15, "Toggle", false);
    y += 25;

    snprintf(buf, sizeof(buf), "Points: %d", settings_mgr->getMaxTrailPoints());
    display->drawText(10, y, buf, COLOR_TEXT, 1);
    y += 15;

    drawButton(10, y, 20, 15, "-", false);
    drawButton(35, y, 20, 15, "+", false);
    y += 25;

    snprintf(buf, sizeof(buf), "Max Aircraft: %d", settings_mgr->getMaxTrails());
    display->drawText(10, y, buf, COLOR_TEXT, 1);
    y += 15;

    drawButton(10, y, 20, 15, "-", false);
    drawButton(35, y, 20, 15, "+", false);
    y += 30;

    drawButton(10, TFT_HEIGHT - 25, 170, 18, "Back to Settings", false);
}

void UIManager::drawDisplaySettingsScreen() {
    display->clear();

    display->drawFilledRect(0, 0, TFT_WIDTH, 25, COLOR_RADAR_BG);
    display->drawText(10, 5, "Display Settings", COLOR_TEXT, 1);

    char buf[64];
    int y = 40;

    const char* theme = (theme_mgr->getTheme() == THEME_LIGHT) ? "Light" : "Dark";
    snprintf(buf, sizeof(buf), "Theme: %s", theme);
    display->drawText(10, y, buf, COLOR_TEXT, 1);
    y += 20;

    drawButton(10, y, 70, 15, "Toggle Theme", false);
    y += 25;

    snprintf(buf, sizeof(buf), "Brightness: %d%%", (settings_mgr->getBrightness() * 100) / 255);
    display->drawText(10, y, buf, COLOR_TEXT, 1);
    y += 15;

    drawButton(10, y, 20, 15, "-", false);
    drawButton(35, y, 20, 15, "+", false);
    y += 25;

    snprintf(buf, sizeof(buf), "Grid Opacity: %d%%", (settings_mgr->getGridOpacity() * 100) / 255);
    display->drawText(10, y, buf, COLOR_TEXT, 1);
    y += 15;

    drawButton(10, y, 20, 15, "-", false);
    drawButton(35, y, 20, 15, "+", false);
    y += 30;

    drawButton(10, TFT_HEIGHT - 25, 170, 18, "Back to Settings", false);
}

void UIManager::handleRadarSettingsTouch() {
    TouchPoint point = touch->getTouchPoint();

    // Decrease radius
    if (point.x > 10 && point.x < 40 && point.y > 60 && point.y < 75) {
        float new_radius = settings_mgr->getRadarRadius() - 10.0f;
        if (new_radius >= MIN_RADIUS_KM) {
            settings_mgr->setRadarRadius(new_radius);
            radar->setRadiusKm(new_radius);
        }
    }
    // Increase radius
    else if (point.x > 50 && point.x < 80 && point.y > 60 && point.y < 75) {
        float new_radius = settings_mgr->getRadarRadius() + 10.0f;
        if (new_radius <= MAX_RADIUS_KM) {
            settings_mgr->setRadarRadius(new_radius);
            radar->setRadiusKm(new_radius);
        }
    }
    // Back button
    else if (point.x > 10 && point.x < 190 && point.y > (TFT_HEIGHT - 25) && point.y < TFT_HEIGHT) {
        current_state = UI_SETTINGS;
    }
}

void UIManager::handleTrailSettingsTouch() {
    TouchPoint point = touch->getTouchPoint();

    // Toggle trails
    if (point.x > 10 && point.x < 70 && point.y > 60 && point.y < 75) {
        settings_mgr->setTrailsEnabled(!settings_mgr->areTrailsEnabled());
        if (trail_mgr) {
            trail_mgr->setEnabled(settings_mgr->areTrailsEnabled());
        }
    }
    // Decrease trail points
    else if (point.x > 10 && point.x < 30 && point.y > 90 && point.y < 105) {
        int new_points = settings_mgr->getMaxTrailPoints() - 10;
        settings_mgr->setMaxTrailPoints(new_points);
    }
    // Increase trail points
    else if (point.x > 35 && point.x < 55 && point.y > 90 && point.y < 105) {
        int new_points = settings_mgr->getMaxTrailPoints() + 10;
        settings_mgr->setMaxTrailPoints(new_points);
    }
    // Decrease max aircraft
    else if (point.x > 10 && point.x < 30 && point.y > 120 && point.y < 135) {
        int new_trails = settings_mgr->getMaxTrails() - 5;
        settings_mgr->setMaxTrails(new_trails);
    }
    // Increase max aircraft
    else if (point.x > 35 && point.x < 55 && point.y > 120 && point.y < 135) {
        int new_trails = settings_mgr->getMaxTrails() + 5;
        settings_mgr->setMaxTrails(new_trails);
    }
    // Back button
    else if (point.x > 10 && point.x < 190 && point.y > (TFT_HEIGHT - 25) && point.y < TFT_HEIGHT) {
        current_state = UI_SETTINGS;
    }
}

void UIManager::handleDisplaySettingsTouch() {
    TouchPoint point = touch->getTouchPoint();

    // Toggle theme
    if (point.x > 10 && point.x < 80 && point.y > 60 && point.y < 75) {
        theme_mgr->toggleTheme();
        settings_mgr->setTheme(theme_mgr->getTheme());
    }
    // Decrease brightness
    else if (point.x > 10 && point.x < 30 && point.y > 90 && point.y < 105) {
        int new_brightness = settings_mgr->getBrightness() - 25;
        settings_mgr->setBrightness(new_brightness);
    }
    // Increase brightness
    else if (point.x > 35 && point.x < 55 && point.y > 90 && point.y < 105) {
        int new_brightness = settings_mgr->getBrightness() + 25;
        settings_mgr->setBrightness(new_brightness);
    }
    // Decrease grid opacity
    else if (point.x > 10 && point.x < 30 && point.y > 120 && point.y < 135) {
        int new_opacity = settings_mgr->getGridOpacity() - 25;
        settings_mgr->setGridOpacity(new_opacity);
    }
    // Increase grid opacity
    else if (point.x > 35 && point.x < 55 && point.y > 120 && point.y < 135) {
        int new_opacity = settings_mgr->getGridOpacity() + 25;
        settings_mgr->setGridOpacity(new_opacity);
    }
    // Back button
    else if (point.x > 10 && point.x < 190 && point.y > (TFT_HEIGHT - 25) && point.y < TFT_HEIGHT) {
        current_state = UI_SETTINGS;
    }
}
