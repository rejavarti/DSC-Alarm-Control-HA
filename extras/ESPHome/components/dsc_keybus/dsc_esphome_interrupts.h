// DSC Keybus ESPHome Interrupt Fix
// This file provides proper interrupt attachment for ESPHome environment

#pragma once

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

// Forward declarations for interrupt handlers
#ifdef dscClassicSeries
void IRAM_ATTR dscClockInterrupt();
void IRAM_ATTR dscDataInterrupt();
#else
void IRAM_ATTR dscClockInterrupt();
void IRAM_ATTR dscDataInterrupt();
#endif

namespace esphome {
namespace dsc_keybus {

static const char *const TAG = "dsc_interrupts";

class DSCInterruptManager {
public:
    static bool attach_clock_interrupt(uint8_t pin) {
        ESP_LOGD(TAG, "Attempting to attach clock interrupt to GPIO %d", pin);
        
        try {
            // Use ESPHome's GPIO interrupt system
            auto *gpio_pin = new GPIOPin(pin, gpio::FLAG_INPUT, false);
            if (!gpio_pin) {
                ESP_LOGE(TAG, "Failed to create GPIO pin object for pin %d", pin);
                return false;
            }
            
            // Attach interrupt using ESPHome's ISR system
            gpio_pin->attach_interrupt(&dscClockInterrupt, gpio::INTERRUPT_ANY_EDGE);
            
            ESP_LOGI(TAG, "Successfully attached clock interrupt to GPIO %d", pin);
            return true;
            
        } catch (const std::exception& e) {
            ESP_LOGE(TAG, "Exception attaching clock interrupt: %s", e.what());
            return false;
        }
    }
    
    static bool detach_clock_interrupt(uint8_t pin) {
        ESP_LOGD(TAG, "Detaching clock interrupt from GPIO %d", pin);
        
        try {
            auto *gpio_pin = new GPIOPin(pin, gpio::FLAG_INPUT, false);
            if (gpio_pin) {
                gpio_pin->detach_interrupt();
                ESP_LOGI(TAG, "Successfully detached clock interrupt from GPIO %d", pin);
                return true;
            }
        } catch (const std::exception& e) {
            ESP_LOGE(TAG, "Exception detaching clock interrupt: %s", e.what());
        }
        
        return false;
    }
};

} // namespace dsc_keybus
} // namespace esphome

// Override the Arduino compatibility stubs with proper ESPHome implementations
#ifdef ESP32
#undef attachInterrupt
#undef detachInterrupt

// ESPHome-compatible interrupt attachment
inline void attachInterrupt(uint8_t pin, void (*callback)(), uint8_t mode) {
    ESP_LOGD("dsc_interrupts", "Arduino attachInterrupt called for pin %d", pin);
    
    // For DSC clock pin, use our proper interrupt manager
    if (callback == dscClockInterrupt) {
        if (esphome::dsc_keybus::DSCInterruptManager::attach_clock_interrupt(pin)) {
            ESP_LOGI("dsc_interrupts", "Successfully attached DSC clock interrupt");
        } else {
            ESP_LOGE("dsc_interrupts", "Failed to attach DSC clock interrupt");
        }
    } else {
        ESP_LOGW("dsc_interrupts", "Unknown interrupt callback, using stub");
    }
}

inline void detachInterrupt(uint8_t pin) {
    ESP_LOGD("dsc_interrupts", "Arduino detachInterrupt called for pin %d", pin);
    esphome::dsc_keybus::DSCInterruptManager::detach_clock_interrupt(pin);
}

#endif // ESP32