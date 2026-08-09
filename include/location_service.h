#ifndef LOCATION_SERVICE_H
#define LOCATION_SERVICE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

struct Location {
    float latitude;
    float longitude;
    String postcode;
    String city;
    String country;
    bool valid;
};

class LocationService {
private:
    Location current_location;
    HTTPClient http_client;

public:
    LocationService();
    bool lookupPostcode(const String& postcode, const String& country = "GB");
    Location getLocation() const { return current_location; }
    bool setManualLocation(float lat, float lon);

private:
    bool parseLocationResponse(const String& response);
};

#endif
