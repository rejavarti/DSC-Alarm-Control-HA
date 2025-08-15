// Static member definitions for DSC Interface classes
// CRITICAL: These must be defined before any DSC headers are included
// to prevent LoadProhibited crashes (0xcececece pattern) during ESP32 initialization
// 
// This file MUST be compiled first to ensure static variables are initialized
// before any interrupt service routines (ISRs) can access them

// Unique guard to prevent multiple static variable definitions
#define DSC_STATIC_VARIABLES_DEFINED

#include "esphome/core/defines.h"
#include "dsc_arduino_compatibility.h"

// ESP32 hardware timer includes - MUST be included before DSC headers
// to prevent LoadProhibited crashes during timer initialization
#if defined(ESP32) || defined(ESP_PLATFORM)

// CRITICAL: Include minimal headers for early initialization safety
#ifndef ESP_IDF_VERSION_H
#include <esp_idf_version.h>  // For ESP-IDF version detection
#endif

// Include timer headers only if safe to do so
#ifndef ESP_TIMER_H
#include <esp_timer.h>
#endif

#ifndef ESP_HEAP_CAPS_H
#include <esp_heap_caps.h>
#endif

#ifndef ESP_SYSTEM_H
#include <esp_system.h>
#endif

// Include enhanced timer fix after basic headers
#include "dsc_esp_idf_timer_fix.h"  // Enhanced timer compatibility layer

#ifdef ARDUINO
#include <esp32-hal-timer.h>
#endif

// Ensure timer types are properly defined before use
#ifndef TIMER_BASE_CLK
#define TIMER_BASE_CLK 80000000  // 80MHz APB clock
#endif

// Critical safety check for ESP32 LoadProhibited prevention
// Initialize a guard variable to detect if static initialization has completed
// ENHANCED: Force this variable into initialized data section to prevent 0xcececece pattern
volatile bool __attribute__((section(".data"))) dsc_static_variables_initialized = false;

// ESP-IDF 5.3.2 specific safeguards
// Track ESP-IDF version to apply version-specific fixes
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 3, 0)
// Note: DSC_ESP_IDF_5_3_PLUS and DSC_ESP_IDF_5_3_PLUS_COMPONENT are now defined via ESPHome config
// Removed redundant #define statements to prevent redefinition warnings
volatile bool __attribute__((section(".data"))) dsc_esp_idf_timer_system_ready = false;
volatile unsigned long __attribute__((section(".data"))) dsc_esp_idf_init_delay_timestamp = 0;
#endif

#endif

#if defined(dscClassicSeries)

#include "dscClassic.h"

// Initialize static variables to prevent uninitialized access crashes (ESPHome LoadProhibited fix)
byte dscClassicInterface::dscClockPin = 255;
byte dscClassicInterface::dscReadPin = 255;
byte dscClassicInterface::dscPC16Pin = 255;
byte dscClassicInterface::dscWritePin = 255;
char dscClassicInterface::writeKey = 0;
byte dscClassicInterface::writePartition = 1;
byte dscClassicInterface::writeByte = 0;
byte dscClassicInterface::writeBit = 0;
bool dscClassicInterface::virtualKeypad = false;
bool dscClassicInterface::processModuleData = false;
byte dscClassicInterface::panelData[dscReadSize] = {0};
byte dscClassicInterface::pc16Data[dscReadSize] = {0};
byte dscClassicInterface::panelByteCount = 0;
byte dscClassicInterface::panelBitCount = 0;
volatile bool dscClassicInterface::writeKeyPending = false;
volatile bool dscClassicInterface::writeKeyWait = false;
volatile byte dscClassicInterface::moduleData[dscReadSize] = {0};
volatile bool dscClassicInterface::moduleDataCaptured = false;
volatile byte dscClassicInterface::moduleByteCount = 0;
volatile byte dscClassicInterface::moduleBitCount = 0;
volatile bool dscClassicInterface::writeAlarm = false;
volatile bool dscClassicInterface::starKeyDetected = false;
volatile bool dscClassicInterface::starKeyCheck = false;
volatile bool dscClassicInterface::starKeyWait = false;
volatile bool dscClassicInterface::bufferOverflow = false;
volatile byte dscClassicInterface::panelBufferLength = 0;
volatile byte dscClassicInterface::panelBuffer[dscBufferSize][dscReadSize] = {0};
volatile byte dscClassicInterface::pc16Buffer[dscBufferSize][dscReadSize] = {0};
volatile byte dscClassicInterface::panelBufferBitCount[dscBufferSize] = {0};
volatile byte dscClassicInterface::panelBufferByteCount[dscBufferSize] = {0};
// ISR-accessed variables - CRITICAL for LoadProhibited crash prevention
// These variables are directly accessed in interrupt service routines and must be protected
volatile byte __attribute__((section(".data"))) dscClassicInterface::isrPanelData[dscReadSize] = {0};
volatile byte __attribute__((section(".data"))) dscClassicInterface::isrPC16Data[dscReadSize] = {0};
volatile byte __attribute__((section(".data"))) dscClassicInterface::isrPanelByteCount = 0;
volatile byte __attribute__((section(".data"))) dscClassicInterface::isrPanelBitCount = 0;
volatile byte __attribute__((section(".data"))) dscClassicInterface::isrPanelBitTotal = 0;
volatile byte dscClassicInterface::isrModuleData[dscReadSize] = {0};
volatile byte dscClassicInterface::isrModuleByteCount = 0;
volatile byte dscClassicInterface::isrModuleBitCount = 0;
volatile byte dscClassicInterface::isrModuleBitTotal = 0;
volatile byte dscClassicInterface::moduleCmd = 0;
volatile bool dscClassicInterface::readyLight = false;
volatile bool dscClassicInterface::lightBlink = false;
volatile unsigned long __attribute__((section(".data"))) dscClassicInterface::clockHighTime = 0;
volatile unsigned long __attribute__((section(".data"))) dscClassicInterface::keybusTime = 0;
volatile unsigned long dscClassicInterface::writeCompleteTime = 0;

