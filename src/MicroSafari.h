/*!
 * @file MicroSafari.h
 * @brief MicroSafari ESP32 Library - Header file
 *
 * This library provides easy connectivity to the MicroSafari IoT platform
 * for ESP32 devices, handling WiFi connection, data transmission, and
 * platform communication.
 *
 * @version 1.0.0
 * @date 2025-08-22
 * @author MicroSafari Team
 */

#ifndef MICROSAFARI_H
#define MICROSAFARI_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

/**
 * @brief Response structure for MicroSafari API calls
 */
struct MicroSafariResponse {
    bool success;           // Whether the operation was successful
    int httpCode;          // HTTP status code
    String errorMessage;   // Error message if operation failed
    String responseData;   // Response data from server

    // Constructor
    MicroSafariResponse() : success(false), httpCode(0), errorMessage(""), responseData("") {}
};

/**
 * @brief Main MicroSafari library class
 */
class MicroSafari {
private:
    // Configuration
    String _wifiSSID;
    String _wifiPassword;
    String _apiKey;
    String _platformURL;
    String _deviceName;

    // State
    bool _debugEnabled;
    bool _initialized;

    // HTTP client
    HTTPClient _httpClient;

    // Internal methods
    void debugPrint(const String& message);
    void debugPrintln(const String& message);
    String createSensorDataPayload(float temperature, float humidity, float soilMoisture, float lightLevel);

public:
    /**
     * @brief Constructor
     */
    MicroSafari();
    
    /**
     * @brief Destructor
     */
    ~MicroSafari();
    
    /**
     * @brief Initialize the MicroSafari library
     * @param wifiSSID WiFi network name
     * @param wifiPassword WiFi password
     * @param apiKey Device API key from MicroSafari dashboard
     * @param platformURL URL of your MicroSafari instance
     * @param deviceName Name for this device
     * @return true if initialization successful, false otherwise
     */
    bool begin(const char* wifiSSID, const char* wifiPassword, const char* apiKey, const char* platformURL, const char* deviceName);

    /**
     * @brief Enable or disable debug output
     * @param enabled true to enable debug output, false to disable
     */
    void setDebug(bool enabled);

    /**
     * @brief Connect to WiFi network
     * @return true if connection successful, false otherwise
     */
    bool connectWiFi();

    /**
     * @brief Check if WiFi is connected
     * @return true if connected, false otherwise
     */
    bool isWiFiConnected();
    
    /**
     * @brief Test connection to MicroSafari platform
     * @return true if platform is reachable, false otherwise
     */
    bool testConnection();
    
    /**
     * @brief Send sensor data to MicroSafari platform
     * @param temperature Temperature reading in Celsius
     * @param humidity Humidity percentage (0-100)
     * @param soilMoisture Soil moisture percentage (0-100)
     * @param lightLevel Light level in lux
     * @return MicroSafariResponse with operation result
     */
    MicroSafariResponse sendSensorData(float temperature, float humidity, float soilMoisture, float lightLevel);

    /**
     * @brief Main loop function - call this in your main loop()
     */
    void loop();
    
    /**
     * @brief Get WiFi signal strength
     * @return RSSI value in dBm
     */
    int getWiFiRSSI();

    /**
     * @brief Get local IP address
     * @return IP address as string
     */
    String getLocalIP();
};

#endif // MICROSAFARI_H