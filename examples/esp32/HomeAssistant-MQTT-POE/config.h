#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Module types for pin configuration
enum ModuleType {
  MODULE_ESP32_GENERIC = 0,
  MODULE_ESP32_POE = 1,
  MODULE_ESP32_S2 = 2,
  MODULE_ESP32_C3 = 3,
  MODULE_CUSTOM = 99
};

// Configuration structure
struct DSCConfig {
  // Hardware/Module Configuration
  ModuleType moduleType = MODULE_ESP32_POE;  // Default to ESP32-POE as specified
  uint8_t dscClockPin = 13;                 // Default ESP32-POE pins as requested
  uint8_t dscReadPin = 16;
  uint8_t dscPC16Pin = 32;                  // DSC Classic Series only
  uint8_t dscWritePin = 33;                 // Virtual keypad
  
  // Network Configuration
  bool useEthernet = true;              // true for Ethernet, false for WiFi
  bool useDHCP = true;                  // true for DHCP, false for static IP
  
  // WiFi Settings
  char wifiSSID[32] = "";
  char wifiPassword[64] = "";
  
  // Static IP Settings (used when useDHCP = false)
  char staticIP[16] = "192.168.1.100";
  char staticGateway[16] = "192.168.1.1";
  char staticSubnet[16] = "255.255.255.0";
  char staticDNS[16] = "8.8.8.8";
  
  // MQTT Configuration
  char mqttServer[64] = "";
  int mqttPort = 1883;
  char mqttUsername[32] = "";
  char mqttPassword[64] = "";
  char mqttClientName[32] = "dscKeybusInterface";
  
  // DSC System Configuration
  char accessCode[8] = "";
  
  // MQTT Topics (default values match original HomeAssistant-MQTT implementation)
  char mqttPartitionTopic[64] = "dsc/Get/Partition";     // Compatible with dsc/Get/Partition1, etc.
  char mqttZoneTopic[64] = "dsc/Get/Zone";               // Compatible with dsc/Get/Zone1, etc.
  char mqttFireTopic[64] = "dsc/Get/Fire";               // Compatible with dsc/Get/Fire1, etc.
  char mqttPgmTopic[64] = "dsc/Get/PGM";                 // Compatible with dsc/Get/PGM1, etc.
  char mqttTroubleTopic[64] = "dsc/Get/Trouble";         // System trouble status
  char mqttStatusTopic[64] = "dsc/Status";               // Online/offline status
  char mqttSubscribeTopic[64] = "dsc/Set";               // Command topic
  
  // Zone Configuration
  struct {
    char name[32];
    char deviceClass[16];  // door, window, motion, smoke, etc.
    bool enabled;
  } zones[8] = {
    {"Zone 1", "door", true},
    {"Zone 2", "window", true},
    {"Zone 3", "motion", true},
    {"Zone 4", "door", false},
    {"Zone 5", "window", false},
    {"Zone 6", "motion", false},
    {"Zone 7", "smoke", false},
    {"Zone 8", "door", false}
  };
  
  // Configuration validation
  uint32_t configVersion = 1;
  uint32_t checksum = 0;
};

// Global configuration instance
extern DSCConfig config;

// Configuration functions
void loadConfig();
void saveConfig();
void resetConfig();
void setDefaultPinsForModule(ModuleType moduleType);
const char* getModuleName(ModuleType moduleType);
uint32_t calculateChecksum(const DSCConfig& cfg);
bool validateConfig(const DSCConfig& cfg);

#endif