# DSC Keybus Interface Examples

This directory contains example sketches for the DSC Keybus Interface library with WiFi Manager functionality.

## WiFi Configuration

All examples now include automatic WiFi Manager functionality. **No code modification is required** for basic WiFi setup.

### Quick Start
1. Flash any example to your ESP32
2. Power on the device
3. If no WiFi credentials are saved, the device will create a "DSC-Config" access point
4. Connect to "DSC-Config" with password: `12345678`
5. Open http://192.168.4.1 in your browser
6. Enter your WiFi credentials and click "Save and Connect"
7. The device will restart and connect to your WiFi network

### Examples

#### HomeAssistant-MQTT
MQTT integration for Home Assistant with automatic WiFi configuration.
- **Path**: `esp32/HomeAssistant-MQTT/HomeAssistant-MQTT.ino`
- **Features**: WiFi Manager, MQTT publishing, Home Assistant integration
- **Configuration**: WiFi via web portal, MQTT settings in code

#### VirtualKeypad-Web  
Web-based virtual keypad interface with integrated WiFi configuration.
- **Path**: `esp32/VirtualKeypad-Web/VirtualKeypad-Web.ino`
- **Features**: WiFi Manager, WebSocket interface, virtual keypad controls
- **Configuration**: WiFi via web portal, access keypad at device IP

### Advanced Configuration

For developers who prefer hardcoded credentials, you can still set them in the sketch:

```cpp
const char* wifiSSID = "YourNetworkName";
const char* wifiPassword = "YourPassword";
```

The device will try hardcoded credentials first, then fall back to the web configuration if they fail.

### Troubleshooting

**Device not connecting to WiFi:**
1. Connect to "DSC-Config" network
2. Check that you entered the correct WiFi credentials
3. Ensure your WiFi network is 2.4GHz (ESP32 doesn't support 5GHz)
4. Check WiFi signal strength

**Can't find "DSC-Config" network:**
1. Wait 30-60 seconds after powering on
2. Check serial monitor for status messages
3. Try powering off/on the device

**Need to change WiFi credentials:**
1. Connect to serial monitor
2. The device will fall back to configuration mode if WiFi fails
3. Or clear saved credentials by uploading fresh firmware