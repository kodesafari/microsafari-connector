/*!
 * @file DiagnosticsDemo.ino
 * @brief Comprehensive diagnostics and monitoring example for MicroSafari ESP32 Library
 * 
 * This example demonstrates:
 * - Real-time connection diagnostics
 * - Error monitoring and reporting
 * - System health checks
 * - Performance monitoring
 * - Connectivity testing
 * - Status reporting for farm managers
 * 
 * Perfect for: Field deployment monitoring, remote troubleshooting,
 * system maintenance, and ensuring reliable IoT operations for
 * Indonesian millennial farmers.
 * 
 * @version 1.0.0
 * @date 2025-08-22
 * @author MicroSafari Team
 */

#include <MicroSafari.h>

// WiFi credentials
const char* WIFI_SSID = "your_wifi_ssid";
const char* WIFI_PASSWORD = "your_wifi_password";

// MicroSafari configuration
const char* API_KEY = "your_device_api_key";
const char* PLATFORM_URL = "https://your-microsafari-instance.com";
const char* DEVICE_NAME = "ESP32-Diagnostics-Station";

// Diagnostic intervals
const unsigned long DIAGNOSTICS_INTERVAL = 60000;  // 1 minute
const unsigned long HEARTBEAT_INTERVAL = 300000;   // 5 minutes
const unsigned long CONNECTIVITY_TEST_INTERVAL = 900000; // 15 minutes

// Create MicroSafari instance
MicroSafari microSafari;

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);
    
    Serial.println();
    Serial.println("==========================================");
    Serial.println("MicroSafari Diagnostics & Monitoring Demo");
    Serial.println("==========================================");
    Serial.println("🔧 IoT System Health Monitor for Smart Farms");
    Serial.println();
    
    // Configure MicroSafari for comprehensive monitoring
    microSafari.setDebug(true);
    microSafari.setRetryConfig(5, 3000);           // 5 retries, 3 second delay
    microSafari.setHeartbeatInterval(HEARTBEAT_INTERVAL);
    microSafari.setMaxConsecutiveFailures(3);      // Reset after 3 failures
    microSafari.setAutoReconnect(true);
    
    // Initialize MicroSafari
    Serial.println("🚀 Initializing MicroSafari library...");
    if (!microSafari.begin(WIFI_SSID, WIFI_PASSWORD, API_KEY, PLATFORM_URL, DEVICE_NAME)) {
        Serial.println("❌ Failed to initialize MicroSafari library!");
        Serial.println("Please check your configuration and restart.");
        while (true) {
            delay(1000);
        }
    }
    
    Serial.println("✅ Library initialized successfully!");
    
    // Connect to WiFi
    Serial.println("📶 Connecting to WiFi...");
    if (!microSafari.connectWiFi()) {
        Serial.println("❌ Initial WiFi connection failed!");
        Serial.println("Auto-reconnect is enabled, will continue trying...");
    } else {
        Serial.println("✅ WiFi connected successfully!");
        
        // Test platform connection
        Serial.println("🔗 Testing platform connection...");
        if (microSafari.testConnection()) {
            Serial.println("✅ Platform connection verified!");
        } else {
            Serial.println("⚠️  Platform connection test failed, but continuing...");
        }
    }
    
    Serial.println();
    Serial.println("🔧 Diagnostics system ready!");
    Serial.println("📊 Monitoring system health and connectivity...");
    Serial.println();
    
    // Display initial diagnostics
    displaySystemDiagnostics();
}

void loop() {
    // Call the library's loop function (handles auto-reconnect and heartbeats)
    microSafari.loop();
    
    // Perform regular diagnostics
    static unsigned long lastDiagnostics = 0;
    if (millis() - lastDiagnostics >= DIAGNOSTICS_INTERVAL) {
        performDiagnostics();
        lastDiagnostics = millis();
    }
    
    // Run comprehensive connectivity test
    static unsigned long lastConnectivityTest = 0;
    if (millis() - lastConnectivityTest >= CONNECTIVITY_TEST_INTERVAL) {
        runConnectivityTest();
        lastConnectivityTest = millis();
    }
    
    delay(5000); // 5 second loop delay for diagnostics
}

/**
 * @brief Perform comprehensive system diagnostics
 */
void performDiagnostics() {
    Serial.println("🔍 === SYSTEM DIAGNOSTICS ===");
    
    // Display current status
    displaySystemStatus();
    
    // Check for errors
    String lastError = microSafari.getLastError();
    if (lastError != "No errors recorded") {
        Serial.println("🚨 Recent Error: " + lastError);
    }
    
    // Display connection diagnostics
    String diagnostics = microSafari.getConnectionDiagnostics();
    Serial.println(diagnostics);
    
    // Send diagnostic data if connected
    if (microSafari.isPlatformActive()) {
        sendDiagnosticData();
    } else {
        Serial.println("⚠️  Platform not active, skipping diagnostic data transmission");
    }
    
    Serial.println("🔍 === END DIAGNOSTICS ===");
    Serial.println();
}

/**
 * @brief Display current system status in a readable format
 */
