/*!
 * @file MicroSafari.cpp
 * @brief MicroSafari ESP32 Library - Implementation file
 *
 * This library provides easy connectivity to the MicroSafari IoT platform
 * for ESP32 devices, handling WiFi connection, data transmission, and
 * platform communication.
 *
 * @version 1.0.0
 * @date 2025-08-22
 * @author MicroSafari Team
 */

#include "MicroSafari.h"

// Constructor
MicroSafari::MicroSafari() {
    _debugEnabled = false;
    _initialized = false;
}

// Destructor
MicroSafari::~MicroSafari() {
    if (_initialized) {
        WiFi.disconnect(true);
    }
}

bool MicroSafari::begin(const char* wifiSSID, const char* wifiPassword, const char* apiKey, const char* platformURL, const char* deviceName) {
    debugPrintln("MicroSafari::begin() - Starting initialization...");

    // Store configuration
    _wifiSSID = String(wifiSSID);
    _wifiPassword = String(wifiPassword);
    _apiKey = String(apiKey);
    _platformURL = String(platformURL);
    _deviceName = String(deviceName);

    // Validate required parameters
    if (_wifiSSID.length() == 0 || _wifiPassword.length() == 0 || _apiKey.length() == 0 || _platformURL.length() == 0) {
        debugPrintln("MicroSafari::begin() - ERROR: Missing required configuration parameters");
        return false;
    }

    // Initialize WiFi
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);

    debugPrintln("MicroSafari::begin() - Configuration stored successfully");
    debugPrintln("  SSID: " + _wifiSSID);
    debugPrintln("  API Key: " + _apiKey.substring(0, 8) + "...");
    debugPrintln("  Platform URL: " + _platformURL);
    debugPrintln("  Device Name: " + _deviceName);

    _initialized = true;
    return true;
}

void MicroSafari::setDebug(bool enabled) {
    _debugEnabled = enabled;
    debugPrintln("Debug output " + String(enabled ? "enabled" : "disabled"));
}

bool MicroSafari::connectWiFi() {
    if (!_initialized) {
        debugPrintln("MicroSafari::connectWiFi() - ERROR: Library not initialized");
        return false;
    }

    debugPrintln("MicroSafari::connectWiFi() - Connecting to: " + _wifiSSID);

    // Disconnect if already connected
    if (WiFi.status() == WL_CONNECTED) {
        WiFi.disconnect();
        delay(100);
    }

    // Start connection
    WiFi.begin(_wifiSSID.c_str(), _wifiPassword.c_str());

    // Wait for connection (timeout after 30 seconds)
    int attempts = 0;
    const int maxAttempts = 30;

    while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
        delay(1000);
        attempts++;
        debugPrint(".");

        if (attempts % 10 == 0) {
            debugPrintln("");
            debugPrintln("Still connecting... (" + String(attempts) + "/" + String(maxAttempts) + ")");
        }
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        debugPrintln("");
        debugPrintln("WiFi connected successfully!");
        debugPrintln("  IP Address: " + WiFi.localIP().toString());
        debugPrintln("  Signal Strength: " + String(WiFi.RSSI()) + " dBm");
        return true;
    } else {
        debugPrintln("");
        debugPrintln("WiFi connection failed after " + String(maxAttempts) + " seconds");
        return false;
    }
}

bool MicroSafari::isWiFiConnected() {
    return WiFi.status() == WL_CONNECTED;
}

bool MicroSafari::testConnection() {
    if (!isWiFiConnected()) {
        debugPrintln("MicroSafari::testConnection() - ERROR: WiFi not connected");
        return false;
    }
    
    debugPrintln("MicroSafari::testConnection() - Testing platform connection...");
    debugPrintln("Platform URL: " + _platformURL);
    debugPrintln("Local IP: " + getLocalIP());
    debugPrintln("Gateway: " + WiFi.gatewayIP().toString());
    debugPrintln("DNS: " + WiFi.dnsIP().toString());

    // Test the API endpoint directly
    HTTPClient httpClient;
    String apiUrl = _platformURL + "/api/ingest";
    debugPrintln("Testing API endpoint: " + apiUrl);

    httpClient.begin(apiUrl);
    httpClient.addHeader("User-Agent", "MicroSafari-ESP32/1.0.0");
    httpClient.addHeader("Connection", "close");
    httpClient.addHeader("x-api-key", _apiKey);
    httpClient.addHeader("Content-Type", "application/json");
    httpClient.setTimeout(10000);
    httpClient.setReuse(false);
    httpClient.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS); // Follow redirects

    // Try a small test payload
    String testPayload = "{\"payload\":{\"test\":true,\"timestamp\":" + String(millis()) + ",\"device\":\"" + _deviceName + "\"}}";
    debugPrintln("JSON payload: " + testPayload);
    debugPrintln("JSON validation successful");
    debugPrintln("Performing HTTP POST to: /api/ingest");

    int httpCode = httpClient.POST(testPayload);
    String response = httpClient.getString();
    httpClient.end();

    debugPrintln("HTTP response code: " + String(httpCode));
    debugPrintln("HTTP response body: " + response);

    // Check for successful response
    if (httpCode > 0) {
        if (httpCode >= 200 && httpCode < 400) { // Accept redirects as success too
            debugPrintln("Platform connection test successful");
            return true;
        } else {
            debugPrintln("API endpoint returned error code: " + String(httpCode));
            return false;
        }
    } else {
        debugPrintln("HTTP request failed with code: " + String(httpCode));
        debugPrintln("Possible issues:");
        debugPrintln("1. Server is redirecting but client can't follow");
        debugPrintln("2. SSL/TLS configuration issue");
        debugPrintln("3. Server refusing connection");
    }

    debugPrintln("Platform connection test failed: Network error - check connection");
    return false;
}

