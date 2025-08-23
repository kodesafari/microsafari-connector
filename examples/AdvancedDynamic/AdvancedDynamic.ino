/*!
 * @file AdvancedDynamic.ino
 * @brief Advanced dynamic JSON structures for specialized agricultural monitoring
 * 
 * This example shows sophisticated use cases for the dynamic JsonObject API:
 * - Time-series data arrays
 * - Nested sensor hierarchies
 * - Custom data validation and quality metrics
 * - Multi-language support (Indonesian + English)
 * - Error handling and data recovery scenarios
 * - Real-world precision agriculture applications
 * 
 * @version 1.0.0
 * @date 2025-08-23
 * @author MicroSafari Team
 */

#include <MicroSafari.h>

// Configuration
const char* WIFI_SSID = "your_wifi_ssid";
const char* WIFI_PASSWORD = "your_wifi_password";
const char* API_KEY = "your_device_api_key";
const char* PLATFORM_URL = "https://your-microsafari-instance.com";
const char* DEVICE_NAME = "ESP32-Advanced-Agri-Monitor";

const unsigned long DATA_INTERVAL = 60000; // 1 minute intervals

MicroSafari microSafari;
int sampleSet = 0;

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);
    
    Serial.println();
    Serial.println("===============================================");
    Serial.println("MicroSafari Advanced Dynamic JSON Demo");
    Serial.println("===============================================");
    Serial.println("Demonstrating sophisticated agricultural IoT:");
    Serial.println("- Time-series measurements");
    Serial.println("- Nested data hierarchies");
    Serial.println("- Multi-language metadata");
    Serial.println("- Data quality assessments");
    Serial.println("- Error recovery scenarios");
    Serial.println();
    
    microSafari.setDebug(true);
    
    if (!microSafari.begin(WIFI_SSID, WIFI_PASSWORD, API_KEY, PLATFORM_URL, DEVICE_NAME)) {
        Serial.println("❌ Setup failed!");
        while (true) delay(1000);
    }
    
    if (!microSafari.connectWiFi()) {
        Serial.println("❌ WiFi failed!");
        while (true) delay(1000);
    }
    
    Serial.println("✅ Advanced monitoring system ready!");
    Serial.println();
}

void loop() {
    microSafari.loop();
    
    static unsigned long lastSample = 0;
    
    if (millis() - lastSample >= DATA_INTERVAL) {
        sampleSet++;
        
        Serial.printf("🔬 Advanced Sample Set %d\n", sampleSet);
        Serial.println("============================");
        
        // Cycle through different advanced scenarios
        switch (sampleSet % 4) {
            case 1:
                sendTimeSeriesData();
                break;
            case 2:
                sendPrecisionAgricultureData();
                break;
            case 3:
                sendQualityControlData();
                break;
            case 0:
                sendMultiLanguageData();
                break;
        }
        
        lastSample = millis();
        Serial.println();
    }
    
    delay(5000);
}

/**
 * @brief Send time-series sensor data with historical context
 */
