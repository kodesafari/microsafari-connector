/*!
 * @file TrafficLightControl.ino
 * @brief Traffic Light Control System example for MicroSafari ESP32 Library
 * 
 * This example demonstrates how to:
 * - Connect to WiFi and MicroSafari platform
 * - Send traffic light state data (red, yellow, green)
 * - Poll for device commands from the web dashboard
 * - Control individual LEDs based on dashboard commands
 * - Handle bidirectional IoT communication
 * 
 * Hardware Requirements:
 * - ESP32 development board
 * - 3 LEDs (Red, Yellow, Green)
 * - 3 resistors (220Ω-330Ω)
 * - Breadboard and jumper wires
 * 
 * Wiring:
 * - Red LED: GPIO 16 (Pin 16) -> 220Ω resistor -> LED -> GND
 * - Yellow LED: GPIO 17 (Pin 17) -> 220Ω resistor -> LED -> GND  
 * - Green LED: GPIO 18 (Pin 18) -> 220Ω resistor -> LED -> GND
 * 
 * Dashboard Setup:
 * 1. Create 3 switch widgets in your MicroSafari dashboard
 * 2. Configure each switch with data sources: "red_light", "yellow_light", "green_light"
 * 3. Use switches to control individual lights remotely
 * 
 * @version 1.0.0
 * @date 2025-08-23
 * @author MicroSafari Team
 */

#include <MicroSafari.h>

// WiFi credentials - CHANGE THESE TO YOUR NETWORK
const char* WIFI_SSID = "your-wifi";
const char* WIFI_PASSWORD = "your wifi password";

// MicroSafari configuration - GET THESE FROM YOUR DASHBOARD
const char* API_KEY = "your-api-key";
const char* PLATFORM_URL = "your-platform-url";  // e.g., "https://api.microsafari.com"
const char* DEVICE_NAME = "your -device-name";  // e.g., "TrafficLightController"

// GPIO pin definitions for LEDs
const int RED_LED_PIN = 23;
const int YELLOW_LED_PIN = 22;
const int GREEN_LED_PIN = 21;

// State variables for each light
bool redLightState = false;
bool yellowLightState = false;
bool greenLightState = false;

// Timing intervals
const unsigned long STATUS_INTERVAL = 10000;  // Send status every 10 seconds
const unsigned long COMMAND_POLL_INTERVAL = 2000;  // Poll for commands every 2 seconds

// Create MicroSafari instance
MicroSafari microSafari;

// Forward declarations
bool handleCommand(const String& dataSource, const String& value);

void setup() {
    // Initialize Serial communication
    Serial.begin(115200);
    while (!Serial) {
        delay(10);
    }
    
    Serial.println();
    Serial.println("=======================================");
    Serial.println("MicroSafari Traffic Light Control Demo");
    Serial.println("=======================================");
    
    // Initialize LED pins
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(YELLOW_LED_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);
    
    // Turn off all LEDs initially
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(YELLOW_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, LOW);
    
    Serial.println("🚦 LED pins initialized");
    
    // Enable debug output
    microSafari.setDebug(true);
    
    // Set command callback function
    microSafari.setCommandCallback(handleCommand);
    
    // Initialize MicroSafari library
    Serial.println("Initializing MicroSafari library...");
    if (!microSafari.begin(WIFI_SSID, WIFI_PASSWORD, API_KEY, PLATFORM_URL, DEVICE_NAME)) {
        Serial.println("❌ Failed to initialize MicroSafari library!");
        Serial.println("Please check your configuration and restart.");
        while (true) {
            delay(1000);
        }
    }
    
    Serial.println("✅ Library initialized successfully!");
    
    // Connect to WiFi
    Serial.println("Connecting to WiFi...");
    if (!microSafari.connectWiFi()) {
        Serial.println("❌ WiFi connection failed!");
        Serial.println("Please check your WiFi credentials and restart.");
        while (true) {
            delay(1000);
        }
    }
    
    Serial.println("✅ WiFi connected successfully!");
    
    // Test platform connection
    Serial.println("Testing platform connection...");
    if (microSafari.testConnection()) {
        Serial.println("✅ Platform connection successful!");
        Serial.println();
        Serial.println("🚦 Traffic light control system ready!");
        Serial.println("📱 Use your MicroSafari dashboard to control the lights");
        Serial.println("🔄 System will poll for commands every 2 seconds");
        Serial.println();
        
        // Startup light sequence
        Serial.println("🎨 Running startup sequence...");
        startupLightSequence();
        
    } else {
        Serial.println("❌ Platform connection failed!");
        Serial.println("Please check your API key and platform URL.");
        // Continue anyway for demonstration purposes
    }
}

