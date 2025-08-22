/*!
 * @file AdvancedSensor.ino
 * @brief Advanced sensor data example with custom JSON payload for MicroSafari
 * 
 * This example demonstrates:
 * - Custom JSON payload creation
 * - Multiple sensor types and readings
 * - Error handling and retry logic
 * - Status LED indicators
 * - Battery monitoring
 * - GPS location data (simulated)
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
const char* DEVICE_NAME = "ESP32-Advanced-Farm-Station";

// Hardware pins
const int LED_STATUS_PIN = 2;      // Built-in LED for status
const int LED_ERROR_PIN = 4;       // External LED for errors
const int BATTERY_PIN = A0;        // Battery voltage monitoring

// Timing constants
const unsigned long SENSOR_INTERVAL = 60000; // 1 minute between readings
const unsigned long RETRY_INTERVAL = 10000;  // 10 seconds between retries
const int MAX_RETRIES = 3;

// Farm location (simulated GPS coordinates for Indonesian farm)
const float FARM_LATITUDE = -6.2088;   // Jakarta area
const float FARM_LONGITUDE = 106.8456;

// Create MicroSafari instance
MicroSafari microSafari;

// Global variables
unsigned long lastSensorReading = 0;
unsigned long lastRetry = 0;
int failedAttempts = 0;
bool systemError = false;

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);
    
    // Initialize hardware
    pinMode(LED_STATUS_PIN, OUTPUT);
    pinMode(LED_ERROR_PIN, OUTPUT);
    pinMode(BATTERY_PIN, INPUT);
    
    // Startup LED sequence
    blinkLED(LED_STATUS_PIN, 3, 200);
    
    Serial.println();
    Serial.println("=========================================");
    Serial.println("MicroSafari Advanced Sensor Data System");
    Serial.println("=========================================");
    Serial.println("🚜 Indonesian Smart Farming IoT Station");
    Serial.println();
    
    // Initialize MicroSafari
    microSafari.setDebug(true);
    
    if (!microSafari.begin(WIFI_SSID, WIFI_PASSWORD, API_KEY, PLATFORM_URL, DEVICE_NAME)) {
        Serial.println("❌ MicroSafari initialization failed!");
        systemError = true;
        return;
    }
    
    // Connect to WiFi with retries
    if (!connectWithRetry()) {
        Serial.println("❌ WiFi connection failed after retries!");
        systemError = true;
        return;
    }
    
    // Test platform connection
    if (microSafari.testConnection()) {
        Serial.println("✅ Platform connection verified!");
        digitalWrite(LED_STATUS_PIN, HIGH);
    } else {
        Serial.println("⚠️  Platform connection test failed, but continuing...");
    }
    
    Serial.println();
    Serial.println("🌱 Advanced sensor system ready!");
    Serial.println("📊 Collecting comprehensive farm data...");
    Serial.println();
}

void loop() {
    if (systemError) {
        errorLoop();
        return;
    }
    
    microSafari.loop();
    
    // Handle sensor readings
    if (millis() - lastSensorReading >= SENSOR_INTERVAL) {
        if (microSafari.isWiFiConnected()) {
            sendAdvancedSensorData();
            lastSensorReading = millis();
            failedAttempts = 0;
        } else {
            handleConnectionFailure();
        }
    }
    
    // Update status LED
    updateStatusLED();
    
    delay(1000);
}

/**
 * @brief Send comprehensive sensor data with custom JSON structure
 */