void sendTimeSeriesData() {
    Serial.println("📊 Advanced Demo 1: Time-Series Data Collection");
    Serial.println("-----------------------------------------------");
    
    DynamicJsonDocument doc(4096);
    JsonObject data = doc.to<JsonObject>();
    
    // Station metadata
    data["station_type"] = "weather_time_series";
    data["measurement_interval_minutes"] = 5;
    data["data_retention_days"] = 30;
    
    // Create time-series array for the last hour (12 x 5-minute intervals)
    JsonArray timeSeries = data.createNestedArray("hourly_measurements");
    
    unsigned long currentTime = millis();
    for (int i = 11; i >= 0; i--) { // Last 12 measurements (1 hour)
        JsonObject measurement = timeSeries.createNestedObject();
        
        // Timestamp (5 minutes ago per iteration)
        measurement["timestamp_ms"] = currentTime - (i * 5 * 60 * 1000);
        measurement["time_offset_minutes"] = i * 5;
        
        // Simulate realistic weather progression
        float baseTemp = 28.0;
        float tempVariation = sin((i * 3.14159) / 6.0) * 3.0; // Gradual change
        measurement["temperature_c"] = baseTemp + tempVariation + (random(-10, 10) / 10.0);
        
        // Humidity inversely related to temperature
        float humidity = 85.0 - (tempVariation * 2) + (random(-30, 30) / 10.0);
        measurement["humidity_percent"] = max(40.0f, min(95.0f, humidity));
        
        // Progressive soil moisture change (irrigation effect)
        float soilBase = 60.0 + (i > 6 ? 15.0 : 0.0); // Irrigation at 30min mark
        measurement["soil_moisture_percent"] = soilBase + (random(-50, 50) / 10.0);
        
        // Wind pattern simulation
        measurement["wind_speed_kmh"] = 8.0 + sin(i * 0.5) * 4.0 + (random(-20, 20) / 10.0);
        measurement["wind_direction_deg"] = (180 + (i * 15) + random(-30, 30)) % 360;
        
        // Data quality indicators
        measurement["sensor_quality"] = (random(85, 100));
        measurement["signal_strength_dbm"] = -45 - random(0, 25);
    }
    
    // Statistical analysis of the time series
    JsonObject analysis = data.createNestedObject("analysis");
    analysis["temperature_trend"] = "warming";
    analysis["moisture_event_detected"] = true;
    analysis["wind_pattern"] = "variable_sw";
    analysis["data_completeness_percent"] = 100.0;
    analysis["anomaly_count"] = 0;
    
    // Predictions based on trends
    JsonObject predictions = data.createNestedObject("predictions");
    predictions["next_irrigation_needed_hours"] = 4.5;
    predictions["weather_stability_score"] = 7.8;
    predictions["plant_stress_risk"] = "low";
    
    Serial.printf("📈 Time-series dataset: %d measurements over 1 hour\n", timeSeries.size());
    Serial.println("Sending comprehensive historical context...");
    
    MicroSafariResponse response = microSafari.sendSensorData(data);
    
    if (response.success) {
        Serial.println("✅ Time-series data transmitted successfully!");
    } else {
        Serial.printf("❌ Failed: %s\n", response.errorMessage.c_str());
    }
}

/**
 * @brief Send precision agriculture monitoring data
 */