void loop() {
    // Call the library's loop function
    microSafari.loop();
    
    // Poll for commands at regular intervals
    static unsigned long lastCommandPoll = 0;
    if (millis() - lastCommandPoll >= COMMAND_POLL_INTERVAL) {
        if (microSafari.isWiFiConnected()) {
            pollForCommands();
        }
        lastCommandPoll = millis();
    }
    
    // Send status update at regular intervals
    static unsigned long lastStatusUpdate = 0;
    if (millis() - lastStatusUpdate >= STATUS_INTERVAL) {
        if (microSafari.isWiFiConnected()) {
            sendTrafficLightStatus();
        }
        lastStatusUpdate = millis();
    }
    
    delay(100);  // Small delay to prevent watchdog issues
}

/**
 * @brief Poll for device commands from the dashboard
 */
void pollForCommands() {
    Serial.println("🔄 Polling for commands...");
    
    // Poll for commands using the library (this will be implemented in MicroSafari.cpp)
    // For now, we'll simulate command checking by calling a custom function
    checkDeviceCommands();
}

/**
 * @brief Check for device commands and execute them
 */
void checkDeviceCommands() {
    // Poll for commands from the MicroSafari platform
    MicroSafariResponse commandResponse = microSafari.pollCommands();
    
    if (commandResponse.success) {
        // Commands are automatically executed by the library's pollCommands() method
        // But we can also manually process the response if needed
        Serial.println("   ✅ Command poll successful");
    } else if (commandResponse.httpCode == 404) {
        // 404 means no command polling endpoint available - this is expected for now
        Serial.println("   ℹ️  Command polling endpoint not available (this is normal for now)");
    } else {
        Serial.printf("   ⚠️  Command poll failed: %s (HTTP %d)\n", 
                     commandResponse.errorMessage.c_str(), 
                     commandResponse.httpCode);
    }
}

/**
 * @brief Send current traffic light status to the platform
 * 
 * Note: This function uses sendSensorData() instead of sendCustomData() to ensure
 * the JSON payload is properly wrapped in the required 'payload' field structure
 * that the MicroSafari platform expects at /api/ingest endpoint.
 */
void sendTrafficLightStatus() {
    Serial.println("📡 Sending traffic light status...");
    
    // Create JSON payload using ArduinoJson library for proper structure
    DynamicJsonDocument doc(512);
    JsonObject trafficData = doc.to<JsonObject>();
    
    // Add traffic light states
    trafficData["red_light"] = redLightState ? 1 : 0;
    trafficData["yellow_light"] = yellowLightState ? 1 : 0;
    trafficData["green_light"] = greenLightState ? 1 : 0;
    
    // Add additional metadata
    trafficData["timestamp"] = millis();
    trafficData["device_name"] = DEVICE_NAME;
    trafficData["system_uptime"] = millis() / 1000;
    
    Serial.println("📊 Current Status:");
    Serial.printf("   🔴 Red Light: %s\n", redLightState ? "ON" : "OFF");
    Serial.printf("   🟡 Yellow Light: %s\n", yellowLightState ? "ON" : "OFF");
    Serial.printf("   🟢 Green Light: %s\n", greenLightState ? "ON" : "OFF");
    
    // Send using sendSensorData method which properly wraps the payload
    MicroSafariResponse response = microSafari.sendSensorData(trafficData);
    
    // Handle response
    if (response.success) {
        Serial.println("✅ Status sent successfully!");
        Serial.printf("   📅 Timestamp: %lu\n", millis());
        Serial.printf("   🔗 HTTP Code: %d\n", response.httpCode);
    } else {
        Serial.println("❌ Failed to send status!");
        Serial.printf("   🚨 Error: %s\n", response.errorMessage.c_str());
        Serial.printf("   🔗 HTTP Code: %d\n", response.httpCode);
    }
    
    Serial.println();
}

