#ifndef RADAR_ENGINE_H
#define RADAR_ENGINE_H

#include <Arduino.h>
#include "config.h"
#include "opensky_api.h"
#include "display_driver.h"
#include "trail_manager.h"
#include <vector>

struct ProjectedAircraft {
    Aircraft* aircraft;
    int screen_x;
    int screen_y;
    float distance;
    bool visible;
};

class RadarEngine {
private:
    float center_latitude;
    float center_longitude;
    float radar_radius_km;
    int zoom_level;
    float pan_x, pan_y;
    int selected_aircraft_idx;
    std::vector<ProjectedAircraft> projected_aircraft;
    TrailManager* trail_manager;

public:
    RadarEngine();
    void init(float lat, float lon);
    void setTrailManager(TrailManager* tm) { trail_manager = tm; }
    void updateRadar(std::vector<Aircraft>& aircraft);
    void projectAircraft(Aircraft& aircraft, int& x, int& y);
    void drawRadar(DisplayDriver& display);
    void drawTrails(DisplayDriver& display);
    void selectAircraft(int x, int y);
    void zoomIn();
    void zoomOut();
    void panRadar(int dx, int dy);

    // Getters
    float getCenterLat() { return center_latitude; }
    float getCenterLon() { return center_longitude; }
    float getRadarRadius() { return radar_radius_km; }
    int getZoomLevel() { return zoom_level; }
    int getSelectedIdx() { return selected_aircraft_idx; }
    ProjectedAircraft* getSelectedAircraft();
    std::vector<ProjectedAircraft>& getProjectedAircraft() { return projected_aircraft; }

private:
    float kmToPixels(float km);
    int getRadiusForZoom(int zoom);
};

#endif
