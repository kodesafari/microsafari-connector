/*!
 * @file MicroSafari.cpp
 * @brief Implementation of MicroSafari ESP32 Arduino Library
 * @version 1.0.0
 * @date 2025-08-22
 */

#include "MicroSafari.h"

/**
 * @brief Constructor
 */
MicroSafari::MicroSafari() {
    _status = MICROSAFARI_DISCONNECTED;
    _lastConnectionAttempt = 0;
    _connectionTimeout = 30000; // 30 seconds default
    _maxRetries = 3; // Default retry count
    _retryDelay = 2000; // 2 seconds between retries
    _lastHeartbeat = 0;
    _heartbeatInterval = 300000; // 5 minutes default
    _consecutiveFailures = 0;
    _maxConsecutiveFailures = 5;
    _lastErrorTime = 0;
    _lastErrorMessage = "";
    _autoReconnect = true;
    _debug = false;
}

/**
 * @brief Destructor
 */
MicroSafari::~MicroSafari() {
    disconnect();
}

/**
 * @brief Internal debug print method
 */
void MicroSafari::debugPrint(const String& message) {
    if (_debug) {
        Serial.print("[MicroSafari] ");
        Serial.println(message);
    }
}

/**
 * @brief Initialize the library
 */
bool MicroSafari::begin(const String& ssid, 
                        const String& password, 
                        const String& apiKey,
                        const String& platformUrl,
                        const String& deviceName) {
    debugPrint("Initializing MicroSafari library...");
    
    // Validate parameters
    if (ssid.isEmpty() || password.isEmpty() || apiKey.isEmpty()) {
        debugPrint("ERROR: SSID, password, and API key cannot be empty");
        return false;
    }
    
    if (platformUrl.isEmpty()) {
        debugPrint("ERROR: Platform URL cannot be empty");
        return false;
    }
    
    // Store configuration
    _ssid = ssid;
    _password = password;
    _apiKey = apiKey;
    _platformUrl = platformUrl;
    _deviceName = deviceName.isEmpty() ? "ESP32-Device" : deviceName;
    
    // Initialize WiFi
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(_deviceName.c_str());
    
    debugPrint("Configuration stored successfully");
    debugPrint("Device name: " + _deviceName);
    debugPrint("Platform URL: " + _platformUrl);
    
    return true;
}

/**
 * @brief Connect to WiFi network
 */
bool MicroSafari::connectWiFi(unsigned long timeout) {
    debugPrint("Attempting WiFi connection...");
    debugPrint("SSID: " + _ssid);
    
    _status = MICROSAFARI_WIFI_CONNECTING;
    _lastConnectionAttempt = millis();
    
    // Start WiFi connection
    WiFi.begin(_ssid.c_str(), _password.c_str());
    
    unsigned long startTime = millis();
    
    // Wait for connection with timeout
    while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < timeout) {
        delay(500);
        debugPrint("Connecting...");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        _status = MICROSAFARI_WIFI_CONNECTED;
        debugPrint("WiFi connected successfully!");
        debugPrint("IP address: " + WiFi.localIP().toString());
        debugPrint("Signal strength: " + String(WiFi.RSSI()) + " dBm");
        
        // Reset failure counter on successful connection
        if (_consecutiveFailures > 0) {
            debugPrint("WiFi reconnected, resetting failure counter");
            _consecutiveFailures = 0;
        }
        
        return true;
    } else {
        _status = MICROSAFARI_ERROR;
        String errorMsg = "WiFi connection failed (status: " + String(WiFi.status()) + ")";
        debugPrint(errorMsg);
        handleConnectionFailure(errorMsg);
        return false;
    }
}

/**
 * @brief Check WiFi connection status
 */
bool MicroSafari::isWiFiConnected() {
    return WiFi.status() == WL_CONNECTED;
}

/**
 * @brief Test connection to MicroSafari platform
 */
