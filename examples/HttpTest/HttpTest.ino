/**
 * @file HttpTest.ino
 * @brief Simple HTTP connection test for MicroSafari library
 * 
 * This example tests the HTTP connection fix for the -1 error code issue.
 * It demonstrates proper HTTP (non-HTTPS) connectivity to a local development server.
 * 
 * @author MicroSafari Team
 */

#include <MicroSafari.h>

// WiFi credentials - CHANGE THESE TO YOUR NETWORK
const char* WIFI_SSID = "kodesafari";
const char* WIFI_PASSWORD = "rustdev123";

// MicroSafari configuration - GET THESE FROM YOUR DASHBOARD
const char* API_KEY = "test-epic12-ec22ceba73dd6de9af5a926b4614e669";
const char* PLATFORM_URL = "http://192.168.18.197:3000"; // HTTP URL for local dev server
const char* DEVICE_NAME = "HTTP Test Device";

// Test interval (in milliseconds)
const unsigned long TEST_INTERVAL = 10000; // Test every 10 seconds

MicroSafari microSafari;
unsigned long lastTest = 0;
int testCount = 0;

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n=== MicroSafari HTTP Connection Test ===");
    Serial.println("Testing fix for HTTP -1 error code issue");
    Serial.println("========================================\n");
    
    // Enable debug mode
    microSafari.setDebug(true);
    
    // Initialize MicroSafari
    Serial.println("Initializing MicroSafari...");
    if (!microSafari.begin(WIFI_SSID, WIFI_PASSWORD, API_KEY, PLATFORM_URL, DEVICE_NAME)) {
        Serial.println("❌ Failed to initialize MicroSafari!");
        while (true) {
            delay(1000);
        }
    }
    Serial.println("✅ MicroSafari initialized successfully");
    
    // Connect to WiFi
    Serial.println("\nConnecting to WiFi...");
    if (!microSafari.connectWiFi(30000)) {
        Serial.println("❌ Failed to connect to WiFi!");
        while (true) {
            delay(1000);
        }
    }
    Serial.println("✅ WiFi connected successfully");
    Serial.println("IP Address: " + microSafari.getIPAddress());
    Serial.println("Signal Strength: " + String(microSafari.getWiFiSignalStrength()) + " dBm");
    
    // Test platform connection
    Serial.println("\nTesting platform connection...");
    if (microSafari.testConnection()) {
        Serial.println("✅ Platform connection test successful!");
    } else {
        Serial.println("❌ Platform connection test failed");
        Serial.println("Last error: " + microSafari.getLastError());
    }
    
    Serial.println("\n=== Starting periodic tests ===");
}

void loop() {
    // Handle MicroSafari background tasks
    microSafari.loop();
    
    // Run periodic tests
    if (millis() - lastTest >= TEST_INTERVAL) {
        lastTest = millis();
        testCount++;
        
        Serial.println("\n--- Test #" + String(testCount) + " ---");
        Serial.println("Status: " + microSafari.getStatusString());
        
        // Send test sensor data
        float temperature = 20.0 + (testCount % 10); // Simulate temperature variation
        float humidity = 50.0 + (testCount % 20);    // Simulate humidity variation
        
        Serial.println("Sending test data: Temp=" + String(temperature) + "°C, Humidity=" + String(humidity) + "%");
        
        MicroSafariResponse response = microSafari.sendSensorData(temperature, humidity);
        
        if (response.success) {
            Serial.println("✅ Data sent successfully!");
            Serial.println("HTTP Code: " + String(response.httpCode));
            Serial.println("Response: " + response.payload);
        } else {
            Serial.println("❌ Failed to send data");
            Serial.println("HTTP Code: " + String(response.httpCode));
            Serial.println("Error: " + response.errorMessage);
            Serial.println("Last error: " + microSafari.getLastError());
        }
        
        // Show connection diagnostics
        Serial.println("Diagnostics: " + microSafari.getConnectionDiagnostics());
    }
    
    delay(100); // Small delay to prevent watchdog issues
}