#pragma once

// DSC Keybus Wrapper - Singleton pattern to prevent multiple definition issues
// This wrapper encapsulates the DSC library to prevent static member conflicts

#include "dsc_arduino_compatibility.h"

// Forward declarations to avoid including DSC headers in other files
#ifdef dscClassicSeries
class dscClassicInterface;
#else
class dscKeybusInterface;
#endif

// DSC Constants that need to be available
// Use different names to avoid conflicts with DSC library constants
#ifndef DSC_MAX_PARTITIONS
#ifdef dscClassicSeries
#define DSC_MAX_PARTITIONS 1
#else
#define DSC_MAX_PARTITIONS 8
#endif
#endif

#ifndef DSC_MAX_ZONES
#ifdef dscClassicSeries  
#define DSC_MAX_ZONES 8
#else
#define DSC_MAX_ZONES 64
#endif
#endif

namespace esphome {
namespace dsc_keybus {

// Singleton wrapper class for DSC interface
class DSCWrapper {
public:
    static DSCWrapper& getInstance();
    
    // Initialize the DSC interface (creates the interface object but doesn't start hardware)
    void init(uint8_t clockPin, uint8_t readPin, uint8_t writePin, uint8_t pc16Pin = 255);
    // Start the hardware interface (timers, interrupts) - call only when system is ready
    void begin();
    void begin(Stream& stream);
    // Check if hardware has been initialized
    bool isHardwareInitialized() const;
    
    // Status methods
    bool loop();
    void resetStatus();
    void stop();
    
    // Write methods
    void write(const char* keys);
    
    // Access to the underlying interface (use carefully)
    #ifdef dscClassicSeries
    dscClassicInterface* getInterface();
    #else
    dscKeybusInterface* getInterface();
    #endif
    
    // Status properties - these forward to the underlying interface
    bool getStatusChanged();
    void setStatusChanged(bool value);
    bool getBufferOverflow();
    void setBufferOverflow(bool value);
    bool getKeybusChanged();
    void setKeybusChanged(bool value);
    bool getKeybusConnected();
    
    // Data access
    uint8_t* getPanelData();  // Returns pointer to panelData array
    
    // Partition and zone status access
    bool getArmed(uint8_t partition);
    bool getExitDelay(uint8_t partition);
    
private:
    DSCWrapper();
    ~DSCWrapper();
    DSCWrapper(const DSCWrapper&) = delete;
    DSCWrapper& operator=(const DSCWrapper&) = delete;
    
    #ifdef dscClassicSeries
    dscClassicInterface* dsc_interface_;
    #else
    dscKeybusInterface* dsc_interface_;
    #endif
    bool initialized_;
    bool hardware_initialized_;
};

// Global accessor function
DSCWrapper& getDSC();

} // namespace dsc_keybus
} // namespace esphome