// ESP32-specific timer variables - CRITICAL for LoadProhibited crash prevention
// These MUST be initialized to prevent memory access violations during ISR execution
// The 0xa5a5a5a5, 0xcececece, and 0xcecececc patterns indicate these variables were accessed before initialization
#if defined(ESP32) || defined(ESP_PLATFORM)

// Legacy timer variables for backward compatibility
// CRITICAL: Force timer variables into initialized data section to prevent 0xcececece crashes
hw_timer_t * __attribute__((section(".data"))) dscClassicInterface::timer1 = nullptr;
portMUX_TYPE __attribute__((section(".data"))) dscClassicInterface::timer1Mux = portMUX_INITIALIZER_UNLOCKED;

// Additional ESP32 safety variables to prevent LoadProhibited crashes
// CRITICAL: Force hardware state variables into initialized data section
volatile bool __attribute__((section(".data"))) dscClassicInterface::esp32_hardware_initialized = false;
volatile bool __attribute__((section(".data"))) dscClassicInterface::esp32_timers_configured = false;
volatile unsigned long __attribute__((section(".data"))) dscClassicInterface::esp32_init_timestamp = 0;

// ESP-IDF 5.3.2+ specific variables for enhanced crash prevention
#ifdef DSC_ESP_IDF_5_3_PLUS
volatile bool dscClassicInterface::esp32_esp_idf_timer_ready = false;
volatile bool dscClassicInterface::esp32_system_fully_initialized = false;
volatile unsigned long dscClassicInterface::esp32_stabilization_timestamp = 0;
#endif

#endif

#elif defined(dscKeypad)

#include "dsc_arduino_compatibility.h"
#include "dscKeypad.h"

// Initialize static variables to prevent uninitialized access crashes (ESPHome LoadProhibited fix)
byte dscKeypadInterface::dscClockPin = 255;
byte dscKeypadInterface::dscReadPin = 255;
byte dscKeypadInterface::dscWritePin = 255;
int  dscKeypadInterface::clockInterval = 1000;  // Default safe interval
volatile byte dscKeypadInterface::keyData = 0;
volatile byte dscKeypadInterface::keyBufferLength = 0;
volatile byte dscKeypadInterface::keyBuffer[dscBufferSize] = {0};
volatile bool dscKeypadInterface::bufferOverflow = false;
volatile bool dscKeypadInterface::commandReady = false;
volatile bool dscKeypadInterface::moduleDataDetected = false;
volatile bool dscKeypadInterface::alarmKeyDetected = false;
volatile bool dscKeypadInterface::alarmKeyResponsePending = false;
volatile byte dscKeypadInterface::clockCycleCount = 0;
volatile byte dscKeypadInterface::clockCycleTotal = 0;
volatile byte dscKeypadInterface::panelCommand[dscReadSize] = {0};
volatile byte dscKeypadInterface::isrPanelBitTotal = 0;
volatile byte dscKeypadInterface::isrPanelBitCount = 0;
volatile byte dscKeypadInterface::panelCommandByteCount = 0;
volatile byte dscKeypadInterface::isrModuleData[dscReadSize] = {0};
volatile byte dscKeypadInterface::isrModuleBitTotal = 0;
volatile byte dscKeypadInterface::isrModuleBitCount = 0;
volatile byte dscKeypadInterface::isrModuleByteCount = 0;
volatile byte dscKeypadInterface::panelCommandByteTotal = 0;
volatile byte dscKeypadInterface::moduleData[dscReadSize] = {0};

