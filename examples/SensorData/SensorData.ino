/*!
 * @file SensorData.ino
 * @brief Sensor data transmission testing example for MicroSafari
 * 
 * This example focuses on:
 * - Testing different data transmission methods
 * - Validating API endpoint integration
 * - Demonstrating error handling
 * - Performance monitoring
 * 
 * @version 1.0.0
 * @date 2025-08-22
 * @author MicroSafari Team
 */

#include <MicroSafari.h>

// Configuration
const char* WIFI_SSID = "your_wifi_ssid";
const char* WIFI_PASSWORD = "your_wifi_password";
const char* API_KEY = "your_device_api_key";
const char* PLATFORM_URL = "https://your-microsafari-instance.com";
const char* DEVICE_NAME = "ESP32-Data-Test";

// Test parameters
const unsigned long TEST_INTERVAL = 15000; // Test every 15 seconds
const int NUM_TEST_CYCLES = 5;

MicroSafari microSafari;
int testCycle = 0;

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);
    
    Serial.println();
    Serial.println("=====================================");
    Serial.println("MicroSafari Data Transmission Tester");
    Serial.println("=====================================");
    Serial.println();
    
    microSafari.setDebug(true);
    
    // Initialize and connect
    if (!microSafari.begin(WIFI_SSID, WIFI_PASSWORD, API_KEY, PLATFORM_URL, DEVICE_NAME)) {
        Serial.println("❌ Initialization failed!");
        while (true) delay(1000);
    }
    
    if (!microSafari.connectWiFi()) {
        Serial.println("❌ WiFi connection failed!");
        while (true) delay(1000);
    }
    
    Serial.println("✅ System ready for testing!");
    Serial.printf("📊 Will run %d test cycles\n", NUM_TEST_CYCLES);
    Serial.println();
}

void loop() {
    microSafari.loop();
    
    static unsigned long lastTest = 0;
    
    if (millis() - lastTest >= TEST_INTERVAL && testCycle < NUM_TEST_CYCLES) {
        runTestCycle(testCycle + 1);
        testCycle++;
        lastTest = millis();
    }
    
    if (testCycle >= NUM_TEST_CYCLES) {
        Serial.println("🎉 All test cycles completed!");
        Serial.println("Check your MicroSafari dashboard for received data.");
        while (true) delay(1000);
    }
    
    delay(1000);
}

/**
 * @brief Run a complete test cycle
 */
void runTestCycle(int cycle) {
    Serial.printf("🧪 Test Cycle %d/%d\n", cycle, NUM_TEST_CYCLES);
    Serial.println("====================");
    
    // Test 1: Simple parameter method
    testSimpleParameterMethod(cycle);
    delay(2000);
    
    // Test 2: JSON object method
    testJsonObjectMethod(cycle);
    delay(2000);
    
    // Test 3: Complex data structure
    testComplexDataStructure(cycle);
    
    Serial.println("✅ Test cycle completed");
    Serial.println();
}

/**
 * @brief Test the simple parameter-based sending method
 */
void testSimpleParameterMethod(int cycle) {
    Serial.println("📤 Test 1: Simple Parameter Method");
    
    unsigned long startTime = millis();
    
    float temp = 25.0 + cycle;
    float humidity = 70.0 + (cycle * 2);
    float soilMoisture = 50.0 + (cycle * 3);
    float lightLevel = 500.0 + (cycle * 50);
    
    Serial.printf("   🌡️  Temp: %.1f°C, 💧 Humidity: %.1f%%, 🌱 Soil: %.1f%%, ☀️  Light: %.1f lux\n", 
                  temp, humidity, soilMoisture, lightLevel);
    
    MicroSafariResponse response = microSafari.sendSensorData(temp, humidity, soilMoisture, lightLevel);
    
    unsigned long duration = millis() - startTime;
    
    if (response.success) {
        Serial.printf("   ✅ Success! Duration: %lu ms\n", duration);
    } else {
        Serial.printf("   ❌ Failed: %s (Duration: %lu ms)\n", response.errorMessage.c_str(), duration);
    }
}

/**
 * @brief Test the JSON object method
 */
void testJsonObjectMethod(int cycle) {
    Serial.println("📤 Test 2: JSON Object Method");
    
    unsigned long startTime = millis();
    
    // Create JSON payload
    DynamicJsonDocument doc(512);
    JsonObject data = doc.to<JsonObject>();
    
    data["temperature"] = 28.0 + cycle;
    data["humidity"] = 75.0 + cycle;
    data["soil_ph"] = 6.5 + (cycle * 0.1);
    data["wind_speed"] = 5.0 + (cycle * 0.5);
    data["test_cycle"] = cycle;
    data["test_type"] = "json_object";
    
    Serial.println("   📋 JSON Data:");
    serializeJsonPretty(data, Serial);
    Serial.println();
    
    MicroSafariResponse response = microSafari.sendSensorData(data);
    
    unsigned long duration = millis() - startTime;
    
    if (response.success) {
        Serial.printf("   ✅ Success! Duration: %lu ms\n", duration);
    } else {
        Serial.printf("   ❌ Failed: %s (Duration: %lu ms)\n", response.errorMessage.c_str(), duration);
    }
}

/**
 * @brief Test complex nested data structure
 */
void testComplexDataStructure(int cycle) {
    Serial.println("📤 Test 3: Complex Data Structure");
    
    unsigned long startTime = millis();
    
    // Create complex nested structure
    DynamicJsonDocument doc(1024);
    JsonObject data = doc.to<JsonObject>();
    
    // Environmental data
    JsonObject env = data.createNestedObject("environment");
    env["temperature"] = 30.0 + cycle;
    env["humidity"] = 80.0 + cycle;
    env["pressure"] = 1013.25 + cycle;
    
    // Soil array with multiple sensor readings
    JsonArray soilSensors = data.createNestedArray("soil_sensors");
    for (int i = 0; i < 3; i++) {
        JsonObject sensor = soilSensors.createNestedObject();
        sensor["id"] = i + 1;
        sensor["depth_cm"] = (i + 1) * 10;
        sensor["moisture"] = 45.0 + (cycle * 2) + (i * 5);
        sensor["temperature"] = 24.0 + cycle + (i * 0.5);
    }
    
    // Metadata
    JsonObject meta = data.createNestedObject("metadata");
    meta["test_cycle"] = cycle;
    meta["test_type"] = "complex_structure";
    meta["timestamp"] = millis();
    meta["device_info"] = "ESP32-Test-Station";
    
    Serial.println("   📋 Complex JSON Data:");
    serializeJsonPretty(data, Serial);
    Serial.println();
    
    MicroSafariResponse response = microSafari.sendSensorData(data);
    
    unsigned long duration = millis() - startTime;
    
    if (response.success) {
        Serial.printf("   ✅ Success! Duration: %lu ms\n", duration);
        
        // Parse response for additional info
        DynamicJsonDocument responseDoc(512);
        if (deserializeJson(responseDoc, response.payload) == DeserializationError::Ok) {
            if (responseDoc["device_name"]) {
                Serial.printf("   📱 Device Name: %s\n", responseDoc["device_name"].as<const char*>());
            }
            if (responseDoc["timestamp"]) {
                Serial.printf("   ⏰ Server Timestamp: %s\n", responseDoc["timestamp"].as<const char*>());
            }
        }
    } else {
        Serial.printf("   ❌ Failed: %s (Duration: %lu ms)\n", response.errorMessage.c_str(), duration);
        
        // Additional debugging for complex structures
        if (response.httpCode == 400) {
            Serial.println("   💡 Tip: Check if JSON structure matches API expectations");
        }
    }
}