/**
 * @brief Control red LED based on command
 * @param state Boolean state (true = ON, false = OFF)
 */
void controlRedLight(bool state) {
    redLightState = state;
    digitalWrite(RED_LED_PIN, state ? HIGH : LOW);
    Serial.printf("🔴 Red light turned %s\n", state ? "ON" : "OFF");
}

/**
 * @brief Control yellow LED based on command
 * @param state Boolean state (true = ON, false = OFF)
 */
void controlYellowLight(bool state) {
    yellowLightState = state;
    digitalWrite(YELLOW_LED_PIN, state ? HIGH : LOW);
    Serial.printf("🟡 Yellow light turned %s\n", state ? "ON" : "OFF");
}

/**
 * @brief Control green LED based on command
 * @param state Boolean state (true = ON, false = OFF)
 */
void controlGreenLight(bool state) {
    greenLightState = state;
    digitalWrite(GREEN_LED_PIN, state ? HIGH : LOW);
    Serial.printf("🟢 Green light turned %s\n", state ? "ON" : "OFF");
}

/**
 * @brief Process incoming command from dashboard
 * This function is called by the MicroSafari library when commands are received
 * @param dataSource The data source key (e.g., "red_light", "yellow_light", "green_light")
 * @param value The command value (typically "1" for ON, "0" for OFF)
 * @return true if command was executed successfully, false otherwise
 */
bool handleCommand(const String& dataSource, const String& value) {
    Serial.printf("🎯 Executing command: %s = %s\n", dataSource.c_str(), value.c_str());
    
    bool state = (value == "1" || value.equalsIgnoreCase("true") || value.equalsIgnoreCase("on"));
    
    if (dataSource == "red_light") {
        controlRedLight(state);
        return true;
    } else if (dataSource == "yellow_light") {
        controlYellowLight(state);
        return true;
    } else if (dataSource == "green_light") {
        controlGreenLight(state);
        return true;
    } else {
        Serial.printf("⚠️  Unknown command: %s\n", dataSource.c_str());
        return false;
    }
}

/**
 * @brief Run a startup light sequence to test all LEDs
 */
void startupLightSequence() {
    Serial.println("🎨 Testing LEDs...");
    
    // Test each light individually
    controlRedLight(true);
    delay(500);
    controlRedLight(false);
    delay(200);
    
    controlYellowLight(true);
    delay(500);
    controlYellowLight(false);
    delay(200);
    
    controlGreenLight(true);
    delay(500);
    controlGreenLight(false);
    delay(200);
    
    // Flash all lights together
    Serial.println("💫 All lights test...");
    for (int i = 0; i < 3; i++) {
        controlRedLight(true);
        controlYellowLight(true);
        controlGreenLight(true);
        delay(200);
        
        controlRedLight(false);
        controlYellowLight(false);
        controlGreenLight(false);
        delay(200);
    }
    
    Serial.println("✅ LED test complete!");
    Serial.println();
}

/**
 * @brief Simulate a traditional traffic light pattern (for testing)
 * Call this function to run an automated sequence
 */
void runTrafficLightPattern() {
    Serial.println("🚦 Running traditional traffic light pattern...");
    
    // Green phase
    controlGreenLight(true);
    delay(5000);  // Green for 5 seconds
    
    // Yellow phase
    controlGreenLight(false);
    controlYellowLight(true);
    delay(2000);  // Yellow for 2 seconds
    
    // Red phase
    controlYellowLight(false);
    controlRedLight(true);
    delay(5000);  // Red for 5 seconds
    
    // Turn off red
    controlRedLight(false);
    
    Serial.println("🔄 Traffic light pattern complete!");
}