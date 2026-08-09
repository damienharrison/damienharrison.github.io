#ifndef OPENSKY_API_H
#define OPENSKY_API_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <vector>

struct Aircraft {
    String callsign;
    String icao24;
    float latitude;
    float longitude;
    float altitude;  // in feet
    float velocity;  // m/s
    float track;     // heading in degrees (0-360)
    bool on_ground;
    uint32_t last_update;
};

class OpenSkyAPI {
private:
    HTTPClient http_client;
    std::vector<Aircraft> aircraft_list;
    uint32_t last_fetch;
    bool is_connected;

public:
    OpenSkyAPI();
    bool connect();
    bool fetchAircraft(float lat, float lon, float radius_km);
    std::vector<Aircraft>& getAircraft() { return aircraft_list; }
    bool isConnected() { return is_connected; }
    uint32_t getLastFetch() { return last_fetch; }
    size_t getAircraftCount() { return aircraft_list.size(); }

private:
    bool parseAircraftData(const String& json_data, float center_lat, float center_lon, float radius_km);
    float calculateDistance(float lat1, float lon1, float lat2, float lon2);
    float calculateBearing(float lat1, float lon1, float lat2, float lon2);
};

#endif
