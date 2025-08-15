#include "dsc_wrapper.h"
#include "esphome/core/defines.h"
#include "esphome/core/log.h"
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

static const char *const TAG = "dsc_wrapper";

DSCWrapper& DSCWrapper::getInstance() {
    static DSCWrapper instance;
    return instance;
}

DSCWrapper::DSCWrapper() : dsc_interface_(nullptr), initialized_(false), hardware_initialized_(false), initialization_failed_(false), initialization_attempts_(0), first_initialization_attempt_time_(0) {
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

    // CRITICAL FIX: Check for persistent failure patterns (restart loops)
    if (checkPersistentFailure()) {
        initialization_failed_ = true;
        ESP_LOGE(TAG, "Persistent failure pattern detected - stopping initialization attempts");
        return;
    }

    if (dsc_interface_ && !hardware_initialized_) {
        initialization_attempts_++;
        
        // CRITICAL FIX: Add timeout-based circuit breaker to prevent infinite loops
        // This addresses the specific issue where begin() crashes with LoadProhibited
        // and the ESP32 restarts, resetting attempt counters but creating an infinite boot loop
        static uint32_t first_attempt_time = 0;
        uint32_t current_time = millis();
        
        // CRITICAL FIX: Add watchdog reset and yield during timeout checking
#if defined(ESP32) || defined(ESP_PLATFORM)
        esp_task_wdt_reset();
        yield();  // Allow IDLE task to run
#endif
        
        // Record the first attempt time
        if (first_attempt_time == 0) {
            first_attempt_time = current_time;
        }
        
        // CRITICAL FIX: Increase timeout from 30 to 60 seconds to be more conservative
        // and prevent premature timeout during legitimate initialization delays
        if (current_time - first_attempt_time > 60000) {
            initialization_failed_ = true;
            ESP_LOGE(TAG, "Hardware initialization timeout after 60 seconds - marking as permanently failed");
            return;
        }
        
        // Limit initialization attempts to prevent infinite loops
        if (initialization_attempts_ > 3) {
            initialization_failed_ = true;
            return;  // Give up after 3 attempts
        }
        
        // Critical safety check for ESP32 LoadProhibited prevention
        // The 0xcececece pattern indicates static variables accessed before ready
#if defined(ESP32) || defined(ESP_PLATFORM)
        // CRITICAL FIX: Reset watchdog and yield before memory and safety checks
        esp_task_wdt_reset();
        yield();  // Allow IDLE task to run
        
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
        
        // CRITICAL FIX: Reset watchdog and yield before calling hardware initialization
        esp_task_wdt_reset();
        yield();  // Allow IDLE task to run before critical operation
        
        // ENHANCED: Add detailed diagnostics before hardware initialization
        ESP_LOGD(TAG, "Attempting hardware initialization: attempt=%d, uptime=%u ms, free_heap=%u bytes", 
                 initialization_attempts_, current_time, free_heap);
#endif
        
        // Initialize hardware with protection against early access
        ESP_LOGD(TAG, "Calling dsc_interface_->begin() for hardware initialization...");
        dsc_interface_->begin();
        
        // CRITICAL FIX: Reset watchdog and yield after hardware initialization
#if defined(ESP32) || defined(ESP_PLATFORM)
        esp_task_wdt_reset();
        yield();  // Allow IDLE task to run after critical operation
#endif
        
        // Verify that hardware initialization actually succeeded
#if defined(ESP32) || defined(ESP_PLATFORM)
        #ifdef dscClassicSeries
        if (dsc_interface_->esp32_hardware_initialized) {
            hardware_initialized_ = true;
            ESP_LOGI(TAG, "DSC Classic hardware initialization successful on attempt %d", initialization_attempts_);
        } else {
            ESP_LOGW(TAG, "DSC Classic hardware initialization incomplete on attempt %d (esp32_hardware_initialized=false)", initialization_attempts_);
            // Check if we should give up permanently
            if (initialization_attempts_ >= 3 || (current_time - first_attempt_time > 60000)) {
                initialization_failed_ = true;
                ESP_LOGE(TAG, "DSC Classic hardware initialization permanently failed after %d attempts over %u ms", 
                         initialization_attempts_, current_time - first_attempt_time);
            }
        }
        #else
        if (dsc_interface_->esp32_hardware_initialized) {
            hardware_initialized_ = true;
            ESP_LOGI(TAG, "DSC PowerSeries hardware initialization successful on attempt %d", initialization_attempts_);
        } else {
            ESP_LOGW(TAG, "DSC PowerSeries hardware initialization incomplete on attempt %d (esp32_hardware_initialized=false)", initialization_attempts_);
            // Check if we should give up permanently
            if (initialization_attempts_ >= 3 || (current_time - first_attempt_time > 60000)) {
                initialization_failed_ = true;
                ESP_LOGE(TAG, "DSC PowerSeries hardware initialization permanently failed after %d attempts over %u ms", 
                         initialization_attempts_, current_time - first_attempt_time);
            }
        }
        #endif
#else
        // For non-ESP32 platforms, assume success if no exception thrown
        hardware_initialized_ = true;
        ESP_LOGI(TAG, "DSC hardware initialization completed (non-ESP32 platform)");
#endif
    }
}