bool MicroSafari::testConnection() {
    if (!isWiFiConnected()) {
        debugPrint("Cannot test connection - WiFi not connected");
        return false;
    }
    
    debugPrint("Testing platform connection...");
    
    // Create a simple test payload
    DynamicJsonDocument testDoc(256);
    testDoc["test"] = true;
    testDoc["timestamp"] = millis();
    testDoc["device"] = _deviceName;
    
    JsonObject testData = testDoc.as<JsonObject>();
    MicroSafariResponse response = sendSensorData(testData);
    
    if (response.success) {
        _status = MICROSAFARI_PLATFORM_CONNECTED;
        debugPrint("Platform connection test successful");
        return true;
    } else {
        debugPrint("Platform connection test failed: " + response.errorMessage);
        return false;
    }
}

/**
 * @brief Send sensor data with JsonObject
 */
MicroSafariResponse MicroSafari::sendSensorData(const JsonObject& sensorData) {
    debugPrint("Preparing to send sensor data...");
    
    // Create the complete payload structure expected by /api/ingest
    DynamicJsonDocument doc(1024);
    doc["payload"] = sensorData;
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    debugPrint("JSON payload: " + jsonString);
    
    // Validate JSON structure before sending
    if (!validateJsonPayload(jsonString)) {
        MicroSafariResponse response;
        response.success = false;
        response.httpCode = 0;
        response.errorMessage = "Invalid JSON payload structure";
        return response;
    }
    
    return performHttpRequest("/api/ingest", jsonString);
}

/**
 * @brief Send raw JSON string data
 */
MicroSafariResponse MicroSafari::sendRawData(const String& jsonPayload) {
    debugPrint("Preparing to send raw JSON data...");
    debugPrint("Raw JSON payload: " + jsonPayload);
    
    // Validate JSON structure before sending
    if (!validateJsonPayload(jsonPayload)) {
        MicroSafariResponse response;
        response.success = false;
        response.httpCode = 0;
        response.errorMessage = "Invalid JSON payload structure";
        return response;
    }
    
    return performHttpRequest("/api/ingest", jsonPayload);
}

/**
 * @brief Send sensor data with individual parameters
 */
MicroSafariResponse MicroSafari::sendSensorData(float temperature, 
                                                float humidity, 
                                                float soilMoisture, 
                                                float lightLevel) {
    // Create JSON object with sensor readings
    DynamicJsonDocument doc(512);
    JsonObject sensorData = doc.to<JsonObject>();
    
    // Add mandatory readings
    sensorData["temperature"] = temperature;
    sensorData["humidity"] = humidity;
    
    // Add optional readings if provided (not -1)
    if (soilMoisture != -1) {
        sensorData["soil_moisture"] = soilMoisture;
    }
    
    if (lightLevel != -1) {
        sensorData["light_level"] = lightLevel;
    }
    
    // Add timestamp and device info
    sensorData["timestamp"] = millis();
    sensorData["device_name"] = _deviceName;
    
    return sendSensorData(sensorData);
}

/**
 * @brief Get current status
 */
MicroSafariStatus MicroSafari::getStatus() {
    return _status;
}

/**
 * @brief Get status as string
 */
String MicroSafari::getStatusString() {
    switch (_status) {
        case MICROSAFARI_DISCONNECTED:
            return "Disconnected";
        case MICROSAFARI_WIFI_CONNECTING:
            return "WiFi Connecting";
        case MICROSAFARI_WIFI_CONNECTED:
            return "WiFi Connected";
        case MICROSAFARI_PLATFORM_CONNECTED:
            return "Platform Connected";
        case MICROSAFARI_ERROR:
            return "Error";
        default:
            return "Unknown";
    }
}

/**
 * @brief Get WiFi signal strength
 */
int MicroSafari::getWiFiSignalStrength() {
    if (isWiFiConnected()) {
        return WiFi.RSSI();
    }
    return -999; // Invalid signal strength
}

/**
 * @brief Set debug mode
 */
