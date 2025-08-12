#pragma once

// Minimal DSC Keybus interface for ESPHome compatibility
// This header provides the minimum necessary interface without Arduino dependencies

#include "dsc_common_constants.h"

// Forward declaration of the interface that will be implemented
// in the actual dscKeybusInterface.cpp
class dscKeybusInterfaceMinimal {
public:
  // Minimal interface required by the ESPHome component
  dscKeybusInterfaceMinimal(byte clockPin, byte readPin, byte writePin = 255);
  
  void begin();
  bool loop();
  void stop();
  void resetStatus();
  void write(const char receivedKey);
  void write(const char * receivedKeys, bool blockingWrite = false);
  
  // Status variables that ESPHome reads
  bool statusChanged = false;
  bool keybusConnected = false, keybusChanged = false;
  bool bufferOverflow = false;
  bool writeReady = false;
  bool accessCodePrompt = false;
  
  // Panel/partition status variables
  bool powerChanged = false, powerTrouble = false;
  bool batteryChanged = false, batteryTrouble = false;
  bool keypadFireAlarm = false, keypadPanicAlarm = false;
  bool troubleChanged = false, trouble = false;
  
  // Partition data arrays
  bool disabled[dscPartitions] = {false};
  bool ready[dscPartitions] = {false}, readyChanged[dscPartitions] = {false};
  bool armed[dscPartitions] = {false}, armedAway[dscPartitions] = {false}, armedStay[dscPartitions] = {false};
  bool noEntryDelay[dscPartitions] = {false}, armedChanged[dscPartitions] = {false};
  bool alarm[dscPartitions] = {false}, alarmChanged[dscPartitions] = {false};
  bool exitDelay[dscPartitions] = {false}, exitDelayChanged[dscPartitions] = {false};
  bool entryDelay[dscPartitions] = {false}, entryDelayChanged[dscPartitions] = {false};
  bool fire[dscPartitions] = {false}, fireChanged[dscPartitions] = {false};
  
  // Zone data
  bool openZonesStatusChanged = false;
  bool alarmZonesStatusChanged = false;
  byte openZones[dscZones] = {0}, openZonesChanged[dscZones] = {0};
  byte alarmZones[dscZones] = {0}, alarmZonesChanged[dscZones] = {0};
  
  // Panel data (minimal)
  byte panelData[dscReadSize] = {0};
  byte status[dscPartitions] = {0};
  byte lights[dscPartitions] = {0};
  
  // Write control
  static byte writePartition;
  
private:
  byte clockPin_, readPin_, writePin_;
  bool virtualKeypad_ = false;
};

// Global instance alias for compatibility
extern dscKeybusInterfaceMinimal* dscKeybusInstance;