/*!
 * @file DynamicSensorData.ino
 * @brief Advanced dynamic sensor data example for MicroSafari ESP32 Library
 * 
 * This example demonstrates the dynamic JsonObject capabilities:
 * - Sending custom sensor data with any structure
 * - Nested JSON objects and arrays
 * - Mixed data types (floats, strings, booleans, arrays)
 * - Real-world agricultural monitoring scenarios
 * - Comparing dynamic vs. fixed-parameter approaches
 * 
 * @version 1.0.0
 * @date 2025-08-23
 * @author MicroSafari Team
 */

#include <MicroSafari.h>

// Configuration - CHANGE THESE TO YOUR VALUES
const char* WIFI_SSID = "your_wifi_ssid";
const char* WIFI_PASSWORD = "your_wifi_password";
const char* API_KEY = "your_device_api_key";
const char* PLATFORM_URL = "https://your-microsafari-instance.com";
const char* DEVICE_NAME = "ESP32-Dynamic-Farm-Station";

// Data transmission interval
const unsigned long SEND_INTERVAL = 45000; // 45 seconds

// Create MicroSafari instance
MicroSafari microSafari;

// Demo cycle counter
int demoCycle = 0;

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);
    
    Serial.println();
    Serial.println("=============================================");
    Serial.println("MicroSafari Dynamic Sensor Data Demo");
    Serial.println("=============================================");
    Serial.println("This demo showcases flexible JSON data transmission:");
    Serial.println("- Custom sensor types");
    Serial.println("- Nested data structures");
    Serial.println("- Agricultural monitoring examples");
    Serial.println("- Mixed data types support");
    Serial.println();
    
    // Enable debug for detailed output
    microSafari.setDebug(true);
    
    // Initialize library
    if (!microSafari.begin(WIFI_SSID, WIFI_PASSWORD, API_KEY, PLATFORM_URL, DEVICE_NAME)) {
        Serial.println("❌ Initialization failed!");
        while (true) delay(1000);
    }
    
    // Connect to WiFi
    if (!microSafari.connectWiFi()) {
        Serial.println("❌ WiFi connection failed!");
        while (true) delay(1000);
    }
    
    // Test platform connection
    if (microSafari.testConnection()) {
        Serial.println("✅ All systems ready! Starting dynamic data demo...");
        Serial.println();
    } else {
        Serial.println("⚠️ Platform test failed, but continuing demo...");
    }
}

void loop() {
    microSafari.loop();
    
    static unsigned long lastSend = 0;
    
    if (millis() - lastSend >= SEND_INTERVAL) {
        demoCycle++;
        Serial.printf("🔄 Demo Cycle %d - Demonstrating Dynamic Capabilities\n", demoCycle);
        Serial.println("=================================================");
        
        // Rotate through different demo scenarios
        switch (demoCycle % 5) {
            case 1:
                demonstrateBasicDynamic();
                break;
            case 2:
                demonstrateAgriculturalStation();
                break;
            case 3:
                demonstrateWeatherStation();
                break;
            case 4:
                demonstrateComplexFarmData();
                break;
            case 0:
                demonstrateComparisonExample();
                break;
        }
        
        lastSend = millis();
        Serial.println();
    }
    
    delay(2000);
}

/**
 * @brief Demonstrate basic dynamic data structure
 */
void demonstrateBasicDynamic() {
    Serial.println("📡 Demo 1: Basic Dynamic Data Structure");
    Serial.println("---------------------------------------");
    
    // Create dynamic JSON payload
    DynamicJsonDocument doc(1024);
    JsonObject sensorData = doc.to<JsonObject>();
    
    // Custom sensor readings - not limited to temperature/humidity
    sensorData["ph_level"] = 6.8 + (random(-20, 20) / 100.0);
    sensorData["nitrogen_ppm"] = 150.0 + random(-30, 30);
    sensorData["phosphorus_ppm"] = 45.0 + random(-10, 10);
    sensorData["potassium_ppm"] = 200.0 + random(-40, 40);
    sensorData["conductivity_us"] = 800.0 + random(-100, 100);
    sensorData["water_depth_cm"] = 25.0 + random(-5, 5);
    
    // Add metadata
    sensorData["sensor_type"] = "soil_chemistry";
    sensorData["location"] = "Field-A-North";
    sensorData["measurement_quality"] = "high";
    sensorData["battery_voltage"] = 3.7 + (random(-30, 30) / 100.0);
    
    Serial.println("📊 Sending soil chemistry data:");
    serializeJsonPretty(sensorData, Serial);
    Serial.println();
    
    MicroSafariResponse response = microSafari.sendSensorData(sensorData);
    
    if (response.success) {
        Serial.println("✅ Soil chemistry data sent successfully!");
    } else {
        Serial.printf("❌ Failed: %s\n", response.errorMessage.c_str());
    }
}