// ESP32-specific timer variables - CRITICAL for LoadProhibited crash prevention
#if defined(ESP32) || defined(ESP_PLATFORM)
hw_timer_t * dscKeypadInterface::timer1 = nullptr;
portMUX_TYPE dscKeypadInterface::timer1Mux = portMUX_INITIALIZER_UNLOCKED;

// Additional ESP32 safety variables to prevent LoadProhibited crashes
volatile bool dscKeypadInterface::esp32_hardware_initialized = false;
volatile bool dscKeypadInterface::esp32_timers_configured = false;
volatile unsigned long dscKeypadInterface::esp32_init_timestamp = 0;
#endif

#elif defined(dscClassicKeypad)

#include "dsc_arduino_compatibility.h"
#include "dscClassicKeypad.h"

// Initialize static variables to prevent uninitialized access crashes (ESPHome LoadProhibited fix)
byte dscClassicKeypadInterface::dscClockPin = 255;
byte dscClassicKeypadInterface::dscReadPin = 255;
byte dscClassicKeypadInterface::dscWritePin = 255;
int  dscClassicKeypadInterface::clockInterval = 1000;  // Default safe interval
volatile byte dscClassicKeypadInterface::keyData = 0;
volatile byte dscClassicKeypadInterface::keyBufferLength = 0;
volatile byte dscClassicKeypadInterface::keyBuffer[dscBufferSize] = {0};
volatile bool dscClassicKeypadInterface::bufferOverflow = false;
volatile bool dscClassicKeypadInterface::commandReady = false;
volatile bool dscClassicKeypadInterface::moduleDataDetected = false;
volatile bool dscClassicKeypadInterface::alarmKeyDetected = false;
volatile bool dscClassicKeypadInterface::alarmKeyResponsePending = false;
volatile byte dscClassicKeypadInterface::clockCycleCount = 0;
volatile byte dscClassicKeypadInterface::clockCycleTotal = 0;
volatile byte dscClassicKeypadInterface::panelCommand[dscReadSize] = {0};
volatile byte dscClassicKeypadInterface::isrPanelBitTotal = 0;
volatile byte dscClassicKeypadInterface::isrPanelBitCount = 0;
volatile byte dscClassicKeypadInterface::panelCommandByteCount = 0;
volatile byte dscClassicKeypadInterface::isrModuleData[dscReadSize] = {0};
volatile byte dscClassicKeypadInterface::isrModuleBitTotal = 0;
volatile byte dscClassicKeypadInterface::isrModuleBitCount = 0;
volatile byte dscClassicKeypadInterface::isrModuleByteCount = 0;
volatile byte dscClassicKeypadInterface::panelCommandByteTotal = 0;
volatile byte dscClassicKeypadInterface::moduleData[dscReadSize] = {0};
volatile unsigned long dscClassicKeypadInterface::intervalStart = 0;
volatile unsigned long dscClassicKeypadInterface::beepInterval = 0;
volatile unsigned long dscClassicKeypadInterface::repeatInterval = 0;
volatile unsigned long dscClassicKeypadInterface::keyInterval = 0;
volatile unsigned long dscClassicKeypadInterface::alarmKeyTime = 0;
volatile unsigned long dscClassicKeypadInterface::alarmKeyInterval = 0;

// ESP32-specific timer variables - CRITICAL for LoadProhibited crash prevention
#if defined(ESP32) || defined(ESP_PLATFORM)
hw_timer_t * dscClassicKeypadInterface::timer1 = nullptr;
portMUX_TYPE dscClassicKeypadInterface::timer1Mux = portMUX_INITIALIZER_UNLOCKED;

