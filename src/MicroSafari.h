/*!
 * @file MicroSafari.h
 * @brief Main header file for MicroSafari ESP32 Arduino Library
 * @version 1.0.0
 * @date 2025-08-22
 * 
 * This library provides WiFi connectivity and HTTP communication
 * capabilities for ESP32 devices to connect to the MicroSafari
 * IoT SaaS platform for Indonesian millennial farmers.
 * 
 * @section dependencies Dependencies
 * - WiFi library (ESP32)
 * - HTTPClient library (ESP32)
 * - ArduinoJson library
 * 
 * @section author Author
 * MicroSafari Team
 * 
 * @section license License
 * MIT License
 */

#ifndef MICROSAFARI_H
#define MICROSAFARI_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

/**
 * @brief Connection status enumeration
 */
enum MicroSafariStatus {
    MICROSAFARI_DISCONNECTED = 0,
    MICROSAFARI_WIFI_CONNECTING = 1,
    MICROSAFARI_WIFI_CONNECTED = 2,
    MICROSAFARI_PLATFORM_CONNECTED = 3,
    MICROSAFARI_ERROR = 4
};

/**
 * @brief HTTP response structure
 */
struct MicroSafariResponse {
    int httpCode;
    String payload;
    bool success;
    String errorMessage;
};

/**
 * @brief Main MicroSafari class for ESP32 connectivity
 */
class MicroSafari {
private:
    String _ssid;                    ///< WiFi SSID
    String _password;                ///< WiFi password
    String _apiKey;                  ///< Device API key
    String _platformUrl;             ///< MicroSafari platform URL
    String _deviceName;              ///< Device identifier name
    
    WiFiClientSecure _wifiClient;    ///< Secure WiFi client
    HTTPClient _httpClient;          ///< HTTP client instance
    
    MicroSafariStatus _status;       ///< Current connection status
    unsigned long _lastConnectionAttempt; ///< Last WiFi connection attempt timestamp
    unsigned long _connectionTimeout;     ///< WiFi connection timeout in milliseconds
    int _maxRetries;                 ///< Maximum number of HTTP request retries
    unsigned long _retryDelay;       ///< Delay between HTTP retries in milliseconds
    unsigned long _lastHeartbeat;    ///< Last successful platform communication timestamp
    unsigned long _heartbeatInterval; ///< Heartbeat interval in milliseconds
    int _consecutiveFailures;        ///< Count of consecutive connection failures
    int _maxConsecutiveFailures;     ///< Maximum allowed consecutive failures before reset
    unsigned long _lastErrorTime;    ///< Timestamp of last error occurrence
    String _lastErrorMessage;        ///< Last error message for debugging
    bool _autoReconnect;            ///< Enable automatic reconnection
    
    bool _debug;                     ///< Debug mode flag
    
    /**
     * @brief Internal method to print debug messages
     * @param message Debug message to print
     */
    void debugPrint(const String& message);
    
    /**
     * @brief Internal method to validate API response
     * @param response HTTP response to validate
     * @return true if response is valid, false otherwise
     */
    bool validateResponse(const MicroSafariResponse& response);
    
    /**
     * @brief Internal method to perform HTTP request with retry logic
     * @param endpoint API endpoint to call
     * @param payload JSON payload to send
     * @param method HTTP method (default: POST)
     * @return MicroSafariResponse structure with response details
     */
    MicroSafariResponse performHttpRequest(const String& endpoint, 
                                          const String& payload, 
                                          const String& method = "POST");
    
    /**
     * @brief Internal method to validate JSON payload structure
     * @param jsonPayload JSON string to validate
     * @return true if JSON is valid, false otherwise
     */
    bool validateJsonPayload(const String& jsonPayload);
    
    /**
     * @brief Internal method to check if heartbeat is needed
     * @return true if heartbeat should be sent, false otherwise
     */
    bool needsHeartbeat();
    
    /**
     * @brief Internal method to send heartbeat to platform
     * @return true if heartbeat successful, false otherwise
     */
    bool sendHeartbeat();
    
    /**
     * @brief Internal method to handle connection failure
     * @param errorMessage Error message describing the failure
     */
    void handleConnectionFailure(const String& errorMessage);
    
    /**
     * @brief Internal method to reset connection state
     */
    void resetConnectionState();
    
    /**
     * @brief Internal method to perform WiFi diagnostics
     * @return String containing diagnostic information
     */
    String getWiFiDiagnostics();

public:
    /**
     * @brief Constructor for MicroSafari class
     */
    MicroSafari();
    
    /**
     * @brief Destructor for MicroSafari class
     */
    ~MicroSafari();
    
    /**
     * @brief Initialize the MicroSafari library with connection parameters
     * @param ssid WiFi network SSID
     * @param password WiFi network password
     * @param apiKey Device API key from MicroSafari platform
     * @param platformUrl MicroSafari platform URL (default: production URL)
     * @param deviceName Optional device identifier name
     * @return true if initialization successful, false otherwise
     */
    bool begin(const String& ssid, 
               const String& password, 
               const String& apiKey,
               const String& platformUrl = "https://microsafari.com",
               const String& deviceName = "ESP32-Device");
    