/**
 * @brief Demonstrate agricultural monitoring station
 */
void demonstrateAgriculturalStation() {
    Serial.println("📡 Demo 2: Agricultural Monitoring Station");
    Serial.println("------------------------------------------");
    
    DynamicJsonDocument doc(2048);
    JsonObject data = doc.to<JsonObject>();
    
    // Environmental conditions
    JsonObject environment = data.createNestedObject("environment");
    environment["air_temp_c"] = 28.5 + (random(-50, 50) / 10.0);
    environment["air_humidity_percent"] = 75.0 + random(-20, 20);
    environment["wind_speed_kmh"] = 12.0 + random(0, 10);
    environment["wind_direction_deg"] = random(0, 360);
    environment["rainfall_mm"] = random(0, 5) / 10.0;
    environment["uv_index"] = random(1, 12);
    
    // Soil monitoring array for multiple depths
    JsonArray soilLayers = data.createNestedArray("soil_layers");
    const char* depths[] = {"10cm", "25cm", "50cm"};
    for (int i = 0; i < 3; i++) {
        JsonObject layer = soilLayers.createNestedObject();
        layer["depth"] = depths[i];
        layer["temperature_c"] = 24.0 + i + (random(-20, 20) / 10.0);
        layer["moisture_percent"] = 65.0 - (i * 5) + random(-10, 10);
        layer["ph"] = 6.5 + (random(-30, 30) / 100.0);
        layer["salinity_ppm"] = 200 + (i * 50) + random(-50, 50);
    }
    
    // Plant growth monitoring
    JsonObject growth = data.createNestedObject("plant_monitoring");
    growth["growth_stage"] = "flowering";
    growth["plant_height_cm"] = 85.0 + random(-5, 15);
    growth["leaf_temperature_c"] = 26.0 + (random(-30, 30) / 10.0);
    growth["photosynthesis_rate"] = 15.5 + (random(-20, 20) / 10.0);
    growth["stress_level"] = random(1, 6); // 1=low, 5=high
    
    // System status
    JsonObject system = data.createNestedObject("system_status");
    system["station_id"] = "AGR-001";
    system["firmware_version"] = "1.2.3";
    system["uptime_hours"] = millis() / 3600000;
    system["memory_free_kb"] = ESP.getFreeHeap() / 1024;
    system["wifi_signal_dbm"] = microSafari.getWiFiSignalStrength();
    
    Serial.println("📊 Sending comprehensive agricultural data:");
    serializeJsonPretty(data, Serial);
    Serial.println();
    
    MicroSafariResponse response = microSafari.sendSensorData(data);
    
    if (response.success) {
        Serial.println("✅ Agricultural station data sent successfully!");
    } else {
        Serial.printf("❌ Failed: %s\n", response.errorMessage.c_str());
    }
}

/**
 * @brief Demonstrate weather station data
 */
