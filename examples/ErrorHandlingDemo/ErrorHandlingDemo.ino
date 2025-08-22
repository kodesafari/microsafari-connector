/*!
 * @file ErrorHandlingDemo.ino
 * @brief Error handling and connection resilience example for MicroSafari ESP32 Library
 * 
 * This example demonstrates:
 * - Robust error handling strategies
 * - Automatic reconnection mechanisms
 * - Failure recovery procedures
 * - Connection resilience patterns
 * - Data buffering during outages
 * - Smart retry algorithms
 * 
 * Essential for: Remote farm deployments, unreliable network conditions,
 * power fluctuations, and ensuring continuous data collection for
 * Indonesian agricultural IoT systems.
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
const char* DEVICE_NAME = "ESP32-Resilient-Farm-Node";

// Configuration constants
const unsigned long SENSOR_INTERVAL = 30000;      // 30 seconds between readings
const unsigned long ERROR_CHECK_INTERVAL = 10000; // Check for errors every 10 seconds
const int MAX_BUFFERED_READINGS = 50;             // Maximum readings to buffer offline
const int CRITICAL_FAILURE_THRESHOLD = 10;        // Reset after 10 consecutive failures

// Hardware pins
const int LED_STATUS_PIN = 2;
const int LED_ERROR_PIN = 4;
const int RESET_BUTTON_PIN = 0; // Boot button for manual reset

// Create MicroSafari instance
MicroSafari microSafari;

// Data buffering for offline operation
struct SensorReading {
    unsigned long timestamp;
    float temperature;
    float humidity;
    float soilMoisture;
    bool sent;
};

SensorReading readingBuffer[MAX_BUFFERED_READINGS];
int bufferHead = 0;
int bufferTail = 0;
int bufferedCount = 0;

// Error tracking
unsigned long totalFailures = 0;
unsigned long totalSuccesses = 0;
unsigned long lastErrorCheck = 0;
bool emergencyMode = false;

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);
    
    // Initialize hardware
    pinMode(LED_STATUS_PIN, OUTPUT);
    pinMode(LED_ERROR_PIN, OUTPUT);
    pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
    
    Serial.println();
    Serial.println("==============================================");
    Serial.println("MicroSafari Error Handling & Resilience Demo");
    Serial.println("==============================================");
    Serial.println("🛡️  Robust IoT for Indonesian Smart Farming");
    Serial.println();
    
    // Configure MicroSafari for maximum resilience
    microSafari.setDebug(true);
    microSafari.setRetryConfig(5, 2000);              // 5 retries with 2s delay
    microSafari.setMaxConsecutiveFailures(5);         // Reset after 5 failures
    microSafari.setAutoReconnect(true);               // Enable auto-reconnect
    microSafari.setHeartbeatInterval(180000);         // 3 minute heartbeat
    microSafari.setConnectionTimeout(20000);          // 20 second WiFi timeout
    
    // Initialize buffer
    initializeBuffer();
    
    // Startup sequence with error handling
    if (!initializeSystem()) {
        Serial.println("❌ System initialization failed!");
        Serial.println("🔄 Entering emergency mode...");
        emergencyMode = true;
    } else {
        Serial.println("✅ System initialized successfully!");
        Serial.println("🛡️  Error handling and resilience active");
        Serial.println("📊 Ready for robust sensor data collection");
    }
    
    Serial.println();
    displaySystemCapabilities();
}

void loop() {
    // Check for manual reset request
    if (digitalRead(RESET_BUTTON_PIN) == LOW) {
        delay(2000); // Debounce
        if (digitalRead(RESET_BUTTON_PIN) == LOW) {
            performSystemReset();
        }
    }
    
    // Handle emergency mode
    if (emergencyMode) {
        handleEmergencyMode();
        return;
    }
    
    // Main library loop (handles auto-reconnect and heartbeats)
    microSafari.loop();
    
    // Collect and process sensor data
    static unsigned long lastSensorReading = 0;
    if (millis() - lastSensorReading >= SENSOR_INTERVAL) {
        collectAndProcessSensorData();
        lastSensorReading = millis();
    }
    
    // Process buffered data if connected
    if (microSafari.isPlatformActive() && bufferedCount > 0) {
        processBufferedData();
    }
    
    // Regular error checking and recovery
    if (millis() - lastErrorCheck >= ERROR_CHECK_INTERVAL) {
        performErrorCheck();
        lastErrorCheck = millis();
    }
    
    // Update status indicators
    updateStatusIndicators();
    
    delay(1000);
}

/**
 * @brief Initialize system with comprehensive error handling
 */