void displaySystemStatus() {
    Serial.println("📊 Current System Status:");
    Serial.printf("   Connection: %s\n", microSafari.getStatusString().c_str());
    Serial.printf("   WiFi: %s\n", microSafari.isWiFiConnected() ? "Connected" : "Disconnected");
    Serial.printf("   Platform: %s\n", microSafari.isPlatformActive() ? "Active" : "Inactive");
    Serial.printf("   Signal: %d dBm\n", microSafari.getWiFiSignalStrength());
    Serial.printf("   IP Address: %s\n", microSafari.getIPAddress().c_str());
    Serial.printf("   MAC Address: %s\n", microSafari.getMacAddress().c_str());
    Serial.printf("   Last Heartbeat: %lu seconds ago\n", (millis() - microSafari.getLastHeartbeat()) / 1000);
    Serial.printf("   Free Memory: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("   Uptime: %lu minutes\n", millis() / 60000);
}

/**
 * @brief Display comprehensive system diagnostics
 */
void displaySystemDiagnostics() {
    Serial.println("🔧 === SYSTEM DIAGNOSTICS ===");
    
    // Get detailed status as JSON
    JsonObject status = microSafari.getDetailedStatus();
    
    Serial.println("📊 Detailed System Information:");
    Serial.printf("   Device Name: %s\n", DEVICE_NAME);
    Serial.printf("   Status: %s\n", status["status"].as<String>().c_str());
    Serial.printf("   WiFi Connected: %s\n", status["wifi_connected"].as<bool>() ? "Yes" : "No");
    Serial.printf("   Platform Active: %s\n", status["platform_active"].as<bool>() ? "Yes" : "No");
    Serial.printf("   Signal Strength: %d dBm\n", status["signal_strength"].as<int>());
    Serial.printf("   IP Address: %s\n", status["ip_address"].as<String>().c_str());
    Serial.printf("   MAC Address: %s\n", status["mac_address"].as<String>().c_str());
    Serial.printf("   Consecutive Failures: %d/%d\n", status["consecutive_failures"].as<int>(), status["max_failures"].as<int>());
    Serial.printf("   Auto-reconnect: %s\n", status["auto_reconnect"].as<bool>() ? "Enabled" : "Disabled");
    Serial.printf("   Heartbeat Interval: %lu seconds\n", status["heartbeat_interval"].as<unsigned long>() / 1000);
    Serial.printf("   Uptime: %lu seconds\n", status["uptime_seconds"].as<unsigned long>());
    Serial.printf("   Free Heap: %d bytes\n", status["free_heap"].as<int>());
    
    if (status.containsKey("last_error")) {
        Serial.printf("   Last Error: %s\n", status["last_error"].as<String>().c_str());
        Serial.printf("   Error Time: %lu\n", status["error_time"].as<unsigned long>());
    }
    
    Serial.println("🔧 === END DIAGNOSTICS ===");
    Serial.println();
}

/**
 * @brief Send diagnostic data to the platform
 */
void sendDiagnosticData() {
    Serial.println("📡 Sending diagnostic data to platform...");
    
    // Create diagnostic payload
    DynamicJsonDocument doc(1024);
    JsonObject diagnosticData = doc.to<JsonObject>();
    
    // System diagnostics
    JsonObject system = diagnosticData.createNestedObject("system");
    system["device_name"] = DEVICE_NAME;
    system["firmware_version"] = "1.0.0";
    system["uptime_seconds"] = millis() / 1000;
    system["free_heap"] = ESP.getFreeHeap();
    system["cpu_frequency"] = ESP.getCpuFreqMHz();
    
    // Network diagnostics
    JsonObject network = diagnosticData.createNestedObject("network");
    network["wifi_connected"] = microSafari.isWiFiConnected();
    network["signal_strength"] = microSafari.getWiFiSignalStrength();
    network["ip_address"] = microSafari.getIPAddress();
    network["mac_address"] = microSafari.getMacAddress();
    
    // MicroSafari diagnostics
    JsonObject platform = diagnosticData.createNestedObject("platform");
    platform["status"] = microSafari.getStatusString();
    platform["platform_active"] = microSafari.isPlatformActive();
    platform["last_heartbeat"] = microSafari.getLastHeartbeat();
    platform["consecutive_failures"] = 0; // Would need getter for this
    
    // Environmental data (if available)
    JsonObject environment = diagnosticData.createNestedObject("environment");
    environment["temperature"] = 28.5; // Simulated
    environment["humidity"] = 75.0;    // Simulated
    
    // Metadata
    diagnosticData["type"] = "diagnostic_report";
    diagnosticData["timestamp"] = millis();
    diagnosticData["report_interval"] = DIAGNOSTICS_INTERVAL / 1000;
    
    // Send data
    MicroSafariResponse response = microSafari.sendSensorData(diagnosticData);
    
    if (response.success) {
        Serial.println("✅ Diagnostic data sent successfully!");
        Serial.printf("   📅 Response: %s\n", response.payload.c_str());
    } else {
        Serial.println("❌ Failed to send diagnostic data!");
        Serial.printf("   🚨 Error: %s\n", response.errorMessage.c_str());
    }
}

/**
 * @brief Run comprehensive connectivity test
 */
void runConnectivityTest() {
    Serial.println("🔍 === CONNECTIVITY TEST ===");
    
    if (microSafari.runConnectivityTest()) {
        Serial.println("✅ All connectivity tests passed!");
        
        // Force a heartbeat after successful test
        Serial.println("💓 Sending heartbeat...");
        if (microSafari.forceHeartbeat()) {
            Serial.println("✅ Heartbeat sent successfully!");
        } else {
            Serial.println("❌ Heartbeat failed!");
        }
    } else {
        Serial.println("❌ Connectivity test failed!");
        Serial.println("🔧 Diagnostic information:");
        Serial.println(microSafari.getConnectionDiagnostics());
        
        // Clear errors if too many have accumulated
        Serial.println("🧹 Clearing error history...");
        microSafari.clearErrors();
    }
    
    Serial.println("🔍 === END CONNECTIVITY TEST ===");
    Serial.println();
}