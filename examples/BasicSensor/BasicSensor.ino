/*!
 * @file BasicSensor.ino
 * @brief Basic sensor data transmission example for MicroSafari ESP32 Library
 *
 * This example demonstrates how to:
 * - Connect to WiFi and MicroSafari platform
 * - Send simulated sensor data (temperature, humidity, soil moisture, light)
 * - Handle response from the platform
 * - Implement periodic data transmission
 *
 * @version 1.0.0
 * @date 2025-08-22
 * @author MicroSafari Team
 */

#include <MicroSafari.h>
#include <HTTPClient.h>

// WiFi credentials - CHANGE THESE TO YOUR NETWORK
const char* WIFI_SSID = "your_wifi_ssid";
const char* WIFI_PASSWORD = "your_wifi_password";

// MicroSafari configuration - GET THESE FROM YOUR DASHBOARD
const char* API_KEY = "your_device_api_key";
const char* PLATFORM_URL = "https://your-microsafari-instance.com";
const char* DEVICE_NAME = "ESP32-Farm-Sensor-01";

// Sensor reading interval (in milliseconds)
const unsigned long SENSOR_INTERVAL = 5000; // Send data every 5 seconds (was 30 seconds)

// Create MicroSafari instance
MicroSafari microSafari;

void setup() {
    // Initialize Serial communication
    Serial.begin(115200);
    while (!Serial) {
        delay(10);
    }

    Serial.println();
    Serial.println("====================================");
    Serial.println("MicroSafari Basic Sensor Data Demo");
    Serial.println("====================================");

    // Enable debug output
    microSafari.setDebug(true);

    // Initialize MicroSafari library
    Serial.println("Initializing MicroSafari library...");
    Serial.printf("Config: SSID=%s, API_KEY=%s, URL=%s, DEVICE=%s\n", WIFI_SSID, API_KEY, PLATFORM_URL, DEVICE_NAME);
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

    // Add network diagnostics
    Serial.println("🔍 Network Diagnostics:");
    Serial.printf("   📍 Local IP: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("   🚪 Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
    Serial.printf("   🌐 DNS: %s\n", WiFi.dnsIP().toString().c_str());
    Serial.printf("   📶 Signal: %d dBm\n", WiFi.RSSI());

    // Test basic connectivity
    Serial.println("🔗 Testing basic connectivity...");
    HTTPClient testClient;
    testClient.begin("http://192.168.18.197:3000");
    testClient.setTimeout(5000);
    int testCode = testClient.GET();
    String testResponse = testClient.getString();
    testClient.end();

    Serial.printf("   Server response code: %d\n", testCode);
    if (testCode > 0) {
        Serial.println("   ✅ Server is reachable!");
    } else {
        Serial.println("   ❌ Cannot reach server");
        Serial.println("   💡 Check if server is running and accessible");
    }
    Serial.println();

    // Test platform connection
    Serial.println("Testing platform connection...");
    if (microSafari.testConnection()) {
        Serial.println("✅ Platform connection successful!");
        Serial.println();
        Serial.println("🌱 Ready to send sensor data...");
        Serial.println("📊 Data will be transmitted every 30 seconds");
        Serial.println();
    } else {
        Serial.println("❌ Platform connection failed!");
        Serial.println("Please check your API key and platform URL.");
        // Continue anyway for demonstration purposes
    }
}

void loop() {
    // Call the library's loop function
    microSafari.loop();

    // Send sensor data at regular intervals
    static unsigned long lastSensorReading = 0;
    if (millis() - lastSensorReading >= SENSOR_INTERVAL) {
        if (microSafari.isWiFiConnected()) {
            sendSensorData();
        } else {
            Serial.println("⚠️  WiFi disconnected - skipping sensor reading");
        }
        lastSensorReading = millis();
    }

    delay(1000);
}

/**
 * @brief Read sensors and send data to MicroSafari platform
 */
void sendSensorData() {
    Serial.println("📡 Reading sensors and sending data...");

    // Simulate sensor readings (replace with actual sensor code)
    float temperature = readTemperatureSensor();
    float humidity = readHumiditySensor();
    float soilMoisture = readSoilMoistureSensor();
    float lightLevel = readLightSensor();

    // Display readings
    Serial.println("📊 Sensor Readings:");
    Serial.printf("   🌡️  Temperature: %.2f°C\n", temperature);
    Serial.printf("   💧 Humidity: %.2f%%\n", humidity);
    Serial.printf("   🌱 Soil Moisture: %.2f%%\n", soilMoisture);
    Serial.printf("   ☀️  Light Level: %.2f lux\n", lightLevel);

    // Send data using the convenient method
    MicroSafariResponse response = microSafari.sendSensorData(temperature, humidity, soilMoisture, lightLevel);

    // Handle response
    if (response.success) {
        Serial.println("✅ Data sent successfully!");
        Serial.printf("   📅 Timestamp: %lu\n", millis());
        Serial.printf("   🔗 HTTP Code: %d\n", response.httpCode);
    } else {
        Serial.println("❌ Failed to send data!");
        Serial.printf("   🚨 Error: %s\n", response.errorMessage.c_str());
        Serial.printf("   🔗 HTTP Code: %d\n", response.httpCode);

        if (response.httpCode == 401) {
            Serial.println("   💡 Tip: Check your API key in the MicroSafari dashboard");
        } else if (response.httpCode == 503) {
            Serial.println("   💡 Tip: Platform may be in development mode");
        }
    }

    Serial.println();
}

/**
 * @brief Simulate temperature sensor reading (DHT22, DS18B20, etc.)
 * Replace this with actual sensor code
 */
float readTemperatureSensor() {
    // Simulate realistic Indonesian farm temperature (24-35°C)
    return 24.0 + (random(0, 1100) / 100.0);
}

/**
 * @brief Simulate humidity sensor reading (DHT22, SHT30, etc.)
 * Replace this with actual sensor code
 */
float readHumiditySensor() {
    // Simulate realistic Indonesian humidity (60-90%)
    return 60.0 + (random(0, 3000) / 100.0);
}

/**
 * @brief Simulate soil moisture sensor reading (capacitive, resistive)
 * Replace this with actual sensor code
 */
float readSoilMoistureSensor() {
    // Simulate soil moisture (0-100%)
    return random(0, 10000) / 100.0;
}

/**
 * @brief Simulate light sensor reading (BH1750, LDR, etc.)
 * Replace this with actual sensor code
 */
float readLightSensor() {
    // Simulate light levels (0-1000 lux)
    // Higher during day, lower at night
    unsigned long currentTime = millis() / 1000; // seconds
    float baseLight = 100 + (sin(currentTime / 3600.0) * 400); // Simulate day/night cycle
    return max(0.0f, baseLight + (random(-100, 100)));
}