void MicroSafari::setDebug(bool enable) {
    _debug = enable;
    debugPrint(enable ? "Debug mode enabled" : "Debug mode disabled");
}

/**
 * @brief Set connection timeout
 */
void MicroSafari::setConnectionTimeout(unsigned long timeout) {
    _connectionTimeout = timeout;
    debugPrint("Connection timeout set to " + String(timeout) + "ms");
}

/**
 * @brief Set HTTP retry configuration
 */
void MicroSafari::setRetryConfig(int maxRetries, unsigned long retryDelay) {
    _maxRetries = maxRetries;
    _retryDelay = retryDelay;
    debugPrint("Retry config set: " + String(maxRetries) + " retries, " + String(retryDelay) + "ms delay");
}

/**
 * @brief Set heartbeat interval
 */
void MicroSafari::setHeartbeatInterval(unsigned long interval) {
    _heartbeatInterval = interval;
    debugPrint("Heartbeat interval set to " + String(interval) + "ms");
}

/**
 * @brief Force immediate heartbeat
 */
bool MicroSafari::forceHeartbeat() {
    return sendHeartbeat();
}

/**
 * @brief Get last heartbeat timestamp
 */
unsigned long MicroSafari::getLastHeartbeat() {
    return _lastHeartbeat;
}

/**
 * @brief Check if platform is actively connected
 */
bool MicroSafari::isPlatformActive() {
    if (!isWiFiConnected()) {
        return false;
    }
    
    // Consider platform active if last heartbeat was within 2x the interval
    return (millis() - _lastHeartbeat) < (_heartbeatInterval * 2);
}

/**
 * @brief Handle connection failure
 */
void MicroSafari::handleConnectionFailure(const String& errorMessage) {
    _consecutiveFailures++;
    _lastErrorTime = millis();
    _lastErrorMessage = errorMessage;
    
    debugPrint("Connection failure #" + String(_consecutiveFailures) + ": " + errorMessage);
    
    if (_consecutiveFailures >= _maxConsecutiveFailures) {
        debugPrint("Maximum consecutive failures reached, resetting connection...");
        resetConnectionState();
    }
}

/**
 * @brief Reset connection state
 */
void MicroSafari::resetConnectionState() {
    debugPrint("Resetting connection state...");
    
    // Disconnect and reset WiFi
    WiFi.disconnect();
    delay(1000);
    
    // Reset internal state
    _status = MICROSAFARI_DISCONNECTED;
    _consecutiveFailures = 0;
    _lastConnectionAttempt = 0;
    
    // Reinitialize WiFi
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(_deviceName.c_str());
    
    debugPrint("Connection state reset complete");
}

/**
 * @brief Get WiFi diagnostics
 */
String MicroSafari::getWiFiDiagnostics() {
    String diagnostics = "WiFi Diagnostics:\n";
    diagnostics += "Status: " + String(WiFi.status()) + "\n";
    diagnostics += "SSID: " + WiFi.SSID() + "\n";
    diagnostics += "RSSI: " + String(WiFi.RSSI()) + " dBm\n";
    diagnostics += "IP: " + WiFi.localIP().toString() + "\n";
    diagnostics += "Gateway: " + WiFi.gatewayIP().toString() + "\n";
    diagnostics += "DNS: " + WiFi.dnsIP().toString() + "\n";
    diagnostics += "MAC: " + WiFi.macAddress() + "\n";
    return diagnostics;
}

/**
 * @brief Enable/disable auto-reconnect
 */
void MicroSafari::setAutoReconnect(bool enable) {
    _autoReconnect = enable;
    debugPrint("Auto-reconnect " + String(enable ? "enabled" : "disabled"));
}

/**
 * @brief Set max consecutive failures
 */
void MicroSafari::setMaxConsecutiveFailures(int maxFailures) {
    _maxConsecutiveFailures = maxFailures;
    debugPrint("Max consecutive failures set to " + String(maxFailures));
}

/**
 * @brief Get connection diagnostics
 */
