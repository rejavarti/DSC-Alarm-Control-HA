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
  config.checksum = calculateChecksum(config);
  Serial.println("Configuration reset to defaults");
}