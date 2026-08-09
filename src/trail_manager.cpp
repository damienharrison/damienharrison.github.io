#include "trail_manager.h"
#include <math.h>

TrailManager::TrailManager() {}

void TrailManager::addTrailPoint(const String& icao24, float latitude, float longitude, float altitude) {
    if (!enabled) {
        return;
    }

    // Prune old trails periodically
    uint32_t now = millis();
    if (now - last_prune > 60000) {  // Prune every minute
        pruneOldTrails();
        last_prune = now;
    }

    // Check if aircraft trail exists
    auto it = trails.find(icao24);

    if (it == trails.end()) {
        // New aircraft - create trail
        if (trails.size() >= MAX_TRAILS) {
            // Remove oldest trail if at capacity
            auto oldest_it = trails.begin();
            uint32_t oldest_time = oldest_it->second.last_point_time;

            for (auto it2 = trails.begin(); it2 != trails.end(); ++it2) {
                if (it2->second.last_point_time < oldest_time) {
                    oldest_time = it2->second.last_point_time;
                    oldest_it = it2;
                }
            }
            trails.erase(oldest_it);
        }

        AircraftTrail new_trail;
        new_trail.icao24 = icao24;
        new_trail.last_point_time = now;

        TrailPoint point;
        point.latitude = latitude;
        point.longitude = longitude;
        point.altitude = altitude;
        point.timestamp = now;
        new_trail.points.push_back(point);

        trails[icao24] = new_trail;
    } else {
        // Existing aircraft
        AircraftTrail& trail = it->second;

        // Only add point if enough time has passed (avoid too many points)
        if (now - trail.last_point_time >= POINT_INTERVAL) {
            TrailPoint point;
            point.latitude = latitude;
            point.longitude = longitude;
            point.altitude = altitude;
            point.timestamp = now;

            trail.points.push_back(point);
            trail.last_point_time = now;

            // Limit trail length
            limitTrailLength(trail);
        }
    }
}

AircraftTrail* TrailManager::getTrail(const String& icao24) {
    auto it = trails.find(icao24);
    if (it != trails.end()) {
        return &it->second;
    }
    return nullptr;
}

void TrailManager::clearTrail(const String& icao24) {
    trails.erase(icao24);
}

void TrailManager::clearAllTrails() {
    trails.clear();
}

void TrailManager::pruneOldTrails() {
    uint32_t now = millis();
    std::vector<String> to_remove;

    for (auto& pair : trails) {
        if (now - pair.second.last_point_time > TRAIL_TIMEOUT) {
            to_remove.push_back(pair.first);
        }
    }

    for (const auto& icao : to_remove) {
        trails.erase(icao);
    }

    if (to_remove.size() > 0) {
        Serial.printf("Pruned %d old trails\n", to_remove.size());
    }
}

int TrailManager::getPointCount(const String& icao24) {
    auto trail = getTrail(icao24);
    if (trail) {
        return trail->points.size();
    }
    return 0;
}

void TrailManager::limitTrailLength(AircraftTrail& trail) {
    if (trail.points.size() > MAX_TRAIL_POINTS) {
        // Remove oldest points, keep only MAX_TRAIL_POINTS
        int to_remove = trail.points.size() - MAX_TRAIL_POINTS;
        trail.points.erase(trail.points.begin(), trail.points.begin() + to_remove);
    }
}