String MicroSafari::getConnectionDiagnostics() {
    String diagnostics = "=== MicroSafari Connection Diagnostics ===\n";
    diagnostics += "Status: " + getStatusString() + "\n";
    diagnostics += "Platform Active: " + String(isPlatformActive() ? "Yes" : "No") + "\n";
    diagnostics += "Consecutive Failures: " + String(_consecutiveFailures) + "/" + String(_maxConsecutiveFailures) + "\n";
    diagnostics += "Last Heartbeat: " + String((millis() - _lastHeartbeat) / 1000) + "s ago\n";
    diagnostics += "Auto-reconnect: " + String(_autoReconnect ? "Enabled" : "Disabled") + "\n";
    diagnostics += "Free Heap: " + String(ESP.getFreeHeap()) + " bytes\n";
    diagnostics += "Uptime: " + String(millis() / 1000) + "s\n";
    
    if (isWiFiConnected()) {
        diagnostics += getWiFiDiagnostics();
    }
    
    if (!_lastErrorMessage.isEmpty()) {
        diagnostics += "\nLast Error: " + _lastErrorMessage + "\n";
        diagnostics += "Error Time: " + String((millis() - _lastErrorTime) / 1000) + "s ago\n";
    }
    
    return diagnostics;
}

/**
 * @brief Get last error information
 */
String MicroSafari::getLastError() {
    if (_lastErrorMessage.isEmpty()) {
        return "No errors recorded";
    }
    
    return "[" + String((millis() - _lastErrorTime) / 1000) + "s ago] " + _lastErrorMessage;
}

/**
 * @brief Clear error history
 */
void MicroSafari::clearErrors() {
    _consecutiveFailures = 0;
    _lastErrorTime = 0;
    _lastErrorMessage = "";
    debugPrint("Error history cleared");
}

/**
 * @brief Run comprehensive connectivity test
 */
bool MicroSafari::runConnectivityTest() {
    debugPrint("Running comprehensive connectivity test...");
    
    // Test 1: WiFi connectivity
    if (!isWiFiConnected()) {
        debugPrint("Connectivity test failed: WiFi not connected");
        return false;
    }
    debugPrint("✓ WiFi connectivity test passed");
    
    // Test 2: Internet connectivity (ping gateway)
    IPAddress gateway = WiFi.gatewayIP();
    if (gateway.toString() == "0.0.0.0") {
        debugPrint("Connectivity test failed: No gateway available");
        return false;
    }
    debugPrint("✓ Gateway connectivity test passed");
    
    // Test 3: Platform connectivity
    if (!testConnection()) {
        debugPrint("Connectivity test failed: Platform unreachable");
        return false;
    }
    debugPrint("✓ Platform connectivity test passed");
    
    // Test 4: JSON validation
    DynamicJsonDocument testDoc(256);
    testDoc["payload"]["test"] = "connectivity";
    String testJson;
    serializeJson(testDoc, testJson);
    
    if (!validateJsonPayload(testJson)) {
        debugPrint("Connectivity test failed: JSON validation error");
        return false;
    }
    debugPrint("✓ JSON validation test passed");
    
    debugPrint("All connectivity tests passed!");
    clearErrors(); // Clear any previous errors on successful test
    return true;
}

/**
 * @brief Get detailed status as JSON
 */
JsonObject MicroSafari::getDetailedStatus() {
    DynamicJsonDocument doc(1024);
    JsonObject status = doc.to<JsonObject>();
    
    status["status"] = getStatusString();
    status["wifi_connected"] = isWiFiConnected();
    status["platform_active"] = isPlatformActive();
    status["signal_strength"] = getWiFiSignalStrength();
    status["ip_address"] = getIPAddress();
    status["mac_address"] = getMacAddress();
    status["consecutive_failures"] = _consecutiveFailures;
    status["max_failures"] = _maxConsecutiveFailures;
    status["auto_reconnect"] = _autoReconnect;
    status["last_heartbeat"] = _lastHeartbeat;
    status["heartbeat_interval"] = _heartbeatInterval;
    status["uptime_seconds"] = millis() / 1000;
    status["free_heap"] = ESP.getFreeHeap();
    
    if (!_lastErrorMessage.isEmpty()) {
        status["last_error"] = _lastErrorMessage;
        status["error_time"] = _lastErrorTime;
    }
    
    return status;
}