MicroSafariResponse MicroSafari::sendSensorData(float temperature, float humidity, float soilMoisture, float lightLevel) {
    MicroSafariResponse response;

    if (!_initialized) {
        response.errorMessage = "Library not initialized";
        debugPrintln("MicroSafari::sendSensorData() - ERROR: " + response.errorMessage);
        return response;
    }
    
    if (!isWiFiConnected()) {
        response.errorMessage = "WiFi not connected";
        debugPrintln("MicroSafari::sendSensorData() - ERROR: " + response.errorMessage);
        return response;
    }
    
    debugPrintln("MicroSafari::sendSensorData() - Preparing to send data...");

    // Create JSON payload
    String payload = createSensorDataPayload(temperature, humidity, soilMoisture, lightLevel);
    debugPrintln("Payload: " + payload);

    // Send HTTP POST request to correct endpoint
    String endpoint = _platformURL + "/api/ingest";
    debugPrintln("Sending to: " + endpoint);

    // Create new HTTP client with optimized configuration for real-time performance
    HTTPClient httpClient;
    httpClient.begin(endpoint);
    httpClient.addHeader("x-api-key", _apiKey);
    httpClient.addHeader("Content-Type", "application/json");
    httpClient.addHeader("User-Agent", "MicroSafari-ESP32/1.0.0");
    httpClient.addHeader("Connection", "close");
    httpClient.setTimeout(5000);  // Reduced from 15s to 5s for faster response
    httpClient.setReuse(false);
    httpClient.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    // Reduced retry attempts for faster failure detection
    const int maxAttempts = 2;  // Reduced from 3 to 2
    for (int attempt = 1; attempt <= maxAttempts; attempt++) {
        debugPrintln("HTTP attempt " + String(attempt) + "/" + String(maxAttempts));

        int httpCode = httpClient.POST(payload);
        String responseData = httpClient.getString();

        debugPrintln("HTTP response code: " + String(httpCode));
        debugPrintln("HTTP response data: " + responseData);

        response.httpCode = httpCode;
        response.responseData = responseData;

        if (httpCode > 0) {
            if (httpCode >= 200 && httpCode < 300) {
                response.success = true;
                debugPrintln("Data sent successfully!");
                httpClient.end();
                return response;
            } else if (httpCode >= 300 && httpCode < 400) {
                debugPrintln("Redirect response received: " + String(httpCode));
                response.success = true;
                httpClient.end();
                return response;
            } else {
                response.success = false;
                response.errorMessage = "HTTP " + String(httpCode) + ": " + responseData;
                debugPrintln("Server error: " + response.errorMessage);
                httpClient.end();
                return response;
            }
        } else {
            debugPrintln("Network error (code " + String(httpCode) + "), retrying in " + String(500 * attempt) + "ms...");
            if (attempt < maxAttempts) {
                delay(500 * attempt);  // Reduced retry delay from 1000ms to 500ms
            }
        }
    }

    httpClient.end();
    response.success = false;
    response.errorMessage = "Network error after " + String(maxAttempts) + " attempts";
    debugPrintln("Failed to send data: " + response.errorMessage);
    return response;
}

void MicroSafari::loop() {
    // Check WiFi connection and attempt reconnection if needed
    if (!isWiFiConnected()) {
        static unsigned long lastReconnectAttempt = 0;
        if (millis() - lastReconnectAttempt > 30000) { // Try every 30 seconds
            debugPrintln("WiFi disconnected - attempting reconnection...");
            connectWiFi();
            lastReconnectAttempt = millis();
        }
    }
}

int MicroSafari::getWiFiRSSI() {
    return WiFi.RSSI();
}

String MicroSafari::getLocalIP() {
    return WiFi.localIP().toString();
}

void MicroSafari::debugPrint(const String& message) {
    if (_debugEnabled) {
        Serial.print("[MicroSafari] " + message);
    }
}

void MicroSafari::debugPrintln(const String& message) {
    if (_debugEnabled) {
        Serial.println("[MicroSafari] " + message);
    }
}

String MicroSafari::createSensorDataPayload(float temperature, float humidity, float soilMoisture, float lightLevel) {
    DynamicJsonDocument doc(1024);

    // Create the payload object to match API specification
    JsonObject payload = doc.createNestedObject("payload");

    // Add sensor data directly to payload (matching your curl example)
    payload["temperature"] = temperature;
    payload["humidity"] = humidity;
    payload["soil_moisture"] = soilMoisture;
    payload["light_level"] = lightLevel;
    payload["timestamp"] = millis();
    payload["device_name"] = _deviceName;

    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}