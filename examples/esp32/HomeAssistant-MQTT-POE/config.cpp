#include "config.h"
#include <Preferences.h>

// Global configuration instance
DSCConfig config;
Preferences preferences;

// Calculate simple checksum for configuration validation
uint32_t calculateChecksum(const DSCConfig& cfg) {
  uint32_t checksum = 0;
  const uint8_t* data = (const uint8_t*)&cfg;
  size_t size = sizeof(DSCConfig) - sizeof(cfg.checksum); // Exclude checksum field itself
  
  for (size_t i = 0; i < size; i++) {
    checksum += data[i];
  }
  return checksum;
}

// Validate configuration integrity
bool validateConfig(const DSCConfig& cfg) {
  return (cfg.configVersion == 1 && cfg.checksum == calculateChecksum(cfg));
}

// Load configuration from flash
void loadConfig() {
  preferences.begin("dsc-config", true); // Read-only mode
  
  if (preferences.isKey("config")) {
    size_t configSize = preferences.getBytesLength("config");
    if (configSize == sizeof(DSCConfig)) {
      preferences.getBytes("config", &config, sizeof(DSCConfig));
      
      // Validate loaded configuration
      if (validateConfig(config)) {
        Serial.println("Configuration loaded successfully");
        preferences.end();
        return;
      } else {
        Serial.println("Configuration validation failed, using defaults");
      }
    } else {
      Serial.println("Configuration size mismatch, using defaults");
    }
  } else {
    Serial.println("No saved configuration found, using defaults");
  }
  
  preferences.end();
  
  // Use default configuration
  resetConfig();
}

// Save configuration to flash
void saveConfig() {
  // Calculate and set checksum before saving
  config.checksum = calculateChecksum(config);
  
  preferences.begin("dsc-config", false); // Read-write mode
  size_t written = preferences.putBytes("config", &config, sizeof(DSCConfig));
  preferences.end();
  
  if (written == sizeof(DSCConfig)) {
    Serial.println("Configuration saved successfully");
  } else {
    Serial.println("Failed to save configuration");
  }
}

// Reset configuration to defaults
void resetConfig() {
  // Configuration is initialized with defaults in the struct definition
  config = DSCConfig();
  // Set default pins based on module type
  setDefaultPinsForModule(config.moduleType);
  config.checksum = calculateChecksum(config);
  Serial.println("Configuration reset to defaults");
}

// Set default pins based on module type
void setDefaultPinsForModule(ModuleType moduleType) {
  switch (moduleType) {
    case MODULE_ESP32_POE:
      // ESP32-POE specific pins (as requested: 13, 16, 32, 33)
      config.dscClockPin = 13;
      config.dscReadPin = 16;
      config.dscPC16Pin = 32;
      config.dscWritePin = 33;
      break;
      
    case MODULE_ESP32_GENERIC:
      // Standard ESP32 pins (avoiding Ethernet conflicts)
      config.dscClockPin = 4;
      config.dscReadPin = 16;
      config.dscPC16Pin = 17;
      config.dscWritePin = 21;
      break;
      
    case MODULE_ESP32_S2:
      // ESP32-S2 specific pins
      config.dscClockPin = 1;
      config.dscReadPin = 3;
      config.dscPC16Pin = 5;
      config.dscWritePin = 7;
      break;
      
    case MODULE_ESP32_C3:
      // ESP32-C3 specific pins  
      config.dscClockPin = 0;
      config.dscReadPin = 1;
      config.dscPC16Pin = 2;
      config.dscWritePin = 3;
      break;
      
    case MODULE_CUSTOM:
      // Keep current pins when set to custom
      break;
  }
}

// Get module name string for display
const char* getModuleName(ModuleType moduleType) {
  switch (moduleType) {
    case MODULE_ESP32_GENERIC: return "ESP32 Generic";
    case MODULE_ESP32_POE: return "ESP32-POE";
    case MODULE_ESP32_S2: return "ESP32-S2";
    case MODULE_ESP32_C3: return "ESP32-C3";
    case MODULE_CUSTOM: return "Custom";
    default: return "Unknown";
  }
}