/**
 * @brief Disconnect and cleanup
 */
void MicroSafari::disconnect() {
    debugPrint("Disconnecting...");
    _httpClient.end();
    WiFi.disconnect();
    _status = MICROSAFARI_DISCONNECTED;
}

/**
 * @brief Main loop function
 */
void MicroSafari::loop() {
    // Check WiFi connection status
    if (!isWiFiConnected() && _status != MICROSAFARI_WIFI_CONNECTING) {
        if (millis() - _lastConnectionAttempt > 30000) { // Retry every 30 seconds
            debugPrint("WiFi disconnected, attempting reconnection...");
            connectWiFi(_connectionTimeout);
        }
    }
    
    // Update status based on WiFi connection
    if (isWiFiConnected() && _status == MICROSAFARI_WIFI_CONNECTING) {
        _status = MICROSAFARI_WIFI_CONNECTED;
    } else if (!isWiFiConnected() && _status != MICROSAFARI_WIFI_CONNECTING) {
        _status = MICROSAFARI_DISCONNECTED;
    }
    
    // Send heartbeat if needed and WiFi is connected
    if (isWiFiConnected() && needsHeartbeat()) {
        debugPrint("Heartbeat interval reached, sending heartbeat...");
        if (!sendHeartbeat()) {
            handleConnectionFailure("Heartbeat failed");
        } else {
            // Reset failure counter on successful heartbeat
            if (_consecutiveFailures > 0) {
                debugPrint("Heartbeat successful, resetting failure counter");
                _consecutiveFailures = 0;
            }
        }
    }
    
    // Handle auto-reconnection if enabled
    if (_autoReconnect && !isWiFiConnected() && _status == MICROSAFARI_DISCONNECTED) {
        if (millis() - _lastConnectionAttempt > (30000 + (_consecutiveFailures * 10000))) {
            debugPrint("Auto-reconnect triggered (failure count: " + String(_consecutiveFailures) + ")");
            connectWiFi(_connectionTimeout);
        }
    }
}

/**
 * @brief Get MAC address
 */
String MicroSafari::getMacAddress() {
    return WiFi.macAddress();
}

/**
 * @brief Get IP address
 */
String MicroSafari::getIPAddress() {
    if (isWiFiConnected()) {
        return WiFi.localIP().toString();
    }
    return "0.0.0.0";
}

/**
 * @brief Validate HTTP response
 */
bool MicroSafari::validateResponse(const MicroSafariResponse& response) {
    return response.success && response.httpCode == 201;
}

/**
 * @brief Perform HTTP request with retry logic
 */
