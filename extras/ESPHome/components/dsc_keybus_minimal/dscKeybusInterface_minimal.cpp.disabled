#include "dscKeybusInterface_minimal.h"

// Static variable initialization
byte dscKeybusInterfaceMinimal::writePartition = 1;
dscKeybusInterfaceMinimal* dscKeybusInstance = nullptr;

dscKeybusInterfaceMinimal::dscKeybusInterfaceMinimal(byte clockPin, byte readPin, byte writePin) 
  : clockPin_(clockPin), readPin_(readPin), writePin_(writePin) {
  if (writePin != 255) virtualKeypad_ = true;
  dscKeybusInstance = this; // Set global instance
}

void dscKeybusInterfaceMinimal::begin() {
  // Initialize GPIO pins (stubbed for ESPHome - actual GPIO management handled by ESPHome)
  writeReady = true;
}

bool dscKeybusInterfaceMinimal::loop() {
  // Minimal loop implementation - just return false for now
  // In a real implementation, this would read from the DSC bus
  // For ESPHome, the actual DSC communication will be handled elsewhere
  return false;
}

void dscKeybusInterfaceMinimal::stop() {
  // Stop DSC communication
  keybusConnected = false;
  writeReady = false;
}

void dscKeybusInterfaceMinimal::resetStatus() {
  // Reset all status flags
  statusChanged = false;
  keybusChanged = false;
  bufferOverflow = false;
  powerChanged = false;
  batteryChanged = false;
  troubleChanged = false;
  
  // Reset partition status
  for (byte i = 0; i < dscPartitions; i++) {
    readyChanged[i] = false;
    armedChanged[i] = false;
    alarmChanged[i] = false;
    exitDelayChanged[i] = false;
    entryDelayChanged[i] = false;
    fireChanged[i] = false;
  }
  
  // Reset zone status
  openZonesStatusChanged = false;
  alarmZonesStatusChanged = false;
  for (byte i = 0; i < dscZones; i++) {
    openZonesChanged[i] = 0;
    alarmZonesChanged[i] = 0;
  }
}

void dscKeybusInterfaceMinimal::write(const char receivedKey) {
  // Stub implementation for single key write
  // In a real implementation, this would send the key to the DSC bus
}

void dscKeybusInterfaceMinimal::write(const char * receivedKeys, bool blockingWrite) {
  // Stub implementation for multi-key write
  // In a real implementation, this would send the keys to the DSC bus
}