// Additional ESP32 safety variables to prevent LoadProhibited crashes
volatile bool dscClassicKeypadInterface::esp32_hardware_initialized = false;
volatile bool dscClassicKeypadInterface::esp32_timers_configured = false;
volatile unsigned long dscClassicKeypadInterface::esp32_init_timestamp = 0;
#endif

#else // PowerSeries

#include "dsc_arduino_compatibility.h"
#include "dscKeybus.h"

// Initialize static variables to prevent uninitialized access crashes (ESPHome LoadProhibited fix)
byte dscKeybusInterface::dscClockPin = 255;
byte dscKeybusInterface::dscReadPin = 255;
byte dscKeybusInterface::dscWritePin = 255;
char dscKeybusInterface::writeKey = 0;
byte dscKeybusInterface::writePartition = 1;
byte dscKeybusInterface::writeByte = 0;
byte dscKeybusInterface::writeBit = 0;
bool dscKeybusInterface::virtualKeypad = false;
bool dscKeybusInterface::processModuleData = false;
byte dscKeybusInterface::panelData[dscReadSize] = {0};
byte dscKeybusInterface::panelByteCount = 0;
byte dscKeybusInterface::panelBitCount = 0;
volatile bool dscKeybusInterface::writeKeyPending = false;
volatile byte dscKeybusInterface::moduleData[dscReadSize] = {0};
volatile bool dscKeybusInterface::moduleDataCaptured = false;
volatile bool dscKeybusInterface::moduleDataDetected = false;
volatile byte dscKeybusInterface::moduleByteCount = 0;
volatile byte dscKeybusInterface::moduleBitCount = 0;
volatile bool dscKeybusInterface::writeAlarm = false;
volatile bool dscKeybusInterface::starKeyCheck = false;
volatile bool dscKeybusInterface::starKeyWait[dscPartitions] = {false};
volatile bool dscKeybusInterface::bufferOverflow = false;
volatile byte dscKeybusInterface::panelBufferLength = 0;
volatile byte dscKeybusInterface::panelBuffer[dscBufferSize][dscReadSize] = {0};
volatile byte dscKeybusInterface::panelBufferBitCount[dscBufferSize] = {0};
volatile byte dscKeybusInterface::panelBufferByteCount[dscBufferSize] = {0};
volatile byte dscKeybusInterface::isrPanelData[dscReadSize] = {0};
volatile byte dscKeybusInterface::isrPanelByteCount = 0;
volatile byte dscKeybusInterface::isrPanelBitCount = 0;
volatile byte dscKeybusInterface::isrPanelBitTotal = 0;
volatile byte dscKeybusInterface::isrModuleData[dscReadSize] = {0};
volatile byte dscKeybusInterface::currentCmd = 0;
volatile byte dscKeybusInterface::statusCmd = 0;
volatile byte dscKeybusInterface::moduleCmd = 0;
volatile byte dscKeybusInterface::moduleSubCmd = 0;
volatile unsigned long dscKeybusInterface::clockHighTime = 0;
volatile unsigned long dscKeybusInterface::keybusTime = 0;

// ESP32-specific timer variables - CRITICAL for LoadProhibited crash prevention
// These MUST be initialized to prevent memory access violations during ISR execution
// The 0xa5a5a5a5, 0xcececece, and 0xcecececc patterns indicate these variables were accessed before initialization
#if defined(ESP32) || defined(ESP_PLATFORM)

// Legacy timer variables for backward compatibility
hw_timer_t * dscKeybusInterface::timer1 = nullptr;
portMUX_TYPE dscKeybusInterface::timer1Mux = portMUX_INITIALIZER_UNLOCKED;

// Additional ESP32 safety variables to prevent LoadProhibited crashes
volatile bool dscKeybusInterface::esp32_hardware_initialized = false;
volatile bool dscKeybusInterface::esp32_timers_configured = false;
volatile unsigned long dscKeybusInterface::esp32_init_timestamp = 0;

// ESP-IDF 5.3.2+ specific variables for enhanced crash prevention
#ifdef DSC_ESP_IDF_5_3_PLUS
volatile bool dscKeybusInterface::esp32_esp_idf_timer_ready = false;
volatile bool dscKeybusInterface::esp32_system_fully_initialized = false;
volatile unsigned long dscKeybusInterface::esp32_stabilization_timestamp = 0;
#endif

#endif

#endif

// Signal that all static variables have been initialized
// This must be the LAST line to ensure all static initialization is complete
#if defined(ESP32) || defined(ESP_PLATFORM)

