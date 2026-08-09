#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include "config.h"

struct WiFiCredentials {
    String ssid;
    String password;
    bool configured;
};

class WiFiManager {
private:
    WiFiCredentials credentials;
    uint32_t last_connection_attempt;
    int connection_attempts;

public:
    WiFiManager();
    bool initSPIFFS();
    bool loadCredentials();
    bool saveCredentials(const String& ssid, const String& password);
    bool deleteCredentials();
    bool isConfigured() const { return credentials.configured; }
    String getSSID() const { return credentials.ssid; }
    String getPassword() const { return credentials.password; }

    bool connect();
    bool isConnected() const;
    void disconnect();

    bool hasValidCredentials() const;
    int getSignalStrength() const;
    int getConnectionAttempts() const { return connection_attempts; }
    void resetAttempts() { connection_attempts = 0; }

private:
    bool parseCredentialsFile(const String& json_data);
    String generateCredentialsJSON();
    bool fileExists(const char* path);
};

#endif