bool initializeSystem() {
    Serial.println("🚀 Initializing system components...");
    
    // Initialize MicroSafari library
    if (!microSafari.begin(WIFI_SSID, WIFI_PASSWORD, API_KEY, PLATFORM_URL, DEVICE_NAME)) {
        Serial.println("❌ MicroSafari library initialization failed!");
        return false;
    }
    Serial.println("✅ MicroSafari library initialized");
    
    // Attempt WiFi connection with retries
    int wifiAttempts = 0;
    const int maxWifiAttempts = 3;
    
    while (wifiAttempts < maxWifiAttempts) {
        Serial.printf("📶 WiFi connection attempt %d/%d...\n", wifiAttempts + 1, maxWifiAttempts);
        
        if (microSafari.connectWiFi(30000)) {
            Serial.println("✅ WiFi connected successfully!");
            break;
        }
        
        wifiAttempts++;
        if (wifiAttempts < maxWifiAttempts) {
            Serial.println("⚠️  WiFi connection failed, retrying...");
            delay(5000);
        }
    }
    
    if (wifiAttempts >= maxWifiAttempts) {
        Serial.println("❌ WiFi connection failed after all attempts");
        Serial.println("💡 System will continue in offline mode");
        return true; // Continue in offline mode
    }
    
    // Test platform connectivity
    Serial.println("🔗 Testing platform connectivity...");
    if (microSafari.runConnectivityTest()) {
        Serial.println("✅ Platform connectivity verified!");
        return true;
    } else {
        Serial.println("⚠️  Platform connectivity test failed");
        Serial.println("💡 System will operate with limited functionality");
        return true; // Continue with limited functionality
    }
}

/**
 * @brief Collect sensor data with error handling
 */
void collectAndProcessSensorData() {
    Serial.println("📊 Collecting sensor data...");
    
    // Read sensors with error checking
    float temperature = readTemperatureWithRetry();
    float humidity = readHumidityWithRetry();
    float soilMoisture = readSoilMoistureWithRetry();
    
    // Validate sensor readings
    if (!validateSensorReadings(temperature, humidity, soilMoisture)) {
        Serial.println("❌ Sensor readings validation failed!");
        digitalWrite(LED_ERROR_PIN, HIGH);
        return;
    }
    
    // Create sensor reading
    SensorReading reading;
    reading.timestamp = millis();
    reading.temperature = temperature;
    reading.humidity = humidity;
    reading.soilMoisture = soilMoisture;
    reading.sent = false;
    
    // Display readings
    displaySensorReading(reading);
    
    // Try to send immediately if connected
    if (microSafari.isPlatformActive()) {
        if (sendSensorReading(reading)) {
            totalSuccesses++;
            Serial.println("✅ Data sent successfully!");
            digitalWrite(LED_ERROR_PIN, LOW);
        } else {
            totalFailures++;
            Serial.println("❌ Failed to send data, buffering for later...");
            bufferSensorReading(reading);
        }
    } else {
        Serial.println("📦 Platform not active, buffering data...");
        bufferSensorReading(reading);
    }
}

/**
 * @brief Send sensor reading with comprehensive error handling
 */
bool sendSensorReading(const SensorReading& reading) {
    // Create JSON payload
    DynamicJsonDocument doc(512);
    JsonObject sensorData = doc.to<JsonObject>();
    
    sensorData["temperature"] = reading.temperature;
    sensorData["humidity"] = reading.humidity;
    sensorData["soil_moisture"] = reading.soilMoisture;
    sensorData["timestamp"] = reading.timestamp;
    sensorData["device_name"] = DEVICE_NAME;
    sensorData["buffered"] = false;
    
    // Add system health data
    JsonObject health = sensorData.createNestedObject("system_health");
    health["free_heap"] = ESP.getFreeHeap();
    health["wifi_rssi"] = microSafari.getWiFiSignalStrength();
    health["uptime"] = millis() / 1000;
    health["total_failures"] = totalFailures;
    health["total_successes"] = totalSuccesses;
    health["buffered_count"] = bufferedCount;
    
    // Attempt to send
    MicroSafariResponse response = microSafari.sendSensorData(sensorData);
    
    if (response.success) {
        return true;
    } else {
        Serial.printf("   🚨 Send failed: %s\n", response.errorMessage.c_str());
        
        // Handle specific error types
        if (response.httpCode == 401) {
            Serial.println("   🔑 Authentication error - check API key");
        } else if (response.httpCode == 503) {
            Serial.println("   🔧 Service unavailable - development mode");
        } else if (response.httpCode <= 0) {
            Serial.println("   📶 Network error - connection issue");
        }
        
        return false;
    }
}

/**
 * @brief Buffer sensor reading for later transmission
 */
void bufferSensorReading(const SensorReading& reading) {
    if (bufferedCount >= MAX_BUFFERED_READINGS) {
        Serial.println("⚠️  Buffer full, overwriting oldest reading");
        bufferTail = (bufferTail + 1) % MAX_BUFFERED_READINGS;
    } else {
        bufferedCount++;
    }
    
    readingBuffer[bufferHead] = reading;
    bufferHead = (bufferHead + 1) % MAX_BUFFERED_READINGS;
    
    Serial.printf("📦 Reading buffered (%d/%d buffer used)\n", bufferedCount, MAX_BUFFERED_READINGS);
}