void sendPrecisionAgricultureData() {
    Serial.println("📊 Advanced Demo 2: Precision Agriculture Grid");
    Serial.println("----------------------------------------------");
    
    DynamicJsonDocument doc(5120);
    JsonObject data = doc.to<JsonObject>();
    
    // Field information
    data["field_id"] = "FIELD-PRECISION-001";
    data["field_size_hectares"] = 2.5;
    data["crop_variety"] = "Padi Ciherang";
    data["planting_density_per_m2"] = 25;
    
    // GPS coordinates for field boundaries
    JsonArray fieldBoundary = data.createNestedArray("gps_boundary");
    const float coords[][2] = {
        {-6.2088, 106.8456}, {-6.2091, 106.8459}, 
        {-6.2094, 106.8456}, {-6.2091, 106.8453}
    };
    for (int i = 0; i < 4; i++) {
        JsonObject point = fieldBoundary.createNestedObject();
        point["lat"] = coords[i][0];
        point["lon"] = coords[i][1];
    }
    
    // Grid-based monitoring (10m x 10m grid cells)
    JsonArray monitoringGrid = data.createNestedArray("monitoring_grid");
    
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 5; col++) {
            JsonObject cell = monitoringGrid.createNestedObject();
            
            // Grid position
            cell["grid_row"] = row;
            cell["grid_col"] = col;
            cell["grid_id"] = String("R") + row + "C" + col;
            
            // GPS center of cell
            JsonObject gps = cell.createNestedObject("gps_center");
            gps["lat"] = -6.2088 - (row * 0.0001);
            gps["lon"] = 106.8456 + (col * 0.0001);
            
            // Soil conditions vary by position
            JsonObject soil = cell.createNestedObject("soil_conditions");
            soil["moisture_percent"] = 65.0 + (row * 2) - (col * 1.5) + (random(-30, 30) / 10.0);
            soil["temperature_c"] = 26.0 + (random(-20, 20) / 10.0);
            soil["ph_level"] = 6.5 + (row * 0.1) + (random(-20, 20) / 100.0);
            soil["organic_matter_percent"] = 3.2 + (col * 0.15) + (random(-10, 10) / 100.0);
            soil["compaction_psi"] = 15.0 + (row * col * 0.5) + random(-20, 20) / 10.0;
            
            // Plant health varies across field
            JsonObject plants = cell.createNestedObject("plant_health");
            plants["height_cm"] = 75.0 + (col * 2) + random(-50, 50) / 10.0;
            plants["tiller_count"] = 8 + (row + col) / 2 + random(-2, 3);
            plants["leaf_greenness_spad"] = 38.0 + (col * 1.5) + (random(-30, 30) / 10.0);
            plants["disease_score"] = random(1, 6); // 1=healthy, 5=severe
            plants["pest_damage_percent"] = (random(0, 150) / 10.0);
            
            // Yield prediction for this cell
            JsonObject yield = cell.createNestedObject("yield_prediction");
            yield["estimated_kg_per_cell"] = 12.5 + (col * 0.8) - (row * 0.3) + (random(-20, 30) / 10.0);
            yield["quality_grade"] = yield["estimated_kg_per_cell"].as<float>() > 13.0 ? "A" : 
                                    yield["estimated_kg_per_cell"].as<float>() > 11.0 ? "B" : "C";
            yield["maturity_days_remaining"] = 45 + random(-7, 7);
            
            // Microclimate data
            JsonObject microclimate = cell.createNestedObject("microclimate");
            microclimate["air_temp_c"] = 29.0 + (row * 0.3) + (random(-20, 20) / 10.0);
            microclimate["humidity_percent"] = 78.0 - (col * 1.2) + random(-50, 50) / 10.0;
            microclimate["light_intensity_lux"] = 45000 + (col * 2000) + random(-5000, 5000);
        }
    }
    
    // Field-wide analysis
    JsonObject fieldAnalysis = data.createNestedObject("field_analysis");
    fieldAnalysis["total_grid_cells"] = monitoringGrid.size();
    fieldAnalysis["uniform_growth_percent"] = 78.5;
    fieldAnalysis["irrigation_efficiency_score"] = 8.2;
    fieldAnalysis["pest_hotspot_detected"] = false;
    fieldAnalysis["yield_variability_coefficient"] = 0.15;
    
    // Recommended actions
    JsonArray recommendations = data.createNestedArray("precision_recommendations");
    JsonObject rec1 = recommendations.createNestedObject();
    rec1["action"] = "variable_rate_fertilization";
    rec1["zones"] = "R3C4,R4C4,R4C3";
    rec1["priority"] = "medium";
    rec1["estimated_benefit"] = "8% yield increase";
    
    JsonObject rec2 = recommendations.createNestedObject();
    rec2["action"] = "targeted_irrigation";
    rec2["zones"] = "R0C0,R0C1,R1C0";
    rec2["priority"] = "high";
    rec2["estimated_benefit"] = "water savings 15%";
    
    Serial.printf("🌾 Precision agriculture grid: %d monitoring points\n", monitoringGrid.size());
    Serial.println("Sending detailed spatial analysis...");
    
    MicroSafariResponse response = microSafari.sendSensorData(data);
    
    if (response.success) {
        Serial.println("✅ Precision agriculture data sent successfully!");
    } else {
        Serial.printf("❌ Failed: %s\n", response.errorMessage.c_str());
    }
}

/**
 * @brief Send data quality control and sensor calibration data
 */
