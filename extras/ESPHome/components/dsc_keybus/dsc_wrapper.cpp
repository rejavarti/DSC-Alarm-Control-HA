#include "dsc_wrapper.h"
#include "dsc_arduino_compatibility.h"

#ifdef ESP32
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

DSCWrapper::DSCWrapper() : dsc_interface_(nullptr), initialized_(false), hardware_initialized_(false) {
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
    if (dsc_interface_ && !hardware_initialized_) {
#ifdef ESP32
        // Reset watchdog before hardware initialization to prevent timeout
        esp_task_wdt_reset();
#endif
        dsc_interface_->begin();
        hardware_initialized_ = true;
#ifdef ESP32
        // Reset watchdog after hardware initialization
        esp_task_wdt_reset();
#endif
    }
}

void DSCWrapper::begin(Stream& stream) {
    if (dsc_interface_ && !hardware_initialized_) {
#ifdef ESP32
        // Reset watchdog before hardware initialization to prevent timeout
        esp_task_wdt_reset();
#endif
        dsc_interface_->begin(stream);
        hardware_initialized_ = true;
#ifdef ESP32
        // Reset watchdog after hardware initialization
        esp_task_wdt_reset();
#endif
    }
}

bool DSCWrapper::loop() {
    if (dsc_interface_ && hardware_initialized_) {
#ifdef ESP32
        // Reset watchdog during DSC processing to prevent timeout during heavy data processing
        // This is especially important during alarm events when there's heavy keybus traffic
        esp_task_wdt_reset();
#endif
        bool result = dsc_interface_->loop();
#ifdef ESP32
        // Reset watchdog after DSC processing completes
        esp_task_wdt_reset();
#endif
        return result;
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