MicroSafariResponse MicroSafari::performHttpRequest(const String& endpoint, 
                                                   const String& payload, 
                                                   const String& method) {
    MicroSafariResponse response;
    response.success = false;
    response.httpCode = 0;
    
    if (!isWiFiConnected()) {
        response.errorMessage = "WiFi not connected";
        debugPrint("Cannot perform HTTP request - WiFi not connected");
        return response;
    }
    
    debugPrint("Performing HTTP " + method + " to: " + endpoint);
    
    int attempts = 0;
    while (attempts < _maxRetries) {
        attempts++;
        debugPrint("HTTP attempt " + String(attempts) + "/" + String(_maxRetries));
        
        // Prepare HTTP client
        _httpClient.begin(_wifiClient, _platformUrl + endpoint);
        _httpClient.addHeader("Content-Type", "application/json");
        _httpClient.addHeader("X-API-Key", _apiKey);
        _httpClient.addHeader("User-Agent", "MicroSafari-ESP32/1.0.0");
        _httpClient.setTimeout(15000); // 15 second timeout
        
        // Send request based on method
        if (method == "POST") {
            response.httpCode = _httpClient.POST(payload);
        } else if (method == "GET") {
            response.httpCode = _httpClient.GET();
        } else if (method == "PUT") {
            response.httpCode = _httpClient.PUT(payload);
        }
        
        response.payload = _httpClient.getString();
        _httpClient.end();
        
        debugPrint("HTTP response code: " + String(response.httpCode));
        debugPrint("HTTP response body: " + response.payload);
        
        // Check if request was successful
        if (response.httpCode == 201 || response.httpCode == 200) {
            response.success = true;
            _lastHeartbeat = millis(); // Update heartbeat on successful communication
            debugPrint("HTTP request successful!");
            return response;
        } else if (response.httpCode == 401) {
            response.errorMessage = "Authentication failed - check API key";
            debugPrint("Authentication failed - will not retry");
            return response; // Don't retry auth failures
        } else if (response.httpCode == 400) {
            response.errorMessage = "Invalid data format";
            debugPrint("Bad request - will not retry");
            return response; // Don't retry client errors
        }
        
        // For other errors, retry if we have attempts left
        if (attempts < _maxRetries) {
            debugPrint("Request failed, retrying in " + String(_retryDelay) + "ms...");
            delay(_retryDelay);
        }
    }
    
    // All retries exhausted
    if (response.httpCode == 503) {
        response.errorMessage = "Service unavailable - development mode";
    } else if (response.httpCode <= 0) {
        response.errorMessage = "Network error - check connection";
    } else {
        response.errorMessage = "Server error (HTTP " + String(response.httpCode) + ") - all retries exhausted";
    }
    
    debugPrint("HTTP request failed after " + String(_maxRetries) + " attempts");
    handleConnectionFailure(response.errorMessage);
    return response;
}

/**
 * @brief Validate JSON payload structure
 */
bool MicroSafari::validateJsonPayload(const String& jsonPayload) {
    if (jsonPayload.isEmpty()) {
        debugPrint("JSON validation failed: empty payload");
        return false;
    }
    
    // Basic JSON structure validation
    DynamicJsonDocument testDoc(2048);
    DeserializationError error = deserializeJson(testDoc, jsonPayload);
    
    if (error) {
        debugPrint("JSON validation failed: " + String(error.c_str()));
        return false;
    }
    
    // Check for required payload structure
    if (!testDoc.containsKey("payload")) {
        debugPrint("JSON validation failed: missing 'payload' field");
        return false;
    }
    
    debugPrint("JSON validation successful");
    return true;
}

/**
 * @brief Check if heartbeat is needed
 */
bool MicroSafari::needsHeartbeat() {
    return (millis() - _lastHeartbeat) > _heartbeatInterval;
}

/**
 * @brief Send heartbeat to platform
 */
bool MicroSafari::sendHeartbeat() {
    debugPrint("Sending heartbeat to platform...");
    
    // Create heartbeat payload
    DynamicJsonDocument doc(512);
    JsonObject heartbeatData = doc.to<JsonObject>();
    
    heartbeatData["heartbeat"] = true;
    heartbeatData["timestamp"] = millis();
    heartbeatData["device_name"] = _deviceName;
    heartbeatData["signal_strength"] = getWiFiSignalStrength();
    heartbeatData["free_heap"] = ESP.getFreeHeap();
    heartbeatData["uptime"] = millis() / 1000; // Uptime in seconds
    
    // Wrap in payload structure
    DynamicJsonDocument payloadDoc(1024);
    payloadDoc["payload"] = heartbeatData;
    
    String jsonString;
    serializeJson(payloadDoc, jsonString);
    
    MicroSafariResponse response = performHttpRequest("/api/ingest", jsonString);
    
    if (response.success) {
        debugPrint("Heartbeat sent successfully");
        return true;
    } else {
        debugPrint("Heartbeat failed: " + response.errorMessage);
        return false;
    }
}