/**
 * @brief Process buffered data when connection is restored
 */
void processBufferedData() {
    Serial.printf("📤 Processing %d buffered readings...\n", bufferedCount);
    
    int processed = 0;
    int maxProcessPerCycle = 5; // Limit to prevent overwhelming
    
    while (bufferedCount > 0 && processed < maxProcessPerCycle) {
        SensorReading& reading = readingBuffer[bufferTail];
        
        if (!reading.sent) {
            // Mark as buffered data for platform
            DynamicJsonDocument doc(512);
            JsonObject sensorData = doc.to<JsonObject>();
            
            sensorData["temperature"] = reading.temperature;
            sensorData["humidity"] = reading.humidity;
            sensorData["soil_moisture"] = reading.soilMoisture;
            sensorData["timestamp"] = reading.timestamp;
            sensorData["device_name"] = DEVICE_NAME;
            sensorData["buffered"] = true;
            sensorData["buffer_delay"] = (millis() - reading.timestamp) / 1000;
            
            MicroSafariResponse response = microSafari.sendSensorData(sensorData);
            
            if (response.success) {
                reading.sent = true;
                totalSuccesses++;
                Serial.printf("✅ Buffered reading sent (timestamp: %lu)\n", reading.timestamp);
            } else {
                totalFailures++;
                Serial.printf("❌ Failed to send buffered reading: %s\n", response.errorMessage.c_str());
                break; // Stop processing if we're having issues
            }
        }
        
        // Remove from buffer
        bufferTail = (bufferTail + 1) % MAX_BUFFERED_READINGS;
        bufferedCount--;
        processed++;
    }
    
    if (bufferedCount == 0) {
        Serial.println("✅ All buffered data processed successfully!");
    } else {
        Serial.printf("📦 %d readings still buffered for next cycle\n", bufferedCount);
    }
}

/**
 * @brief Perform comprehensive error checking and recovery
 */
void performErrorCheck() {
    // Check for critical failure conditions
    float failureRate = (totalFailures + totalSuccesses > 0) ? 
                       (float)totalFailures / (totalFailures + totalSuccesses) : 0;
    
    if (totalFailures > CRITICAL_FAILURE_THRESHOLD && failureRate > 0.8) {
        Serial.println("🚨 CRITICAL: High failure rate detected!");
        Serial.printf("   Failures: %lu, Successes: %lu (%.1f%% failure rate)\n", 
                     totalFailures, totalSuccesses, failureRate * 100);
        
        performCriticalRecovery();
    }
    
    // Check system resources
    if (ESP.getFreeHeap() < 10000) {
        Serial.println("⚠️  WARNING: Low memory detected!");
        Serial.printf("   Free heap: %d bytes\n", ESP.getFreeHeap());
        
        // Clear some buffer space if needed
        if (bufferedCount > MAX_BUFFERED_READINGS / 2) {
            Serial.println("🧹 Clearing some buffered data to free memory...");
            bufferedCount = MAX_BUFFERED_READINGS / 2;
        }
    }
    
    // Check buffer status
    if (bufferedCount >= MAX_BUFFERED_READINGS * 0.9) {
        Serial.println("⚠️  WARNING: Buffer nearly full!");
        Serial.printf("   Buffer usage: %d/%d (%.1f%%)\n", 
                     bufferedCount, MAX_BUFFERED_READINGS, 
                     (float)bufferedCount / MAX_BUFFERED_READINGS * 100);
    }
    
    // Check last error
    String lastError = microSafari.getLastError();
    if (lastError != "No errors recorded") {
        Serial.println("🔍 Recent error detected: " + lastError);
    }
}

/**
 * @brief Perform critical system recovery
 */
void performCriticalRecovery() {
    Serial.println("🔄 Performing critical system recovery...");
    
    // Clear error history
    microSafari.clearErrors();
    
    // Reset failure counters
    totalFailures = 0;
    totalSuccesses = 0;
    
    // Force connectivity test
    Serial.println("🔍 Running comprehensive connectivity test...");
    if (microSafari.runConnectivityTest()) {
        Serial.println("✅ Connectivity test passed after recovery!");
    } else {
        Serial.println("❌ Connectivity test still failing");
        Serial.println("🔧 Detailed diagnostics:");
        Serial.println(microSafari.getConnectionDiagnostics());
    }
    
    Serial.println("🔄 Critical recovery complete");
}

/**
 * @brief Handle emergency mode operation
 */