void sendAdvancedSensorData() {
    Serial.println("📡 Collecting comprehensive sensor data...");
    
    // Create detailed JSON payload
    DynamicJsonDocument doc(1024);
    JsonObject sensorData = doc.to<JsonObject>();
    
    // Environmental sensors
    JsonObject environment = sensorData.createNestedObject("environment");
    environment["temperature"] = readTemperatureSensor();
    environment["humidity"] = readHumiditySensor();
    environment["air_pressure"] = readPressureSensor();
    environment["uv_index"] = readUVSensor();
    
    // Soil sensors
    JsonObject soil = sensorData.createNestedObject("soil");
    soil["moisture"] = readSoilMoistureSensor();
    soil["ph"] = readSoilPHSensor();
    soil["ec"] = readSoilECSensor();
    soil["temperature"] = readSoilTemperatureSensor();
    
    // Light sensors
    JsonObject light = sensorData.createNestedObject("light");
    light["visible"] = readVisibleLightSensor();
    light["infrared"] = readInfraredSensor();
    light["uv"] = readUVSensor();
    
    // System information
    JsonObject system = sensorData.createNestedObject("system");
    system["battery_voltage"] = readBatteryVoltage();
    system["wifi_rssi"] = microSafari.getWiFiSignalStrength();
    system["uptime_seconds"] = millis() / 1000;
    system["free_heap"] = ESP.getFreeHeap();
    
    // Location information
    JsonObject location = sensorData.createNestedObject("location");
    location["latitude"] = FARM_LATITUDE;
    location["longitude"] = FARM_LONGITUDE;
    location["altitude"] = 25.0; // meters above sea level
    
    // Metadata
    sensorData["timestamp"] = millis();
    sensorData["device_name"] = DEVICE_NAME;
    sensorData["firmware_version"] = "1.0.0";
    sensorData["sensor_type"] = "advanced_farm_station";
    
    // Display readings
    displaySensorReadings(sensorData);
    
    // Send data to platform
    MicroSafariResponse response = microSafari.sendSensorData(sensorData);
    
    if (response.success) {
        Serial.println("✅ Advanced sensor data transmitted successfully!");
        Serial.printf("   📅 Server timestamp: %s\n", response.payload.c_str());
        digitalWrite(LED_ERROR_PIN, LOW);
        failedAttempts = 0;
    } else {
        Serial.println("❌ Failed to transmit sensor data!");
        Serial.printf("   🚨 Error: %s\n", response.errorMessage.c_str());
        handleTransmissionFailure(response);
    }
    
    Serial.println();
}

/**
 * @brief Display sensor readings in organized format
 */
void displaySensorReadings(const JsonObject& data) {
    Serial.println("📊 Current Sensor Readings:");
    Serial.println("   🌡️  Environmental:");
    Serial.printf("      Temperature: %.2f°C\n", data["environment"]["temperature"].as<float>());
    Serial.printf("      Humidity: %.2f%%\n", data["environment"]["humidity"].as<float>());
    Serial.printf("      Pressure: %.2f hPa\n", data["environment"]["air_pressure"].as<float>());
    
    Serial.println("   🌱 Soil Conditions:");
    Serial.printf("      Moisture: %.2f%%\n", data["soil"]["moisture"].as<float>());
    Serial.printf("      pH: %.2f\n", data["soil"]["ph"].as<float>());
    Serial.printf("      EC: %.2f mS/cm\n", data["soil"]["ec"].as<float>());
    
    Serial.println("   ☀️  Light Levels:");
    Serial.printf("      Visible: %.2f lux\n", data["light"]["visible"].as<float>());
    Serial.printf("      UV Index: %.2f\n", data["light"]["uv"].as<float>());
    
    Serial.println("   🔋 System Status:");
    Serial.printf("      Battery: %.2fV\n", data["system"]["battery_voltage"].as<float>());
    Serial.printf("      WiFi: %d dBm\n", data["system"]["wifi_rssi"].as<int>());
}

/**
 * @brief Handle transmission failures with retry logic
 */
void handleTransmissionFailure(const MicroSafariResponse& response) {
    failedAttempts++;
    digitalWrite(LED_ERROR_PIN, HIGH);
    
    if (response.httpCode == 401) {
        Serial.println("   🔑 Authentication issue - check API key");
    } else if (response.httpCode == 503) {
        Serial.println("   🔧 Service unavailable - development mode");
    } else if (response.httpCode <= 0) {
        Serial.println("   📶 Network connectivity issue");
    }
    
    if (failedAttempts >= MAX_RETRIES) {
        Serial.printf("   ⚠️  Maximum retries reached (%d), will try again next cycle\n", MAX_RETRIES);
        failedAttempts = 0;
    }
}

