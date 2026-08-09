#include "opensky_api.h"
#include <math.h>

OpenSkyAPI::OpenSkyAPI() : last_fetch(0), is_connected(false) {}

bool OpenSkyAPI::connect() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        attempts++;
    }

    is_connected = (WiFi.status() == WL_CONNECTED);
    return is_connected;
}

bool OpenSkyAPI::fetchAircraft(float lat, float lon, float radius_km) {
    if (!is_connected) {
        return false;
    }

    if (millis() - last_fetch < OPENSKY_API_TIMEOUT) {
        return false;  // Rate limiting
    }

    String url = String(OPENSKY_API_URL);
    url += "?lamin=" + String(lat - (radius_km / 111.0), 6);
    url += "&lamax=" + String(lat + (radius_km / 111.0), 6);
    url += "&lomin=" + String(lon - (radius_km / (111.0 * cos(lat * M_PI / 180.0))), 6);
    url += "&lomax=" + String(lon + (radius_km / (111.0 * cos(lat * M_PI / 180.0))), 6);

    http_client.begin(url);
    http_client.setTimeout(OPENSKY_API_TIMEOUT);

    int http_code = http_client.GET();
    bool success = false;

    if (http_code == HTTP_CODE_OK) {
        String payload = http_client.getString();
        success = parseAircraftData(payload, lat, lon, radius_km);
    }

    http_client.end();
    last_fetch = millis();
    return success;
}

bool OpenSkyAPI::parseAircraftData(const String& json_data, float center_lat, float center_lon, float radius_km) {
    DynamicJsonDocument doc(24576);  // ~24KB for aircraft data
    DeserializationError error = deserializeJson(doc, json_data);

    if (error) {
        return false;
    }

    aircraft_list.clear();

    JsonArray states = doc["states"];
    if (!states.isNull()) {
        for (JsonVariant state : states) {
            if (state[1].isNull() || state[2].isNull() || state[5].isNull()) {
                continue;  // Skip incomplete data
            }

            Aircraft aircraft;
            aircraft.icao24 = state[0].as<String>();
            aircraft.callsign = state[1].as<String>();
            aircraft.longitude = state[5].as<float>();
            aircraft.latitude = state[6].as<float>();
            aircraft.altitude = state[7].isNull() ? 0 : state[7].as<float>();
            aircraft.on_ground = state[8].as<bool>();
            aircraft.velocity = state[9].isNull() ? 0 : state[9].as<float>();
            aircraft.track = state[10].isNull() ? 0 : state[10].as<float>();
            aircraft.last_update = millis();

            // Filter by distance
            float distance = calculateDistance(center_lat, center_lon, aircraft.latitude, aircraft.longitude);
            if (distance <= radius_km) {
                aircraft_list.push_back(aircraft);
            }
        }
    }

    return true;
}

float OpenSkyAPI::calculateDistance(float lat1, float lon1, float lat2, float lon2) {
    // Haversine formula
    float R = 6371.0;  // Earth's radius in km
    float dLat = (lat2 - lat1) * M_PI / 180.0;
    float dLon = (lon2 - lon1) * M_PI / 180.0;
    float a = sin(dLat / 2.0) * sin(dLat / 2.0) +
              cos(lat1 * M_PI / 180.0) * cos(lat2 * M_PI / 180.0) * sin(dLon / 2.0) * sin(dLon / 2.0);
    float c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
    return R * c;
}

float OpenSkyAPI::calculateBearing(float lat1, float lon1, float lat2, float lon2) {
    float dLon = (lon2 - lon1) * M_PI / 180.0;
    float y = sin(dLon) * cos(lat2 * M_PI / 180.0);
    float x = cos(lat1 * M_PI / 180.0) * sin(lat2 * M_PI / 180.0) -
              sin(lat1 * M_PI / 180.0) * cos(lat2 * M_PI / 180.0) * cos(dLon);
    float bearing = atan2(y, x) * 180.0 / M_PI;
    return fmod(bearing + 360.0, 360.0);
}