void sendQualityControlData() {
    Serial.println("📊 Advanced Demo 3: Quality Control & Calibration");
    Serial.println("-------------------------------------------------");
    
    DynamicJsonDocument doc(3072);
    JsonObject data = doc.to<JsonObject>();
    
    data["measurement_type"] = "quality_assurance";
    data["qc_session_id"] = "QC-" + String(millis());
    data["technician_id"] = "TECH-001";
    
    // Sensor validation tests
    JsonArray sensorTests = data.createNestedArray("sensor_validations");
    
    const char* sensorTypes[] = {"temperature", "humidity", "soil_moisture", "ph_meter", "light_sensor"};
    const char* testMethods[] = {"reference_comparison", "drift_analysis", "linearity_check", "stability_test", "cross_validation"};
    
    for (int i = 0; i < 5; i++) {
        JsonObject test = sensorTests.createNestedObject();
        test["sensor_type"] = sensorTypes[i];
        test["sensor_id"] = String("SN-") + (10000 + i + random(0, 1000));
        test["test_method"] = testMethods[i];
        test["calibration_date"] = "2025-08-01";
        test["last_validation"] = "2025-08-20";
        
        // Reference vs measured values
        JsonObject comparison = test.createNestedObject("reference_comparison");
        float referenceValue = 25.0 + (i * 10);
        float measuredValue = referenceValue + (random(-100, 100) / 100.0); // Small error
        comparison["reference_value"] = referenceValue;
        comparison["measured_value"] = measuredValue;
        comparison["absolute_error"] = abs(measuredValue - referenceValue);
        comparison["relative_error_percent"] = (abs(measuredValue - referenceValue) / referenceValue) * 100;
        
        // Quality metrics
        JsonObject quality = test.createNestedObject("quality_metrics");
        quality["accuracy_score"] = max(70.0, 100.0 - (comparison["relative_error_percent"].as<float>() * 10));
        quality["precision_std_dev"] = 0.1 + (random(0, 50) / 100.0);
        quality["drift_per_day"] = (random(-20, 20) / 1000.0);
        quality["response_time_seconds"] = 1.2 + (random(0, 80) / 100.0);
        quality["temperature_coefficient"] = 0.05 + (random(-10, 10) / 1000.0);
        
        // Pass/fail determination
        test["validation_result"] = quality["accuracy_score"].as<float>() > 85.0 ? "PASS" : 
                                   quality["accuracy_score"].as<float>() > 75.0 ? "MARGINAL" : "FAIL";
        test["needs_recalibration"] = test["validation_result"] != "PASS";
        test["next_validation_due"] = "2025-11-01";
    }
    
    // Environmental conditions during testing
    JsonObject testConditions = data.createNestedObject("test_environment");
    testConditions["lab_temperature_c"] = 23.5 + (random(-10, 10) / 10.0);
    testConditions["lab_humidity_percent"] = 45.0 + random(-5, 5);
    testConditions["atmospheric_pressure_hpa"] = 1013.25 + (random(-20, 20) / 10.0);
    testConditions["test_duration_minutes"] = 45 + random(0, 30);
    testConditions["power_supply_voltage"] = 5.02 + (random(-20, 20) / 1000.0);
    
    // Statistical analysis of all sensors
    JsonObject statistics = data.createNestedObject("fleet_statistics");
    int passCount = 0, marginalCount = 0, failCount = 0;
    
    for (JsonObject test : sensorTests) {
        if (test["validation_result"] == "PASS") passCount++;
        else if (test["validation_result"] == "MARGINAL") marginalCount++;
        else failCount++;
    }
    
    statistics["total_sensors_tested"] = sensorTests.size();
    statistics["pass_rate_percent"] = (passCount * 100.0) / sensorTests.size();
    statistics["marginal_rate_percent"] = (marginalCount * 100.0) / sensorTests.size();
    statistics["failure_rate_percent"] = (failCount * 100.0) / sensorTests.size();
    statistics["fleet_health_score"] = statistics["pass_rate_percent"];
    
    // Maintenance recommendations
    JsonArray maintenance = data.createNestedArray("maintenance_actions");
    
    if (failCount > 0) {
        JsonObject action1 = maintenance.createNestedObject();
        action1["priority"] = "urgent";
        action1["action"] = "replace_failed_sensors";
        action1["sensor_count"] = failCount;
        action1["estimated_cost_usd"] = failCount * 45;
    }
    
    if (marginalCount > 0) {
        JsonObject action2 = maintenance.createNestedObject();
        action2["priority"] = "medium";
        action2["action"] = "recalibrate_marginal_sensors";
        action2["sensor_count"] = marginalCount;
        action2["estimated_time_hours"] = marginalCount * 0.5;
    }
    
    JsonObject action3 = maintenance.createNestedObject();
    action3["priority"] = "routine";
    action3["action"] = "next_scheduled_validation";
    action3["date"] = "2025-11-01";
    action3["all_sensors"] = true;
    
    Serial.printf("🔬 Quality control report: %d sensors tested\n", sensorTests.size());
    Serial.printf("✅ Pass rate: %.1f%%, Fleet health: %.1f/10\n", 
                  statistics["pass_rate_percent"].as<float>(),
                  statistics["fleet_health_score"].as<float>() / 10);
    
    MicroSafariResponse response = microSafari.sendSensorData(data);
    
    if (response.success) {
        Serial.println("✅ Quality control data transmitted!");
    } else {
        Serial.printf("❌ Failed: %s\n", response.errorMessage.c_str());
    }
}