void demonstrateWeatherStation() {
    Serial.println("📡 Demo 3: Weather Station Data");
    Serial.println("-------------------------------");
    
    DynamicJsonDocument doc(1536);
    JsonObject data = doc.to<JsonObject>();
    
    // Current conditions
    JsonObject current = data.createNestedObject("current_conditions");
    current["timestamp"] = millis();
    current["temperature_c"] = 29.5 + (random(-40, 40) / 10.0);
    current["feels_like_c"] = current["temperature_c"].as<float>() + 2.0;
    current["humidity_percent"] = 78.0 + random(-15, 15);
    current["pressure_hpa"] = 1013.25 + (random(-50, 50) / 10.0);
    current["dew_point_c"] = 24.0 + (random(-30, 30) / 10.0);
    current["visibility_km"] = 10.0 + random(-30, 20) / 10.0;
    
    // Wind measurements
    JsonObject wind = current.createNestedObject("wind");
    wind["speed_kmh"] = 8.5 + (random(0, 150) / 10.0);
    wind["direction_deg"] = random(0, 360);
    wind["gust_kmh"] = wind["speed_kmh"].as<float>() + random(0, 50) / 10.0;
    
    // Precipitation
    JsonObject precip = current.createNestedObject("precipitation");
    precip["rain_rate_mmh"] = random(0, 20) / 10.0;
    precip["daily_total_mm"] = random(0, 150) / 10.0;
    precip["intensity"] = precip["rain_rate_mmh"].as<float>() > 5.0 ? "heavy" : 
                          precip["rain_rate_mmh"].as<float>() > 1.0 ? "moderate" : "light";
    
    // Solar radiation
    JsonObject solar = current.createNestedObject("solar");
    solar["irradiance_wm2"] = random(200, 1200);
    solar["uv_index"] = random(1, 12);
    solar["daylight_hours"] = 12.5 + (random(-30, 30) / 100.0);
    
    // Air quality (if sensors available)
    JsonObject air = data.createNestedObject("air_quality");
    air["pm25_ugm3"] = random(5, 50);
    air["pm10_ugm3"] = air["pm25_ugm3"].as<int>() + random(10, 30);
    air["co2_ppm"] = 400 + random(0, 200);
    air["aqi_rating"] = air["pm25_ugm3"].as<int>() < 15 ? "good" : 
                       air["pm25_ugm3"].as<int>() < 35 ? "moderate" : "unhealthy";
    
    Serial.println("📊 Sending weather station data:");
    serializeJsonPretty(data, Serial);
    Serial.println();
    
    MicroSafariResponse response = microSafari.sendSensorData(data);
    
    if (response.success) {
        Serial.println("✅ Weather station data sent successfully!");
    } else {
        Serial.printf("❌ Failed: %s\n", response.errorMessage.c_str());
    }
}

/**
 * @brief Demonstrate complex multi-crop farm monitoring
 */
void demonstrateComplexFarmData() {
    Serial.println("📡 Demo 4: Complex Multi-Crop Farm Monitoring");
    Serial.println("---------------------------------------------");
    
    DynamicJsonDocument doc(3072);
    JsonObject data = doc.to<JsonObject>();
    
    // Farm metadata
    data["farm_id"] = "FARM-ID-12345";
    data["region"] = "Central Java";
    data["season"] = "wet_season";
    data["measurement_time"] = millis();
    
    // Multiple crop monitoring
    JsonArray crops = data.createNestedArray("crop_zones");
    
    const char* cropTypes[] = {"rice", "corn", "soybeans", "chili"};
    const char* zoneNames[] = {"Zone-A", "Zone-B", "Zone-C", "Zone-D"};
    
    for (int i = 0; i < 4; i++) {
        JsonObject crop = crops.createNestedObject();
        crop["zone"] = zoneNames[i];
        crop["crop_type"] = cropTypes[i];
        crop["planting_date"] = "2025-07-01"; // Fixed planting date
        crop["growth_days"] = 53 + random(-5, 15);
        
        // Zone-specific environmental data
        JsonObject environment = crop.createNestedObject("environment");
        environment["soil_temp_c"] = 26.0 + i + (random(-20, 20) / 10.0);
        environment["soil_moisture_percent"] = 70.0 - (i * 3) + random(-10, 10);
        environment["canopy_temp_c"] = 28.0 + i + (random(-25, 25) / 10.0);
        
        // Irrigation system status
        JsonObject irrigation = crop.createNestedObject("irrigation");
        irrigation["active"] = random(0, 2) == 1;
        irrigation["water_flow_lpm"] = irrigation["active"].as<bool>() ? 25.0 + random(0, 100) / 10.0 : 0.0;
        irrigation["pressure_bar"] = irrigation["active"].as<bool>() ? 2.1 + random(-3, 3) / 10.0 : 0.0;
        irrigation["daily_usage_liters"] = 1500 + random(-500, 1000);
        
        // Pest and disease monitoring
        JsonObject health = crop.createNestedObject("plant_health");
        health["pest_trap_count"] = random(0, 25);
        health["disease_spots_percent"] = random(0, 15) / 10.0;
        health["chlorophyll_index"] = 35.0 + random(-50, 50) / 10.0;
        health["stress_indicators"] = health["disease_spots_percent"].as<float>() > 0.5 ? 
                                     (JsonArray)(health.createNestedArray("stress_indicators")).add("disease") :
                                     JsonArray();
        
        // Yield prediction
        JsonObject yield = crop.createNestedObject("yield_prediction");
        yield["expected_kg_per_hectare"] = 4500 + random(-1000, 1500);
        yield["confidence_percent"] = 85.0 + random(-15, 10);
        yield["harvest_date_estimate"] = "2025-09-15";
    }
    
    // Farm-wide systems
    JsonObject systems = data.createNestedObject("farm_systems");
    systems["solar_panel_kw"] = 15.5;
    systems["battery_charge_percent"] = 78.0 + random(-20, 20);
    systems["water_tank_liters"] = 5000.0 + random(-1000, 500);
    systems["fertilizer_stock_kg"] = 250.0 + random(-50, 100);
    
    Serial.println("📊 Sending complex multi-crop farm data:");
    Serial.println("(Note: Large payload - showing key sections)");
    
    // Show summary instead of full JSON due to size
    Serial.printf("Farm ID: %s, Crops: %d zones\n", 
                  data["farm_id"].as<const char*>(), crops.size());
    Serial.printf("Battery: %.1f%%, Water Tank: %.0f L\n",
                  systems["battery_charge_percent"].as<float>(),
                  systems["water_tank_liters"].as<float>());
    
    MicroSafariResponse response = microSafari.sendSensorData(data);
    
    if (response.success) {
        Serial.println("✅ Complex farm data sent successfully!");
    } else {
        Serial.printf("❌ Failed: %s\n", response.errorMessage.c_str());
        if (response.httpCode == 413) {
            Serial.println("💡 Payload too large - consider breaking into smaller chunks");
        }
    }
}