/**
 * @brief Handle WiFi connection failures
 */
void handleConnectionFailure() {
    if (millis() - lastRetry >= RETRY_INTERVAL) {
        Serial.println("📶 WiFi disconnected, attempting reconnection...");
        if (microSafari.connectWiFi(15000)) {
            Serial.println("✅ WiFi reconnected successfully!");
        }
        lastRetry = millis();
    }
}

/**
 * @brief Connect to WiFi with retry logic
 */
bool connectWithRetry() {
    int attempts = 0;
    while (attempts < MAX_RETRIES) {
        Serial.printf("📶 WiFi connection attempt %d/%d...\n", attempts + 1, MAX_RETRIES);
        if (microSafari.connectWiFi(30000)) {
            return true;
        }
        attempts++;
        if (attempts < MAX_RETRIES) {
            delay(5000);
        }
    }
    return false;
}

/**
 * @brief Update status LED based on system state
 */
void updateStatusLED() {
    static unsigned long lastBlink = 0;
    static bool ledState = false;
    
    if (microSafari.getStatus() == MICROSAFARI_PLATFORM_CONNECTED) {
        digitalWrite(LED_STATUS_PIN, HIGH); // Solid on when connected
    } else if (microSafari.isWiFiConnected()) {
        // Slow blink when WiFi connected but platform not verified
        if (millis() - lastBlink >= 1000) {
            ledState = !ledState;
            digitalWrite(LED_STATUS_PIN, ledState);
            lastBlink = millis();
        }
    } else {
        // Fast blink when disconnected
        if (millis() - lastBlink >= 250) {
            ledState = !ledState;
            digitalWrite(LED_STATUS_PIN, ledState);
            lastBlink = millis();
        }
    }
}

/**
 * @brief Error loop for system failures
 */
void errorLoop() {
    static unsigned long lastBlink = 0;
    static bool ledState = false;
    
    if (millis() - lastBlink >= 100) {
        ledState = !ledState;
        digitalWrite(LED_ERROR_PIN, ledState);
        lastBlink = millis();
    }
    
    Serial.println("💥 System in error state - please restart");
    delay(5000);
}

/**
 * @brief Blink LED for startup indication
 */
void blinkLED(int pin, int times, int delayMs) {
    for (int i = 0; i < times; i++) {
        digitalWrite(pin, HIGH);
        delay(delayMs);
        digitalWrite(pin, LOW);
        delay(delayMs);
    }
}

// ========================================
// Sensor Reading Functions (Simulated)
// Replace these with actual sensor code
// ========================================

float readTemperatureSensor() {
    return 25.0 + (random(0, 1000) / 100.0); // 25-35°C
}

float readHumiditySensor() {
    return 65.0 + (random(0, 2500) / 100.0); // 65-90%
}

float readPressureSensor() {
    return 1013.25 + (random(-50, 50) / 10.0); // ±5 hPa variation
}

float readSoilMoistureSensor() {
    return random(0, 10000) / 100.0; // 0-100%
}

float readSoilPHSensor() {
    return 6.0 + (random(0, 200) / 100.0); // pH 6.0-8.0
}

float readSoilECSensor() {
    return 0.5 + (random(0, 300) / 100.0); // 0.5-3.5 mS/cm
}

float readSoilTemperatureSensor() {
    return 22.0 + (random(0, 800) / 100.0); // 22-30°C
}

float readVisibleLightSensor() {
    unsigned long currentTime = millis() / 1000;
    float baseLight = 200 + (sin(currentTime / 3600.0) * 600); // Day/night cycle
    return max(0.0f, baseLight);
}

float readInfraredSensor() {
    return random(0, 1000) / 10.0; // 0-100 W/m²
}

float readUVSensor() {
    unsigned long currentTime = millis() / 1000;
    float baseUV = 3 + (sin(currentTime / 3600.0) * 7); // 0-10 UV index
    return max(0.0f, baseUV);
}

float readBatteryVoltage() {
    int rawValue = analogRead(BATTERY_PIN);
    return (rawValue / 4095.0) * 3.3 * 2; // Voltage divider assumed
}