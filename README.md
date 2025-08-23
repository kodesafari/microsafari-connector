# MicroSafari ESP32 Arduino Library

![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)
![Platform](https://img.shields.io/badge/platform-ESP32-orange.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)

Arduino library for ESP32 devices to connect to the MicroSafari IoT SaaS platform for Indonesian millennial farmers.

## 🌾 Overview

The MicroSafari library provides a simple and robust way to connect ESP32-based agricultural IoT devices to the MicroSafari platform. It handles WiFi connectivity, HTTP communication, and data transmission with built-in error handling and automatic reconnection.

## 🚀 Features

- **WiFi Management**: Automatic connection and reconnection handling
- **HTTP Client**: Secure communication with MicroSafari platform
- **Dynamic Data Transmission**: Send any sensor data structure using flexible JSON objects
- **Backward Compatibility**: Traditional fixed-parameter methods still supported
- **Error Handling**: Comprehensive error detection and reporting
- **Debug Support**: Built-in debugging capabilities
- **Status Monitoring**: Real-time connection and system status
- **Indonesian Optimized**: Designed for Indonesian agricultural environments

## 📦 Installation

### Arduino IDE

1. Download the latest release from the releases page
2. In Arduino IDE, go to **Sketch** → **Include Library** → **Add .ZIP Library**
3. Select the downloaded ZIP file
4. The library will be installed and ready to use

### PlatformIO

Add to your `platformio.ini`:

```ini
lib_deps = 
    MicroSafari
    bblanchon/ArduinoJson@^6.19.4
```

## 🔧 Dependencies

- **WiFi** (ESP32 Core)
- **HTTPClient** (ESP32 Core)
- **ArduinoJson** (^6.19.4)
- **WiFiClientSecure** (ESP32 Core)

## 🎯 Quick Start

### Basic Usage

```cpp
#include <MicroSafari.h>

// WiFi credentials
const char* WIFI_SSID = "your_wifi_ssid";
const char* WIFI_PASSWORD = "your_wifi_password";

// MicroSafari configuration
const char* API_KEY = "your_device_api_key";
const char* PLATFORM_URL = "https://your-microsafari-instance.com";

MicroSafari microSafari;

void setup() {
    Serial.begin(115200);
    
    // Initialize library
    microSafari.begin(WIFI_SSID, WIFI_PASSWORD, API_KEY, PLATFORM_URL);
    
    // Connect to WiFi
    microSafari.connectWiFi();
}

void loop() {
    microSafari.loop(); // Handle automatic reconnection
    
    // Send sensor data every 30 seconds
    static unsigned long lastReading = 0;
    if (millis() - lastReading >= 30000) {
        float temp = 28.5;
        float humidity = 75.0;
        
        microSafari.sendSensorData(temp, humidity);
        lastReading = millis();
    }
    
    delay(1000);
}
```

### Dynamic JSON Usage (Recommended)

The MicroSafari library supports completely flexible sensor data transmission using JsonObjects. You can send ANY sensor type with custom structures:

```cpp
// Create dynamic JSON payload with any sensor data
DynamicJsonDocument doc(1024);
JsonObject data = doc.to<JsonObject>();

// Standard sensors
data["temperature"] = 28.5;
data["humidity"] = 75.0;

// Custom agricultural sensors
data["soil_ph"] = 6.8;
data["nitrogen_ppm"] = 150.0;
data["phosphorus_ppm"] = 45.0;
data["co2_level"] = 420;
data["water_depth_cm"] = 25.0;

// Nested location data
JsonObject location = data.createNestedObject("location");
location["latitude"] = -6.2088;
location["longitude"] = 106.8456;
location["field_name"] = "Sawah Utara";

// Sensor array example
JsonArray soilLayers = data.createNestedArray("soil_layers");
for (int depth = 10; depth <= 50; depth += 20) {
    JsonObject layer = soilLayers.createNestedObject();
    layer["depth_cm"] = depth;
    layer["temperature_c"] = 26.0 + (depth * 0.1);
    layer["moisture_percent"] = 65.0 - (depth * 0.5);
}

// Metadata
data["measurement_time"] = millis();
data["device_location"] = "Greenhouse-A";
data["sensor_quality"] = "high";

// Send to platform
MicroSafariResponse response = microSafari.sendSensorData(data);

if (response.success) {
    Serial.println("Dynamic sensor data sent successfully!");
} else {
    Serial.println("Failed: " + response.errorMessage);
}
```

### Backward Compatibility

The traditional fixed-parameter method is still fully supported:

```cpp
// Legacy method - still works perfectly
float temp = 28.5;
float humidity = 75.0;
float soilMoisture = 60.3;
float lightLevel = 850.0;

MicroSafariResponse response = microSafari.sendSensorData(temp, humidity, soilMoisture, lightLevel);
```

## 📚 API Reference

### Class: MicroSafari

#### Initialization

```cpp
bool begin(const String& ssid, 
           const String& password, 
           const String& apiKey,
           const String& platformUrl = "https://microsafari.com",
           const String& deviceName = "ESP32-Device");
```

Initialize the library with connection parameters.

#### WiFi Management

```cpp
bool connectWiFi(unsigned long timeout = 30000);
bool isWiFiConnected();
void disconnect();
```

#### Data Transmission

```cpp
// Dynamic JSON method (recommended) - supports any sensor data structure
MicroSafariResponse sendSensorData(const JsonObject& sensorData);

// Fixed parameter method (backward compatibility)
MicroSafariResponse sendSensorData(float temperature, 
                                   float humidity, 
                                   float soilMoisture = -1, 
                                   float lightLevel = -1);

// Raw JSON string method (advanced usage)
MicroSafariResponse sendRawData(const String& jsonPayload);
```

#### Status and Monitoring

```cpp
MicroSafariStatus getStatus();
String getStatusString();
int getWiFiSignalStrength();
String getMacAddress();
String getIPAddress();
```

#### Configuration

```cpp
void setDebug(bool enable);
void setConnectionTimeout(unsigned long timeout);
bool testConnection();
void loop(); // Call in main loop for automatic management
```

### Enums

#### MicroSafariStatus

- `MICROSAFARI_DISCONNECTED` - Not connected
- `MICROSAFARI_WIFI_CONNECTING` - Connecting to WiFi
- `MICROSAFARI_WIFI_CONNECTED` - WiFi connected
- `MICROSAFARI_PLATFORM_CONNECTED` - Platform verified
- `MICROSAFARI_ERROR` - Error state

### Structures

#### MicroSafariResponse

```cpp
struct MicroSafariResponse {
    int httpCode;           // HTTP response code
    String payload;         // Response body
    bool success;           // Success status
    String errorMessage;    // Error description
};
```

## 📋 Examples

The library includes comprehensive example sketches:

### Basic Examples
- **WiFiConnection**: Basic WiFi connectivity test
- **BasicSensor**: Simple sensor data transmission using fixed parameters
- **DiagnosticsDemo**: System diagnostics and troubleshooting
- **ErrorHandlingDemo**: Error handling and recovery patterns

### Dynamic Data Examples
- **SensorData**: Data transmission testing with both fixed and dynamic methods
- **DynamicSensorData**: Comprehensive dynamic JSON examples for agriculture
- **AdvancedDynamic**: Sophisticated agricultural IoT scenarios with complex data structures
- **AdvancedSensor**: Complex sensor array with multiple data types

### Key Dynamic Capabilities Demonstrated:
- **Custom Sensor Types**: pH, NPK, CO2, conductivity, water depth
- **Nested Data Structures**: GPS locations, soil layers, environmental conditions
- **Agricultural Monitoring**: Multi-crop farms, precision agriculture grids
- **Time-Series Data**: Historical measurements and trend analysis
- **Multi-Language Support**: Indonesian/English bilingual reports
- **Quality Control**: Sensor calibration and validation data

## 🔐 Security

- Uses HTTPS for all communications
- API key authentication
- No sensitive data stored in sketches
- Secure WiFi connection handling

## 🐛 Troubleshooting

### Common Issues

**WiFi Connection Fails**
- Check SSID and password
- Verify WiFi signal strength
- Ensure 2.4GHz network (ESP32 doesn't support 5GHz)

**Authentication Errors (HTTP 401)**
- Verify API key in MicroSafari dashboard
- Check device registration
- Ensure API key is correctly copied

**Service Unavailable (HTTP 503)**
- Platform may be in development mode
- Check platform URL
- Verify Supabase configuration

**Network Errors**
- Check internet connectivity
- Verify firewall settings
- Ensure platform URL is accessible

### Debug Mode

Enable debug output to see detailed logging:

```cpp
microSafari.setDebug(true);
```

## 🌐 Platform Integration

This library integrates with the MicroSafari platform's `/api/ingest` endpoint. The expected payload format is:

```json
{
  "payload": {
    "temperature": 28.5,
    "humidity": 75.0,
    "soil_moisture": 60.3,
    // ... additional sensor data
  }
}
```

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 🆘 Support

- **Documentation**: [MicroSafari Docs](https://docs.microsafari.com)
- **Issues**: [GitHub Issues](https://github.com/microsafari/firmware/issues)
- **Email**: support@microsafari.com

## 🏷️ Version History

- **v1.0.0** - Initial release with core functionality
  - WiFi management
  - HTTP client implementation
  - Data transmission methods
  - Error handling and debugging
  - Example sketches

---

**🌱 Built for Indonesian Farmers, Powered by IoT Innovation**