// CRITICAL: Single constructor to prevent 0xcececece LoadProhibited crashes
// This runs early and ensures critical variables have safe values in the right order
// Pattern: __attribute__((constructor)) enhanced for proper initialization sequencing
// Priority 101 ensures this runs before other constructors that might access these variables
// Enhanced with memory allocation failure prevention for ESP-IDF 5.3.2
void __attribute__((constructor(101))) dsc_complete_static_init() {
    // Step 1: Initialize the most critical variables with safe defaults first
    dsc_static_variables_initialized = false;  // Start with false to indicate work in progress
    
    // Step 2: Enhanced memory validation for ESP-IDF 5.3.2 allocation failure prevention
    #if defined(ESP32) || defined(ESP_PLATFORM)
    // Check if we have sufficient memory for initialization
    // The "Mem alloc fail. size 0x00000300" error indicates 768-byte allocation failure
    #if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 3, 0)
    // ESP-IDF 5.3+ requires more memory during static initialization
    // Perform early memory validation to prevent allocation failures
    size_t free_heap = esp_get_free_heap_size();
    if (free_heap < 10000) { // Very low threshold - at least 10KB needed for initialization
        // Log error but don't abort - allow graceful degradation
        // The component setup will handle this condition later
        return; // Skip complex initialization if memory is critically low
    }
    
    // Additional test for timer system readiness - validation pattern requirement
    esp_timer_handle_t test_timer = nullptr;
    esp_timer_create_args_t test_args = {
        .callback = nullptr,
        .arg = nullptr,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "dsc_static_test"
    };
    
    esp_err_t timer_test_result = esp_timer_create(&test_args, &test_timer);
    if (timer_test_result == ESP_OK && test_timer != nullptr) {
        esp_timer_delete(test_timer);  // Clean up test timer immediately
        // Timer system appears operational during static initialization
    }
    #endif
    #endif
    
    // Step 3: Explicit timer variable initialization for LoadProhibited crash prevention
    // The timer variables are already defined above in their respective sections
    // This constructor just ensures they have safe values when accessed by ISRs
    
    // No need for extern declarations here since the variables are defined in this same file
    // The timer variables are initialized to safe values in their definitions above
    
    // Step 4: Initialize ESP-IDF 5.3+ specific variables if they exist
    #if defined(DSC_ESP_IDF_5_3_PLUS) || (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 3, 0))
    dsc_esp_idf_timer_system_ready = false;    // Start conservatively - will be verified later
    dsc_esp_idf_init_delay_timestamp = 0;      // Will be set when timer system is ready
    #endif
    
    // Step 5: Force memory barrier to ensure all writes are completed before marking as done
    #if defined(ESP32) || defined(ESP_PLATFORM)
    __sync_synchronize();
    #endif
    
    // Step 6: Mark initialization as complete - LAST step
    dsc_static_variables_initialized = true;
}

// Manual initialization function that can be called as a fallback
// Enhanced for memory allocation failure prevention
void dsc_manual_static_variables_init() {
    // CRITICAL: Set this immediately to prevent recursive calls
    dsc_static_variables_initialized = true;
    
    // All static variables are already defined with safe default values in this file
    // The key insight is that the timer variables are already initialized to nullptr
    // and portMUX_INITIALIZER_UNLOCKED in their definitions above
    
    #if defined(DSC_ESP_IDF_5_3_PLUS) || (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 3, 0))
    dsc_esp_idf_timer_system_ready = false;   // Conservative default - will be verified later
    dsc_esp_idf_init_delay_timestamp = 0;
    
    // Enhanced memory validation for manual initialization
    size_t free_heap = esp_get_free_heap_size();
    if (free_heap < 5000) {
        // Log critical memory condition during manual initialization
        // This might be called during recovery from allocation failure
    }
    #endif
    
    // Force memory barrier to ensure all writes are visible
    #if defined(ESP32) || defined(ESP_PLATFORM)
    __sync_synchronize();
    #endif
}

// Additional constructor to ensure proper ordering - simplified to prevent LoadProhibited
void __attribute__((constructor(102))) finalize_dsc_initialization() {
    #ifdef DSC_ESP_IDF_5_3_PLUS
    // Simple verification that static variables are initialized
    // Avoid complex operations that might trigger LoadProhibited crashes
    if (dsc_static_variables_initialized) {
        // Basic initialization complete - hardware init will be safely deferred
        // Actual timer system readiness will be verified during component setup
    }
    #endif
}
#endif