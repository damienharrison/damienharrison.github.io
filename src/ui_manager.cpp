#include "ui_manager.h"

UIManager::UIManager(DisplayDriver* disp, RadarEngine* rad, TouchHandler* touch_h,
                     OpenSkyAPI* api_h, LocationService* loc)
    : current_state(UI_RADAR), display(disp), radar(rad), touch(touch_h),
      api(api_h), location(loc), last_radar_update(0), last_api_update(0),
      current_radius(DEFAULT_RADIUS_KM) {}

void UIManager::init() {
    display->init();
    touch->init();
}

void UIManager::update() {
    touch->update();

    uint32_t now = millis();

    // Update API data periodically
    if (now - last_api_update > OPENSKY_UPDATE_INTERVAL && api->isConnected()) {
        api->fetchAircraft(radar->getCenterLat(), radar->getCenterLon(), radar->getRadarRadius());
        radar->updateRadar(api->getAircraft());
        last_api_update = now;
    }

    // Update radar display periodically
    if (now - last_radar_update > RADAR_UPDATE_INTERVAL) {
        handleTouchInput();

        switch (current_state) {
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
    // Update radar based on new radius - zoom will be adjusted
}

void UIManager::drawRadarScreen() {
    radar->drawRadar(*display);

    // Draw control hints at bottom
    display->drawText(5, TFT_HEIGHT - 20, "TAP:Info  ZOOM:+/-", COLOR_TEXT, 1);
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

    // Draw back button
    display->drawText(10, TFT_HEIGHT - 20, "TAP to return", COLOR_RADAR_GRID, 1);
}

void UIManager::drawMapScreen() {
    display->clear();

    // Draw title
    display->drawFilledRect(0, 0, TFT_WIDTH, 25, COLOR_RADAR_BG);
    display->drawText(10, 5, "Map View", COLOR_TEXT, 1);

    // Draw simple map representation
    char buf[64];
    snprintf(buf, sizeof(buf), "Center: %.2f, %.2f", radar->getCenterLat(), radar->getCenterLon());
    display->drawText(10, 40, buf, COLOR_TEXT, 1);

    snprintf(buf, sizeof(buf), "Radius: %.0f km", radar->getRadarRadius());
    display->drawText(10, 60, buf, COLOR_TEXT, 1);

    snprintf(buf, sizeof(buf), "Aircraft: %d", radar->getProjectedAircraft().size());
    display->drawText(10, 80, buf, COLOR_TEXT, 1);

    // Draw zoom level
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

    display->drawText(10, y, "Updates available via UI", COLOR_RADAR_GRID, 1);

    display->drawText(10, TFT_HEIGHT - 20, "TAP to return", COLOR_RADAR_GRID, 1);
}

void UIManager::handleTouchInput() {
    if (!touch->isTouched()) {
        return;
    }

    TouchPoint point = touch->getTouchPoint();

    switch (current_state) {
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
            current_state = UI_RADAR;
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

    // Check for long press or swipe (for pan/zoom)
    int dx, dy;
    int delta = touch->getTouchDelta(dx, dy);
    if (delta > 30) {
        // Panning gesture
        radar->panRadar(dx, dy);
    }
}

void UIManager::handleDetailsTouch() {
    current_state = UI_RADAR;
    radar->selectAircraft(-1000, -1000);  // Deselect
}

void UIManager::handleMapTouch() {
    current_state = UI_RADAR;
}