/**
 * @brief Send multi-language metadata for Indonesian farmers
 */
void sendMultiLanguageData() {
    Serial.println("📊 Advanced Demo 4: Multi-Language Support");
    Serial.println("------------------------------------------");
    
    DynamicJsonDocument doc(2560);
    JsonObject data = doc.to<JsonObject>();
    
    data["report_type"] = "bilingual_farm_report";
    data["primary_language"] = "id"; // Indonesian
    data["secondary_language"] = "en"; // English
    
    // Farm information in both languages
    JsonObject farmInfo = data.createNestedObject("farm_information");
    
    JsonObject farmInfoId = farmInfo.createNestedObject("id");
    farmInfoId["nama_petani"] = "Budi Santoso";
    farmInfoId["lokasi_desa"] = "Desa Makmur, Kabupaten Bogor";
    farmInfoId["jenis_tanaman"] = "Padi Sawah";
    farmInfoId["luas_lahan_ha"] = 1.2;
    farmInfoId["musim_tanam"] = "Musim Hujan 2025";
    farmInfoId["tanggal_tanam"] = "15 Juli 2025";
    
    JsonObject farmInfoEn = farmInfo.createNestedObject("en");
    farmInfoEn["farmer_name"] = "Budi Santoso";
    farmInfoEn["village_location"] = "Makmur Village, Bogor District";
    farmInfoEn["crop_type"] = "Rice Paddy";
    farmInfoEn["land_area_ha"] = 1.2;
    farmInfoEn["growing_season"] = "Wet Season 2025";
    farmInfoEn["planting_date"] = "July 15, 2025";
    
    // Current conditions with bilingual descriptions
    JsonObject conditions = data.createNestedObject("current_conditions");
    
    // Sensor readings (universal numbers)
    conditions["air_temperature_celsius"] = 29.5 + (random(-30, 30) / 10.0);
    conditions["soil_moisture_percent"] = 72.0 + random(-15, 15);
    conditions["rainfall_mm_today"] = random(0, 25) / 10.0;
    conditions["soil_ph"] = 6.8 + (random(-30, 30) / 100.0);
    conditions["plant_height_cm"] = 65.0 + random(-10, 20);
    
    // Status descriptions in both languages
    JsonObject statusId = conditions.createNestedObject("status_id");
    statusId["kondisi_cuaca"] = "Mendung dengan kemungkinan hujan";
    statusId["kondisi_tanah"] = "Lembab dan subur";
    statusId["kesehatan_tanaman"] = "Baik, pertumbuhan normal";
    statusId["kebutuhan_air"] = "Cukup untuk 3 hari ke depan";
    statusId["tahap_pertumbuhan"] = "Fase berbunga awal";
    statusId["rekomendasi"] = "Pantau hama dan penyakit";
    
    JsonObject statusEn = conditions.createNestedObject("status_en");
    statusEn["weather_condition"] = "Cloudy with chance of rain";
    statusEn["soil_condition"] = "Moist and fertile";
    statusEn["plant_health"] = "Good, normal growth";
    statusEn["water_needs"] = "Sufficient for next 3 days";
    statusEn["growth_stage"] = "Early flowering phase";
    statusEn["recommendation"] = "Monitor for pests and diseases";
    
    // Weekly summary with cultural context
    JsonObject weeklySummary = data.createNestedObject("weekly_summary");
    
    JsonObject summaryId = weeklySummary.createNestedObject("id");
    summaryId["periode"] = "Minggu ke-6 setelah tanam";
    summaryId["curah_hujan_total_mm"] = 85.5;
    summaryId["hari_hujan"] = 4;
    summaryId["suhu_rata_rata_c"] = 28.2;
    summaryId["pertumbuhan"] = "Sesuai dengan kalender tanam";
    summaryId["kegiatan_minggu_ini"] = "Penyiangan gulma, pengecekan hama";
    summaryId["kegiatan_minggu_depan"] = "Pemupukan susulan, pengaturan air";
    
    JsonObject summaryEn = weeklySummary.createNestedObject("en");
    summaryEn["period"] = "Week 6 after planting";
    summaryEn["total_rainfall_mm"] = 85.5;
    summaryEn["rainy_days"] = 4;
    summaryEn["average_temperature_c"] = 28.2;
    summaryEn["growth_status"] = "On schedule with planting calendar";
    summaryEn["this_week_activities"] = "Weeding, pest inspection";
    summaryEn["next_week_activities"] = "Fertilizer application, water management";
    
    // Agricultural advice with local wisdom
    JsonArray adviceId = data.createNestedArray("saran_petani_id");
    adviceId.add("Musim hujan ini cocok untuk pertumbuhan padi");
    adviceId.add("Perhatikan drainase agar tidak tergenang");
    adviceId.add("Waktu yang tepat untuk pemupukan NPK");
    adviceId.add("Pantau gejala hawar daun bakteri");
    
    JsonArray adviceEn = data.createNestedArray("farming_advice_en");
    adviceEn.add("This rainy season is ideal for rice growth");
    adviceEn.add("Monitor drainage to prevent waterlogging");
    adviceEn.add("Good timing for NPK fertilizer application");
    adviceEn.add("Watch for bacterial leaf blight symptoms");
    
    // Economic information
    JsonObject economics = data.createNestedObject("economic_indicators");
    economics["local_rice_price_idr_per_kg"] = 8500 + random(-500, 1000);
    economics["fertilizer_price_idr_per_kg"] = 12000 + random(-1000, 2000);
    economics["estimated_yield_kg"] = 4200 + random(-800, 1200);
    economics["estimated_revenue_idr"] = economics["estimated_yield_kg"].as<int>() * economics["local_rice_price_idr_per_kg"].as<int>();
    
    JsonObject economicsId = economics.createNestedObject("info_ekonomi_id");
    economicsId["prediksi_untung_rugi"] = "Menguntungkan jika cuaca normal";
    economicsId["tips_pemasaran"] = "Jual bertahap untuk harga terbaik";
    
    JsonObject economicsEn = economics.createNestedObject("economic_info_en");
    economicsEn["profit_prediction"] = "Profitable if weather remains normal";
    economicsEn["marketing_tip"] = "Sell gradually for best prices";
    
    Serial.println("🌏 Sending bilingual agricultural report (Indonesian/English)");
    Serial.printf("👨‍🌾 Petani: %s, Luas: %.1f ha\n", 
                  farmInfoId["nama_petani"].as<const char*>(),
                  farmInfoId["luas_lahan_ha"].as<float>());
    Serial.printf("🌾 Kondisi: %s\n", statusId["kesehatan_tanaman"].as<const char*>());
    
    MicroSafariResponse response = microSafari.sendSensorData(data);
    
    if (response.success) {
        Serial.println("✅ Laporan dwi-bahasa berhasil dikirim!");
        Serial.println("✅ Bilingual report transmitted successfully!");
    } else {
        Serial.printf("❌ Gagal: %s\n", response.errorMessage.c_str());
    }
}