/*
 *  Main entry point for PlatformIO build environments
 *  
 *  This file provides the required setup() and loop() functions for Arduino framework
 *  when building specific environments that need compilation targets.
 *  
 *  For actual usage examples, please refer to the examples directory:
 *  - examples/esp32/
 *  - examples/esp8266/
 *  - examples/Arduino/
 */

#include "Arduino.h"

/*
 * This is a placeholder main file for library development and testing.
 * 
 * To use this library:
 * 1. Copy the appropriate example from examples/ directory
 * 2. Customize the WiFi credentials, MQTT settings, and security system configuration
 * 3. Build and upload to your device
 * 
 * Available examples:
 * - HomeAssistant-MQTT: Full Home Assistant integration via MQTT
 * - KeybusReader: Basic keybus monitoring
 * - VirtualKeypad-Web: Web-based keypad interface
 * - And many more in the examples/ directory
 */

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("DSC Keybus Interface Library");
  
  #ifdef HOME_ASSISTANT_INTEGRATION
  Serial.println("Build: Home Assistant Integration");
  Serial.println("This is a placeholder build for library development.");
  Serial.println();
  Serial.println("To use this library:");
  Serial.println("1. Copy examples/esp32/HomeAssistant-MQTT/HomeAssistant-MQTT.ino");
  Serial.println("2. Customize WiFi and MQTT settings");
  Serial.println("3. Build and upload your customized sketch");
  #else
  Serial.println("Build: Library Development");
  Serial.println("This is a placeholder build for library compilation testing.");
  Serial.println();
  Serial.println("To use this library:");
  Serial.println("1. Browse available examples in the examples/ directory");
  Serial.println("2. Copy the appropriate example for your use case");
  Serial.println("3. Customize settings and build your project");
  #endif
  
  Serial.println();
  Serial.println("Library build successful - ready for use!");
}

void loop() {
  // Minimal loop to satisfy Arduino framework requirements
  delay(10000);
  
  #ifdef HOME_ASSISTANT_INTEGRATION
  Serial.println("Library ready - please use the HomeAssistant-MQTT example for actual functionality");
  #else
  Serial.println("Library ready - please use an appropriate example from the examples/ directory");
  #endif
}