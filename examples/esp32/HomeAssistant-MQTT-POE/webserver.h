#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>
#include "config.h"

// Web server instance
extern WebServer server;

// Debug/status tracking structures
struct DebugInfo {
  // MQTT status
  bool mqttConnected = false;
  unsigned long mqttLastConnectTime = 0;
  int mqttReconnectCount = 0;
  
  // Alarm system status
  bool alarmSystemConnected = false;
  unsigned long systemUptime = 0;
  
  // Recent events (circular buffers)
  struct AlarmEvent {
    char timestamp[20];
    char description[64];
    uint8_t partition;
  };
  
  struct AccessCodeEvent {
    char timestamp[20];
    char code[8];  // Masked for security
    bool valid;
  };
  
  AlarmEvent recentAlarms[5];
  AccessCodeEvent recentCodes[5];
  int alarmIndex = 0;
  int codeIndex = 0;
  
  // Zone status
  struct ZoneStatus {
    bool open = false;
    bool motion = false;
    bool glassBreak = false;
    bool fire = false;
    bool pgmOutput = false;
    unsigned long lastChanged = 0;
  };
  
  ZoneStatus zones[8];
};

extern DebugInfo debugInfo;

// Web server functions
void setupWebServer();
void handleWebServer();
void handleRoot();
void handleConfig();
void handleConfigSave();
void handleDebug();
void handleRestart();
void handleWiFiScan();
void handleMQTTTest();

// Utility functions
void addAlarmEvent(const char* description, uint8_t partition = 0);
void addAccessCodeEvent(const char* code, bool valid);
void updateZoneStatus(int zone, bool open, bool motion = false, bool glassBreak = false, bool fire = false, bool pgmOutput = false);
String getTimeString();
String getMaskedCode(const char* code);

// HTML templates
String getHTMLHeader(const char* title);
String getHTMLFooter();
String getConfigPage();
String getDebugPage();

#endif