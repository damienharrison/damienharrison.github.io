#ifndef TRAIL_MANAGER_H
#define TRAIL_MANAGER_H

#include <Arduino.h>
#include <vector>
#include <map>

struct TrailPoint {
    float latitude;
    float longitude;
    uint32_t timestamp;
    float altitude;  // altitude at this point
};

struct AircraftTrail {
    String icao24;
    std::vector<TrailPoint> points;
    uint32_t last_point_time;
};

class TrailManager {
private:
    static constexpr int MAX_TRAIL_POINTS = 60;  // Store last 60 positions (5 min at 5 sec updates)
    static constexpr int MAX_TRAILS = 50;        // Track up to 50 aircraft
    static constexpr uint32_t POINT_INTERVAL = 5000;  // Add point every 5 seconds (matches API update)
    static constexpr uint32_t TRAIL_TIMEOUT = 600000; // Clear trail after 10 minutes of inactivity

    std::map<String, AircraftTrail> trails;

public:
    TrailManager();

    // Add or update a position for aircraft
    void addTrailPoint(const String& icao24, float latitude, float longitude, float altitude);

    // Get trail for specific aircraft
    AircraftTrail* getTrail(const String& icao24);
    std::map<String, AircraftTrail>& getAllTrails() { return trails; }

    // Clear specific trail
    void clearTrail(const String& icao24);

    // Clear all trails
    void clearAllTrails();

    // Remove old trails (call periodically)
    void pruneOldTrails();

    // Get trail statistics
    int getTrailCount() { return trails.size(); }
    int getPointCount(const String& icao24);

    // Enable/disable trails
    bool isEnabled() const { return enabled; }
    void setEnabled(bool state) { enabled = state; }
    void toggleEnabled() { enabled = !enabled; }

private:
    bool enabled = true;
    uint32_t last_prune = 0;

    void limitTrailLength(AircraftTrail& trail);
};

#endif
