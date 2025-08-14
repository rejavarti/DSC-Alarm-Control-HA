#include "dsc_wrapper.h"
#include "esphome/core/defines.h"
#include "dsc_arduino_compatibility.h"

#if defined(ESP32) || defined(ESP_PLATFORM)
#include <esp_task_wdt.h>  // For ESP32 watchdog timer management
#endif

// Only include DSC headers here - nowhere else!
// This prevents multiple definition issues
#ifdef dscClassicSeries
#include "dscClassic.h"
#else
#include "dscKeybus.h"
#endif

// Include the default pin definitions
#ifdef ESP8266
#define DSC_DEFAULT_CLOCK_PIN 5   // D1 on NodeMCU
#define DSC_DEFAULT_READ_PIN 4    // D2 on NodeMCU  
#define DSC_DEFAULT_WRITE_PIN 15  // D8 on NodeMCU
#define DSC_DEFAULT_PC16_PIN 14   // D5 on NodeMCU
#else
#define DSC_DEFAULT_CLOCK_PIN 18  // ESP32
#define DSC_DEFAULT_READ_PIN 19   // ESP32
#define DSC_DEFAULT_WRITE_PIN 21  // ESP32
#define DSC_DEFAULT_PC16_PIN 17   // ESP32
#endif

namespace esphome {
namespace dsc_keybus {

DSCWrapper& DSCWrapper::getInstance() {
    static DSCWrapper instance;
    return instance;
}

DSCWrapper::DSCWrapper() : dsc_interface_(nullptr), initialized_(false), hardware_initialized_(false), initialization_failed_(false), initialization_attempts_(0) {
}

DSCWrapper::~DSCWrapper() {
    delete dsc_interface_;
}

void DSCWrapper::init(uint8_t clockPin, uint8_t readPin, uint8_t writePin, uint8_t pc16Pin) {
    if (!initialized_) {
        #ifdef dscClassicSeries
        dsc_interface_ = new dscClassicInterface(clockPin, readPin, pc16Pin, writePin);
        #else
        dsc_interface_ = new dscKeybusInterface(clockPin, readPin, writePin);
        #endif
        initialized_ = true;
    }
}

void DSCWrapper::begin() {
    // Prevent infinite initialization attempts by checking failure state
    if (initialization_failed_ || hardware_initialized_) {
        return;  // Already failed or already initialized
    }

    if (dsc_interface_ && !hardware_initialized_) {
        initialization_attempts_++;
        
        // Limit initialization attempts to prevent infinite loops
        if (initialization_attempts_ > 3) {
            initialization_failed_ = true;
            return;  // Give up after 3 attempts
        }
        
        // Critical safety check for ESP32 LoadProhibited prevention
        // The 0xcececece pattern indicates static variables accessed before ready
#if defined(ESP32) || defined(ESP_PLATFORM)
        // Ensure we have adequate heap memory before hardware initialization
        size_t free_heap = esp_get_free_heap_size();
        if (free_heap < 15000) {  // Less than 15KB free
            return;  // Abort hardware initialization - insufficient memory (will retry next loop)
        }
        
        // Additional safety: check that timer variables are properly initialized
        // This prevents the LoadProhibited crash at 0xcececece address
        if (!initialized_) {
            return;  // Interface not properly initialized - abort (will retry next loop)
        }
#endif
        
        // Initialize hardware with protection against early access
        dsc_interface_->begin();
        
        // Verify that hardware initialization actually succeeded
#if defined(ESP32) || defined(ESP_PLATFORM)
        #ifdef dscClassicSeries
        if (dsc_interface_->esp32_hardware_initialized) {
            hardware_initialized_ = true;
        }
        #else
        if (dsc_interface_->esp32_hardware_initialized) {
            hardware_initialized_ = true;
        }
        #endif
#else
        // For non-ESP32 platforms, assume success if no exception thrown
        hardware_initialized_ = true;
#endif
    }
}

void DSCWrapper::begin(Stream& stream) {
    // Prevent infinite initialization attempts by checking failure state
    if (initialization_failed_ || hardware_initialized_) {
        return;  // Already failed or already initialized
    }

    if (dsc_interface_ && !hardware_initialized_) {
        initialization_attempts_++;
        
        // Limit initialization attempts to prevent infinite loops
        if (initialization_attempts_ > 3) {
            initialization_failed_ = true;
            return;  // Give up after 3 attempts
        }
        
        // Critical safety check for ESP32 LoadProhibited prevention
#if defined(ESP32) || defined(ESP_PLATFORM)
        // Ensure we have adequate heap memory before hardware initialization
        size_t free_heap = esp_get_free_heap_size();
        if (free_heap < 15000) {  // Less than 15KB free
            return;  // Abort hardware initialization - insufficient memory (will retry next loop)
        }
        
        // Additional safety: check that timer variables are properly initialized
        if (!initialized_) {
            return;  // Interface not properly initialized - abort (will retry next loop)
        }
#endif
        
        // Initialize hardware with protection against early access
        dsc_interface_->begin(stream);
        
        // Verify that hardware initialization actually succeeded
#if defined(ESP32) || defined(ESP_PLATFORM)
        #ifdef dscClassicSeries
        if (dsc_interface_->esp32_hardware_initialized) {
            hardware_initialized_ = true;
        }
        #else
        if (dsc_interface_->esp32_hardware_initialized) {
            hardware_initialized_ = true;
        }
        #endif
#else
        // For non-ESP32 platforms, assume success if no exception thrown
        hardware_initialized_ = true;
#endif
    }
}

bool DSCWrapper::loop() {
    if (dsc_interface_ && hardware_initialized_) {
        // Process DSC data without watchdog interference to prevent timing issues
        return dsc_interface_->loop();
    }
    return false;
}

void DSCWrapper::resetStatus() {
    if (dsc_interface_) {
        dsc_interface_->resetStatus();
    }
}

void DSCWrapper::stop() {
    if (dsc_interface_ && hardware_initialized_) {
        dsc_interface_->stop();
        hardware_initialized_ = false; // Reset hardware state
    }
}

bool DSCWrapper::isHardwareInitialized() const {
    return hardware_initialized_;
}

bool DSCWrapper::isInitializationFailed() const {
    return initialization_failed_;
}

void DSCWrapper::write(const char* keys) {
    if (dsc_interface_) {
        dsc_interface_->write(keys);
    }
}

bool DSCWrapper::getStatusChanged() {
    if (dsc_interface_) {
        return dsc_interface_->statusChanged;
    }
    return false;
}

void DSCWrapper::setStatusChanged(bool value) {
    if (dsc_interface_) {
        dsc_interface_->statusChanged = value;
    }
}

bool DSCWrapper::getBufferOverflow() {
    if (dsc_interface_) {
        return dsc_interface_->bufferOverflow;
    }
    return false;
}

void DSCWrapper::setBufferOverflow(bool value) {
    if (dsc_interface_) {
        dsc_interface_->bufferOverflow = value;
    }
}

bool DSCWrapper::getKeybusChanged() {
    if (dsc_interface_) {
        return dsc_interface_->keybusChanged;
    }
    return false;
}

void DSCWrapper::setKeybusChanged(bool value) {
    if (dsc_interface_) {
        dsc_interface_->keybusChanged = value;
    }
}

bool DSCWrapper::getKeybusConnected() {
    if (dsc_interface_) {
        return dsc_interface_->keybusConnected;
    }
    return false;
}

uint8_t* DSCWrapper::getPanelData() {
    if (dsc_interface_) {
        return dsc_interface_->panelData;
    }
    return nullptr;
}

bool DSCWrapper::getArmed(uint8_t partition) {
    if (dsc_interface_ && partition < DSC_MAX_PARTITIONS) {
        return dsc_interface_->armed[partition];
    }
    return false;
}

bool DSCWrapper::getExitDelay(uint8_t partition) {
    if (dsc_interface_ && partition < DSC_MAX_PARTITIONS) {
        return dsc_interface_->exitDelay[partition];
    }
    return false;
}

#ifdef dscClassicSeries
dscClassicInterface* DSCWrapper::getInterface() {
    return dsc_interface_;
}
#else
dscKeybusInterface* DSCWrapper::getInterface() {
    return dsc_interface_;
}
#endif

// Global accessor function
DSCWrapper& getDSC() {
    return DSCWrapper::getInstance();
}

} // namespace dsc_keybus
} // namespace esphome