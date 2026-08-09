#include "radar_engine.h"
#include <math.h>
#include <algorithm>

// Zoom levels: 0=250km, 1=100km, 2=50km, 3=25km, 4=10km
int zoom_radius[ZOOM_LEVEL_COUNT] = {250, 100, 50, 25, 10};

RadarEngine::RadarEngine()
    : center_latitude(0), center_longitude(0), radar_radius_km(50),
      zoom_level(DEFAULT_ZOOM), pan_x(0), pan_y(0), selected_aircraft_idx(-1) {}

void RadarEngine::init(float lat, float lon) {
    center_latitude = lat;
    center_longitude = lon;
    radar_radius_km = zoom_radius[zoom_level];
}

int RadarEngine::getRadiusForZoom(int zoom) {
    if (zoom >= 0 && zoom < ZOOM_LEVEL_COUNT) {
        return zoom_radius[zoom];
    }
    return 50;
}

float RadarEngine::kmToPixels(float km) {
    return (km / radar_radius_km) * RADIUS;
}

void RadarEngine::projectAircraft(Aircraft& aircraft, int& x, int& y) {
    // Calculate distance and bearing from center
    float dLat = aircraft.latitude - center_latitude;
    float dLon = aircraft.longitude - center_longitude;

    // Convert to km
    float lat_km = dLat * 111.0;  // 1 degree latitude = ~111 km
    float lon_km = dLon * 111.0 * cos(center_latitude * M_PI / 180.0);

    // Project onto radar (simple Cartesian projection)
    float px = kmToPixels(lon_km);
    float py = -kmToPixels(lat_km);  // Negative because screen Y increases downward

    // Apply pan offset
    px += pan_x;
    py += pan_y;

    // Convert to screen coordinates
    x = CENTER_X + (int)px;
    y = CENTER_Y + (int)py;
}

void RadarEngine::updateRadar(std::vector<Aircraft>& aircraft) {
    projected_aircraft.clear();

    for (auto& a : aircraft) {
        int x, y;
        projectAircraft(a, x, y);

        // Check if within display bounds (with some margin)
        bool visible = (x >= 0 && x < TFT_WIDTH && y >= 0 && y < TFT_HEIGHT);

        // More accurately, check if within circular radar display
        int dx = x - CENTER_X;
        int dy = y - CENTER_Y;
        visible = (dx * dx + dy * dy) <= (RADIUS * RADIUS);

        ProjectedAircraft proj;
        proj.aircraft = &a;
        proj.screen_x = x;
        proj.screen_y = y;
        proj.visible = visible;

        // Calculate distance from center for sorting
        float dist_km = sqrt(dx * dx + dy * dy) * (radar_radius_km / RADIUS);
        proj.distance = dist_km;

        projected_aircraft.push_back(proj);
    }

    // Sort by distance (draw closer aircraft last, on top)
    std::sort(projected_aircraft.begin(), projected_aircraft.end(),
              [](const ProjectedAircraft& a, const ProjectedAircraft& b) {
                  return a.distance > b.distance;
              });
}

void RadarEngine::drawRadar(DisplayDriver& display) {
    display.clear();
    display.drawRadarBackground();
    display.drawRadarGrid(radar_radius_km);

    // Draw all visible aircraft
    for (size_t i = 0; i < projected_aircraft.size(); i++) {
        auto& proj = projected_aircraft[i];
        if (proj.visible) {
            bool selected = (i == selected_aircraft_idx);
            display.drawPlane(proj.screen_x, proj.screen_y,
                            proj.aircraft->track,
                            selected,
                            proj.aircraft->altitude);
        }
    }

    // Draw info overlay
    char info_buf[64];
    snprintf(info_buf, sizeof(info_buf), "R:%.0f %dZ A:%d",
             radar_radius_km, zoom_level, projected_aircraft.size());
    display.drawText(5, 5, info_buf, COLOR_TEXT, 1);
}

void RadarEngine::selectAircraft(int x, int y) {
    selected_aircraft_idx = -1;
    const int TAP_RADIUS = 20;

    for (size_t i = 0; i < projected_aircraft.size(); i++) {
        auto& proj = projected_aircraft[i];
        int dx = x - proj.screen_x;
        int dy = y - proj.screen_y;

        if (dx * dx + dy * dy < TAP_RADIUS * TAP_RADIUS) {
            selected_aircraft_idx = i;
            break;
        }
    }
}

void RadarEngine::zoomIn() {
    if (zoom_level < ZOOM_LEVEL_COUNT - 1) {
        zoom_level++;
        radar_radius_km = getRadiusForZoom(zoom_level);
    }
}

void RadarEngine::zoomOut() {
    if (zoom_level > 0) {
        zoom_level--;
        radar_radius_km = getRadiusForZoom(zoom_level);
    }
}

void RadarEngine::panRadar(int dx, int dy) {
    pan_x += dx;
    pan_y += dy;

    // Clamp panning to reasonable bounds
    int max_pan = RADIUS / 2;
    pan_x = constrain(pan_x, -max_pan, max_pan);
    pan_y = constrain(pan_y, -max_pan, max_pan);
}

ProjectedAircraft* RadarEngine::getSelectedAircraft() {
    if (selected_aircraft_idx >= 0 && selected_aircraft_idx < (int)projected_aircraft.size()) {
        return &projected_aircraft[selected_aircraft_idx];
    }
    return nullptr;
}
