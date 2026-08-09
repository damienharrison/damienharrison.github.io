#include "location_service.h"

LocationService::LocationService() {
    current_location = {0, 0, "", "", "", false};
}

bool LocationService::lookupPostcode(const String& postcode, const String& country) {
    // Using Nominatim OpenStreetMap API for postcode lookup
    String url = "https://nominatim.openstreetmap.org/search?postalcode=" + postcode +
                 "&country=" + country + "&format=json&limit=1";

    http_client.begin(url);
    http_client.setTimeout(OPENSKY_API_TIMEOUT);

    int http_code = http_client.GET();
    bool success = false;

    if (http_code == HTTP_CODE_OK) {
        String payload = http_client.getString();
        success = parseLocationResponse(payload);
    }

    http_client.end();
    return success;
}

bool LocationService::parseLocationResponse(const String& response) {
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, response);

    if (error || doc.size() == 0) {
        current_location.valid = false;
        return false;
    }

    JsonObject result = doc[0];

    current_location.latitude = result["lat"].as<float>();
    current_location.longitude = result["lon"].as<float>();
    current_location.postcode = result["address"]["postcode"] | "";
    current_location.city = result["address"]["city"] | result["address"]["town"] | "";
    current_location.country = result["address"]["country"] | "";
    current_location.valid = true;

    return true;
}

bool LocationService::setManualLocation(float lat, float lon) {
    current_location.latitude = lat;
    current_location.longitude = lon;
    current_location.valid = true;
    return true;
}
