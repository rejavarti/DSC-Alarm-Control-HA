# PlatformIO Configuration

This repository now includes a `platformio.ini` file that allows you to compile and upload projects using PlatformIO instead of the Arduino IDE.

## Supported Environments

The configuration includes the following pre-configured environments:

### Arduino (AVR) Boards
- `uno` - Arduino Uno (ATmega328P)
- `nano` - Arduino Nano (ATmega328P)
- `mega` - Arduino Mega 2560

### ESP8266 Boards
- `nodemcuv2` - NodeMCU v2
- `d1_mini` - Wemos D1 Mini
- `esp12e` - ESP8266 ESP-12E

### ESP32 Boards
- `esp32dev` - ESP32 Development Board (default)
- `esp32-poe` - ESP32-POE by Olimex
- `esp32s2` - ESP32-S2 Saola
- `esp32c3` - ESP32-C3 DevKit

### Special Environments
- `homeassistant-esp32` - Optimized for Home Assistant MQTT integration (ESP32)
- `homeassistant-esp8266` - Optimized for Home Assistant MQTT integration (ESP8266)

## Getting Started

1. Install PlatformIO:
   ```bash
   pip install platformio
   ```

2. Clone this repository and navigate to it:
   ```bash
   git clone https://github.com/rejavarti/DSC-Alarm-Control-HA.git
   cd DSC-Alarm-Control-HA
   ```

3. Copy an example project to use as your main source:
   ```bash
   # For ESP32 Home Assistant MQTT integration
   cp examples/esp32/HomeAssistant-MQTT/HomeAssistant-MQTT.ino src/main.cpp
   
   # For ESP8266 Home Assistant MQTT integration  
   cp examples/esp8266/HomeAssistant-MQTT/HomeAssistant-MQTT.ino src/main.cpp
   
   # For Arduino basic usage
   cp examples/Arduino/Status/Status.ino src/main.cpp
   ```

4. Build the project:
   ```bash
   # Build for the default environment (ESP32)
   pio run
   
   # Build for a specific environment
   pio run --environment uno
   pio run --environment nodemcuv2
   pio run --environment homeassistant-esp32
   ```

5. Upload to your device (with device connected):
   ```bash
   # Upload to default environment
   pio run --target upload
   
   # Upload to specific environment
   pio run --environment esp32dev --target upload
   ```

6. Monitor serial output:
   ```bash
   pio device monitor
   ```

## Pre-configured Dependencies

The `platformio.ini` includes commonly used dependencies:
- **PubSubClient** - For MQTT connectivity
- **ArduinoJson** - For JSON handling (in Home Assistant environments)
- **WiFi/ESP8266WiFi** - Platform-specific WiFi libraries

## Customization

You can customize the configuration by:
- Adding additional library dependencies to `lib_deps`
- Modifying build flags for specific requirements
- Adding new board environments as needed
- Changing upload/monitor settings

## Environment Selection

The default environment is set to `esp32dev`. You can change this by modifying the `default_envs` setting in the `[platformio]` section of `platformio.ini`.

## Examples Integration

All examples in the `examples/` directory can be used with PlatformIO by copying the `.ino` file to `src/main.cpp` and selecting the appropriate environment for compilation.