void handleEmergencyMode() {
    static unsigned long lastEmergencyLog = 0;
    
    if (millis() - lastEmergencyLog >= 30000) { // Log every 30 seconds
        Serial.println("🚨 EMERGENCY MODE: System operating with limited functionality");
        Serial.println("💡 Press and hold boot button for 2 seconds to attempt recovery");
        lastEmergencyLog = millis();
    }
    
    // Flash error LED rapidly
    static unsigned long lastBlink = 0;
    static bool ledState = false;
    
    if (millis() - lastBlink >= 200) {
        ledState = !ledState;
        digitalWrite(LED_ERROR_PIN, ledState);
        lastBlink = millis();
    }
    
    delay(1000);
}

/**
 * @brief Perform manual system reset
 */
void performSystemReset() {
    Serial.println("🔄 Manual system reset requested...");
    
    // Clear all states
    emergencyMode = false;
    totalFailures = 0;
    totalSuccesses = 0;
    initializeBuffer();
    
    // Reset MicroSafari
    microSafari.clearErrors();
    microSafari.disconnect();
    
    // Attempt reinitialization
    if (initializeSystem()) {
        Serial.println("✅ System reset successful!");
        digitalWrite(LED_ERROR_PIN, LOW);
    } else {
        Serial.println("❌ System reset failed, returning to emergency mode");
        emergencyMode = true;
    }
}

/**
 * @brief Update visual status indicators
 */
void updateStatusIndicators() {
    static unsigned long lastUpdate = 0;
    static bool blinkState = false;
    
    if (millis() - lastUpdate >= 500) {
        if (emergencyMode) {
            // Fast blink for emergency
            blinkState = !blinkState;
            digitalWrite(LED_ERROR_PIN, blinkState);
            digitalWrite(LED_STATUS_PIN, LOW);
        } else if (microSafari.isPlatformActive()) {
            // Solid on when fully connected
            digitalWrite(LED_STATUS_PIN, HIGH);
            digitalWrite(LED_ERROR_PIN, LOW);
        } else if (microSafari.isWiFiConnected()) {
            // Slow blink when WiFi connected but platform inactive
            blinkState = !blinkState;
            digitalWrite(LED_STATUS_PIN, blinkState);
        } else {
            // Off when disconnected
            digitalWrite(LED_STATUS_PIN, LOW);
        }
        
        lastUpdate = millis();
    }
}

// ========================================
// Helper Functions
// ========================================

void initializeBuffer() {
    bufferHead = 0;
    bufferTail = 0;
    bufferedCount = 0;
    for (int i = 0; i < MAX_BUFFERED_READINGS; i++) {
        readingBuffer[i].sent = false;
    }
}

void displaySystemCapabilities() {
    Serial.println("🛡️  System Capabilities:");
    Serial.printf("   📦 Buffer Capacity: %d readings\n", MAX_BUFFERED_READINGS);
    Serial.printf("   🔄 Auto-reconnect: Enabled\n");
    Serial.printf("   🔁 Max Retries: 5 attempts\n");
    Serial.printf("   ⏱️  Retry Delay: 2 seconds\n");
    Serial.printf("   💓 Heartbeat: 3 minutes\n");
    Serial.printf("   🚨 Failure Threshold: %d\n", CRITICAL_FAILURE_THRESHOLD);
    Serial.println();
}

void displaySensorReading(const SensorReading& reading) {
    Serial.printf("📊 Sensor Reading [%lu]:\n", reading.timestamp);
    Serial.printf("   🌡️  Temperature: %.2f°C\n", reading.temperature);
    Serial.printf("   💧 Humidity: %.2f%%\n", reading.humidity);
    Serial.printf("   🌱 Soil Moisture: %.2f%%\n", reading.soilMoisture);
}

bool validateSensorReadings(float temp, float humidity, float soil) {
    return (temp > -40 && temp < 85) &&           // Valid temperature range
           (humidity >= 0 && humidity <= 100) &&   // Valid humidity range
           (soil >= 0 && soil <= 100);             // Valid soil moisture range
}

// Sensor reading functions with retry logic
float readTemperatureWithRetry() {
    for (int i = 0; i < 3; i++) {
        float temp = 25.0 + (random(0, 1500) / 100.0); // 25-40°C
        if (temp > -40 && temp < 85) return temp;
        delay(100);
    }
    return 25.0; // Default safe value
}

float readHumidityWithRetry() {
    for (int i = 0; i < 3; i++) {
        float humidity = 60.0 + (random(0, 3500) / 100.0); // 60-95%
        if (humidity >= 0 && humidity <= 100) return humidity;
        delay(100);
    }
    return 70.0; // Default safe value
}

float readSoilMoistureWithRetry() {
    for (int i = 0; i < 3; i++) {
        float moisture = random(0, 10000) / 100.0; // 0-100%
        if (moisture >= 0 && moisture <= 100) return moisture;
        delay(100);
    }
    return 50.0; // Default safe value
}