#include "wifi_manager.h"

WiFiManager::WiFiManager() : last_connection_attempt(0), connection_attempts(0) {
    credentials = {"", "", false};
}

bool WiFiManager::initSPIFFS() {
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS mount failed");
        return false;
    }
    Serial.println("SPIFFS mounted successfully");
    return true;
}

bool WiFiManager::loadCredentials() {
    if (!initSPIFFS()) {
        return false;
    }

    if (!fileExists(WIFI_CONFIG_FILE)) {
        Serial.println("WiFi config file not found");
        credentials.configured = false;
        return false;
    }

    File file = SPIFFS.open(WIFI_CONFIG_FILE, "r");
    if (!file) {
        Serial.println("Failed to open WiFi config file");
        return false;
    }

    String content = "";
    while (file.available()) {
        content += (char)file.read();
    }
    file.close();

    return parseCredentialsFile(content);
}

bool WiFiManager::parseCredentialsFile(const String& json_data) {
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, json_data);

    if (error) {
        Serial.printf("JSON parse error: %s\n", error.c_str());
        return false;
    }

    credentials.ssid = doc["ssid"].as<String>();
    credentials.password = doc["password"].as<String>();
    credentials.configured = true;

    if (credentials.ssid.length() == 0) {
        credentials.configured = false;
        return false;
    }

    return true;
}

bool WiFiManager::saveCredentials(const String& ssid, const String& password) {
    if (ssid.length() == 0) {
        return false;
    }

    if (!initSPIFFS()) {
        return false;
    }

    credentials.ssid = ssid;
    credentials.password = password;
    credentials.configured = true;

    String json = generateCredentialsJSON();

    File file = SPIFFS.open(WIFI_CONFIG_FILE, "w");
    if (!file) {
        Serial.println("Failed to open WiFi config file for writing");
        return false;
    }

    if (!file.print(json)) {
        Serial.println("Failed to write to WiFi config file");
        file.close();
        return false;
    }

    file.close();
    Serial.println("WiFi credentials saved successfully");
    return true;
}

bool WiFiManager::deleteCredentials() {
    if (!initSPIFFS()) {
        return false;
    }

    if (SPIFFS.remove(WIFI_CONFIG_FILE)) {
        credentials = {"", "", false};
        Serial.println("WiFi credentials deleted");
        return true;
    }
    return false;
}

String WiFiManager::generateCredentialsJSON() {
    DynamicJsonDocument doc(512);
    doc["ssid"] = credentials.ssid;
    doc["password"] = credentials.password;

    String json;
    serializeJson(doc, json);
    return json;
}

bool WiFiManager::fileExists(const char* path) {
    return SPIFFS.exists(path);
}

bool WiFiManager::connect() {
    if (!credentials.configured || credentials.ssid.length() == 0) {
        return false;
    }

    // Avoid connection spam
    if (millis() - last_connection_attempt < 5000) {
        return false;
    }

    last_connection_attempt = millis();
    connection_attempts++;

    WiFi.mode(WIFI_STA);
    WiFi.begin(credentials.ssid.c_str(), credentials.password.c_str());

    // Wait up to 10 seconds for connection
    int timeout = 20;  // 20 * 500ms = 10 seconds
    while (!isConnected() && timeout > 0) {
        delay(500);
        timeout--;
    }

    return isConnected();
}

bool WiFiManager::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

void WiFiManager::disconnect() {
    WiFi.disconnect(true);  // true = turn off WiFi radio
}

bool WiFiManager::hasValidCredentials() const {
    return credentials.configured && credentials.ssid.length() > 0;
}

int WiFiManager::getSignalStrength() const {
    if (!isConnected()) {
        return 0;
    }
    // Convert RSSI (-30 to -90 dBm) to percentage (0-100)
    int rssi = WiFi.RSSI();
    return constrain(2 * (rssi + 100), 0, 100);
}