/**
 * @brief Compare dynamic vs fixed-parameter approaches
 */
void demonstrateComparisonExample() {
    Serial.println("📡 Demo 5: Comparison - Dynamic vs Fixed Parameters");
    Serial.println("--------------------------------------------------");
    
    // Generate sensor readings
    float temp = 27.5 + (random(-40, 40) / 10.0);
    float humidity = 72.0 + random(-15, 15);
    float soilMoisture = 68.0 + random(-20, 20);
    float light = 850.0 + random(-200, 200);
    
    Serial.println("📊 Sample Data:");
    Serial.printf("   Temperature: %.1f°C\n", temp);
    Serial.printf("   Humidity: %.1f%%\n", humidity);
    Serial.printf("   Soil Moisture: %.1f%%\n", soilMoisture);
    Serial.printf("   Light: %.1f lux\n", light);
    Serial.println();
    
    // Method 1: Traditional fixed parameters
    Serial.println("🔹 Method 1: Fixed Parameters (backward compatibility)");
    unsigned long startTime = millis();
    MicroSafariResponse response1 = microSafari.sendSensorData(temp, humidity, soilMoisture, light);
    unsigned long duration1 = millis() - startTime;
    
    if (response1.success) {
        Serial.printf("   ✅ Success! Duration: %lu ms\n", duration1);
    } else {
        Serial.printf("   ❌ Failed: %s\n", response1.errorMessage.c_str());
    }
    
    delay(3000); // Brief pause between methods
    
    // Method 2: Dynamic JsonObject with same data + extras
    Serial.println("🔹 Method 2: Dynamic JsonObject (enhanced flexibility)");
    DynamicJsonDocument doc(1024);
    JsonObject dynamicData = doc.to<JsonObject>();
    
    // Same basic data
    dynamicData["temperature"] = temp;
    dynamicData["humidity"] = humidity;
    dynamicData["soil_moisture"] = soilMoisture;
    dynamicData["light_level"] = light;
    
    // Additional data possible with dynamic approach
    dynamicData["soil_ph"] = 6.7 + (random(-20, 20) / 100.0);
    dynamicData["wind_speed"] = 5.5 + (random(0, 50) / 10.0);
    dynamicData["co2_level"] = 420 + random(-30, 80);
    dynamicData["device_location"] = "Greenhouse-2";
    dynamicData["measurement_confidence"] = "high";
    dynamicData["calibration_date"] = "2025-08-01";
    
    startTime = millis();
    MicroSafariResponse response2 = microSafari.sendSensorData(dynamicData);
    unsigned long duration2 = millis() - startTime;
    
    if (response2.success) {
        Serial.printf("   ✅ Success! Duration: %lu ms\n", duration2);
    } else {
        Serial.printf("   ❌ Failed: %s\n", response2.errorMessage.c_str());
    }
    
    Serial.println();
    Serial.println("📈 Comparison Summary:");
    Serial.printf("   Fixed Parameters: %d data points, %lu ms\n", 4, duration1);
    Serial.printf("   Dynamic JSON: %d data points, %lu ms\n", 10, duration2);
    Serial.println("   💡 Dynamic approach allows unlimited sensor types!");
}