    /**
     * @brief Connect to WiFi network
     * @param timeout Connection timeout in milliseconds (default: 30000ms)
     * @return true if connection successful, false otherwise
     */
    bool connectWiFi(unsigned long timeout = 30000);
    
    /**
     * @brief Check if WiFi is connected
     * @return true if WiFi is connected, false otherwise
     */
    bool isWiFiConnected();
    
    /**
     * @brief Test connection to MicroSafari platform
     * @return true if platform is reachable, false otherwise
     */
    bool testConnection();
    
    /**
     * @brief Send sensor data to MicroSafari platform
     * @param sensorData JSON object containing sensor readings
     * @return MicroSafariResponse structure with response details
     */
    MicroSafariResponse sendSensorData(const JsonObject& sensorData);
    
    /**
     * @brief Send raw JSON string data to MicroSafari platform
     * @param jsonPayload JSON string containing sensor readings
     * @return MicroSafariResponse structure with response details
     */
    MicroSafariResponse sendRawData(const String& jsonPayload);
    
    /**
     * @brief Send sensor data using key-value pairs
     * @param temperature Temperature reading
     * @param humidity Humidity reading
     * @param soilMoisture Soil moisture reading (optional, use -1 to skip)
     * @param lightLevel Light level reading (optional, use -1 to skip)
     * @return MicroSafariResponse structure with response details
     */
    MicroSafariResponse sendSensorData(float temperature, 
                                       float humidity, 
                                       float soilMoisture = -1, 
                                       float lightLevel = -1);
    
    /**
     * @brief Get current connection status
     * @return MicroSafariStatus enumeration value
     */
    MicroSafariStatus getStatus();
    
    /**
     * @brief Get status as human-readable string
     * @return String description of current status
     */
    String getStatusString();
    
    /**
     * @brief Get WiFi signal strength (RSSI)
     * @return WiFi signal strength in dBm
     */
    int getWiFiSignalStrength();
    
    /**
     * @brief Enable or disable debug output
     * @param enable true to enable debug, false to disable
     */
    void setDebug(bool enable);
    
    /**
     * @brief Set connection timeout for WiFi
     * @param timeout Timeout in milliseconds
     */
    void setConnectionTimeout(unsigned long timeout);
    
    /**
     * @brief Set HTTP retry configuration
     * @param maxRetries Maximum number of retry attempts (default: 3)
     * @param retryDelay Delay between retries in milliseconds (default: 2000)
     */
    void setRetryConfig(int maxRetries = 3, unsigned long retryDelay = 2000);
    
    /**
     * @brief Set heartbeat interval for platform communication
     * @param interval Heartbeat interval in milliseconds (default: 300000 = 5 minutes)
     */
    void setHeartbeatInterval(unsigned long interval = 300000);
    
    /**
     * @brief Force immediate heartbeat to platform
     * @return true if heartbeat successful, false otherwise
     */
    bool forceHeartbeat();
    
    /**
     * @brief Get last successful communication timestamp
     * @return Timestamp in milliseconds
     */
    unsigned long getLastHeartbeat();
    
    /**
     * @brief Check if device is actively connected to platform
     * @return true if platform communication is active, false otherwise
     */
    bool isPlatformActive();
    
    /**
     * @brief Enable or disable automatic reconnection
     * @param enable true to enable auto-reconnect, false to disable
     */
    void setAutoReconnect(bool enable);
    
    /**
     * @brief Set maximum consecutive failures before connection reset
     * @param maxFailures Maximum consecutive failures (default: 5)
     */
    void setMaxConsecutiveFailures(int maxFailures = 5);
    
    /**
     * @brief Get connection statistics and diagnostics
     * @return String containing connection diagnostics
     */
    String getConnectionDiagnostics();
    
    /**
     * @brief Get last error information
     * @return String containing last error message and timestamp
     */
    String getLastError();
    
    /**
     * @brief Clear error history and reset failure counters
     */
    void clearErrors();
    
    /**
     * @brief Perform comprehensive connection test
     * @return true if all connectivity tests pass, false otherwise
     */
    bool runConnectivityTest();
    
    /**
     * @brief Get comprehensive device status including errors
     * @return JsonObject containing detailed status information
     */
    JsonObject getDetailedStatus();
    
    /**
     * @brief Disconnect from WiFi and cleanup resources
     */
    void disconnect();
    
    /**
     * @brief Main loop function - call this regularly in your main loop
     * Handles automatic reconnection and status monitoring
     */
    void loop();
    
    /**
     * @brief Get the device MAC address
     * @return String containing MAC address
     */
    String getMacAddress();
    
    /**
     * @brief Get current WiFi IP address
     * @return String containing IP address
     */
    String getIPAddress();
};

#endif // MICROSAFARI_H