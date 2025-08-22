/*!
 * @file WiFiConnection.ino
 * @brief Basic WiFi connection example for MicroSafari ESP32 Library
 * 
 * This example demonstrates how to:
 * - Initialize the MicroSafari library
 * - Connect to WiFi network
 * - Monitor connection status
 * - Display network information
 * 
 * @version 1.0.0
 * @date 2025-08-22
 * @author MicroSafari Team
 */

#include <MicroSafari.h>

// WiFi credentials - CHANGE THESE TO YOUR NETWORK
const char* WIFI_SSID = "your_wifi_ssid";
const char* WIFI_PASSWORD = "your_wifi_password";

// MicroSafari configuration - GET THESE FROM YOUR DASHBOARD
const char* API_KEY = "your_device_api_key";
const char* PLATFORM_URL = "https://your-microsafari-instance.com";
const char* DEVICE_NAME = "ESP32-WiFi-Test";

// Create MicroSafari instance
MicroSafari microSafari;

void setup() {
    // Initialize Serial communication
    Serial.begin(115200);
    while (!Serial) {
        delay(10); // Wait for Serial to be ready
    }
    
    Serial.println();
    Serial.println("=================================");
    Serial.println("MicroSafari WiFi Connection Test");
    Serial.println("=================================");
    
    // Enable debug output
    microSafari.setDebug(true);
    
    // Initialize MicroSafari library
    Serial.println("Initializing MicroSafari library...");
    if (!microSafari.begin(WIFI_SSID, WIFI_PASSWORD, API_KEY, PLATFORM_URL, DEVICE_NAME)) {
        Serial.println("Failed to initialize MicroSafari library!");
        Serial.println("Please check your configuration and restart.");
        while (true) {
            delay(1000);
        }
    }
    
    Serial.println("Library initialized successfully!");
    
    // Set connection timeout to 30 seconds
    microSafari.setConnectionTimeout(30000);
    
    // Attempt WiFi connection
    Serial.println("Connecting to WiFi...");
    if (microSafari.connectWiFi()) {
        Serial.println("WiFi connection successful!");
        displayNetworkInfo();
        
        // Test platform connection
        Serial.println("Testing platform connection...");
        if (microSafari.testConnection()) {
            Serial.println("Platform connection test successful!");
        } else {
            Serial.println("Platform connection test failed!");
            Serial.println("Please check your API key and platform URL.");
        }
    } else {
        Serial.println("WiFi connection failed!");
        Serial.println("Please check your WiFi credentials and try again.");
    }
}

void loop() {
    // Call the library's loop function for automatic reconnection
    microSafari.loop();
    
    // Display status every 10 seconds
    static unsigned long lastStatusUpdate = 0;
    if (millis() - lastStatusUpdate >= 10000) {
        displayStatus();
        lastStatusUpdate = millis();
    }
    
    delay(1000);
}

/**
 * @brief Display current connection status
 */
void displayStatus() {
    Serial.println();
    Serial.println("--- Status Update ---");
    Serial.print("Status: ");
    Serial.println(microSafari.getStatusString());
    
    if (microSafari.isWiFiConnected()) {
        Serial.print("IP Address: ");
        Serial.println(microSafari.getIPAddress());
        Serial.print("Signal Strength: ");
        Serial.print(microSafari.getWiFiSignalStrength());
        Serial.println(" dBm");
    } else {
        Serial.println("WiFi: Disconnected");
    }
    
    Serial.print("Uptime: ");
    Serial.print(millis() / 1000);
    Serial.println(" seconds");
    Serial.println("-------------------");
}

/**
 * @brief Display network information after successful connection
 */
void displayNetworkInfo() {
    Serial.println();
    Serial.println("=== Network Information ===");
    Serial.print("SSID: ");
    Serial.println(WIFI_SSID);
    Serial.print("IP Address: ");
    Serial.println(microSafari.getIPAddress());
    Serial.print("MAC Address: ");
    Serial.println(microSafari.getMacAddress());
    Serial.print("Signal Strength: ");
    Serial.print(microSafari.getWiFiSignalStrength());
    Serial.println(" dBm");
    Serial.print("Device Name: ");
    Serial.println(DEVICE_NAME);
    Serial.println("===========================");
    Serial.println();
}