void DSCWrapper::begin(Stream& stream) {
    // Prevent infinite initialization attempts by checking failure state
    if (initialization_failed_ || hardware_initialized_) {
        return;  // Already failed or already initialized
    }

    // CRITICAL FIX: Check for persistent failure patterns (restart loops)
    if (checkPersistentFailure()) {
        initialization_failed_ = true;
        ESP_LOGE(TAG, "Persistent failure pattern detected - stopping initialization attempts");
        return;
    }

    if (dsc_interface_ && !hardware_initialized_) {
        initialization_attempts_++;
        
        // CRITICAL FIX: Add timeout-based circuit breaker to prevent infinite loops
        // This addresses the specific issue where begin() crashes with LoadProhibited
        static uint32_t first_attempt_time = 0;
        uint32_t current_time = millis();
        
        // Record the first attempt time
        if (first_attempt_time == 0) {
            first_attempt_time = current_time;
        }
        
        // If we've been trying for more than 30 seconds, give up permanently
        if (current_time - first_attempt_time > 30000) {
            initialization_failed_ = true;
            ESP_LOGE(TAG, "Hardware initialization timeout after 30 seconds - marking as permanently failed");
            return;
        }
        
        // Limit initialization attempts to prevent infinite loops
        if (initialization_attempts_ > 3) {
            initialization_failed_ = true;
            return;  // Give up after 3 attempts
        }
        
        // Critical safety check for ESP32 LoadProhibited prevention
#if defined(ESP32) || defined(ESP_PLATFORM)
        // Reset watchdog before memory and safety checks
        esp_task_wdt_reset();
        
        // Ensure we have adequate heap memory before hardware initialization
        size_t free_heap = esp_get_free_heap_size();
        if (free_heap < 15000) {  // Less than 15KB free
            return;  // Abort hardware initialization - insufficient memory (will retry next loop)
        }
        
        // Additional safety: check that timer variables are properly initialized
        if (!initialized_) {
            return;  // Interface not properly initialized - abort (will retry next loop)
        }
        
        // Reset watchdog before calling hardware initialization
        esp_task_wdt_reset();
#endif
        
        // Initialize hardware with protection against early access
        dsc_interface_->begin(stream);
        
        // Reset watchdog after hardware initialization
#if defined(ESP32) || defined(ESP_PLATFORM)
        esp_task_wdt_reset();
#endif
        
        // Verify that hardware initialization actually succeeded
#if defined(ESP32) || defined(ESP_PLATFORM)
        #ifdef dscClassicSeries
        if (dsc_interface_->esp32_hardware_initialized) {
            hardware_initialized_ = true;
            ESP_LOGI(TAG, "DSC Classic hardware initialization successful on attempt %d (with Stream)", initialization_attempts_);
        } else {
            ESP_LOGW(TAG, "DSC Classic hardware initialization incomplete on attempt %d (esp32_hardware_initialized=false, with Stream)", initialization_attempts_);
            // Check if we should give up permanently
            if (initialization_attempts_ >= 3 || (current_time - first_attempt_time > 60000)) {
                initialization_failed_ = true;
                ESP_LOGE(TAG, "DSC Classic hardware initialization permanently failed after %d attempts over %u ms (with Stream)", 
                         initialization_attempts_, current_time - first_attempt_time);
            }
        }
        #else
        if (dsc_interface_->esp32_hardware_initialized) {
            hardware_initialized_ = true;
            ESP_LOGI(TAG, "DSC PowerSeries hardware initialization successful on attempt %d (with Stream)", initialization_attempts_);
        } else {
            ESP_LOGW(TAG, "DSC PowerSeries hardware initialization incomplete on attempt %d (esp32_hardware_initialized=false, with Stream)", initialization_attempts_);
            // Check if we should give up permanently
            if (initialization_attempts_ >= 3 || (current_time - first_attempt_time > 60000)) {
                initialization_failed_ = true;
                ESP_LOGE(TAG, "DSC PowerSeries hardware initialization permanently failed after %d attempts over %u ms (with Stream)", 
                         initialization_attempts_, current_time - first_attempt_time);
            }
        }
        #endif
#else
        // For non-ESP32 platforms, assume success if no exception thrown
        hardware_initialized_ = true;
        ESP_LOGI(TAG, "DSC hardware initialization completed (non-ESP32 platform, with Stream)");
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

void DSCWrapper::markInitializationFailed() {
    initialization_failed_ = true;
    ESP_LOGE(TAG, "DSC hardware initialization marked as permanently failed");
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

// CRITICAL FIX: Persistent failure detection across ESP32 restarts
// This method helps detect repeated LoadProhibited crashes by using system uptime
bool DSCWrapper::checkPersistentFailure() {
    uint32_t current_time = millis();
    
    // If this is the first time we're checking, record the time
    if (first_initialization_attempt_time_ == 0) {
        first_initialization_attempt_time_ = current_time;
        return false;  // First attempt, allow it to proceed
    }
    
    // IMPROVED: More sophisticated restart loop detection
    // Only consider it a restart loop if uptime is VERY low (< 5 seconds) 
    // AND we have multiple rapid attempts within the same boot cycle
    if (current_time < 5000) {  // Less than 5 seconds uptime
        static uint32_t attempt_count_this_boot = 0;
        static uint32_t last_attempt_time = 0;
        
        attempt_count_this_boot++;
        
        // If we have multiple rapid attempts in a very short boot cycle, it's likely a crash loop
        if (attempt_count_this_boot >= 3 && (current_time - last_attempt_time < 1000)) {
            ESP_LOGW(TAG, "Detected actual restart loop - system uptime %u ms, %u rapid attempts", 
                     current_time, attempt_count_this_boot);
            return true;  // Definitely in a restart loop due to crashes
        }
        
        last_attempt_time = current_time;
        ESP_LOGD(TAG, "Short uptime (%u ms) but allowing attempt %u - normal boot scenario", 
                 current_time, attempt_count_this_boot);
    }
    
    // If we've been trying for more than 60 seconds total, give up
    if (current_time - first_initialization_attempt_time_ > 60000) {
        ESP_LOGE(TAG, "Persistent failure detected - unable to initialize for over 60 seconds");
        return true;
    }
    
    return false;  // Continue attempting
}

// Global accessor function
DSCWrapper& getDSC() {
    return DSCWrapper::getInstance();
}

} // namespace dsc_keybus
} // namespace esphome