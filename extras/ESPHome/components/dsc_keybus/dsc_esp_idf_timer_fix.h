#ifndef DSC_ESP_IDF_TIMER_FIX_H
#define DSC_ESP_IDF_TIMER_FIX_H

// ESP-IDF 5.3.2 LoadProhibited Crash Fix
// Addresses the 0xcececece pattern LoadProhibited crashes by providing
// a proper ESP-IDF timer implementation that doesn't rely on Arduino functions

#include "esphome/core/defines.h"

#if defined(ESP32) || defined(ESP_PLATFORM)

#ifdef ARDUINO
  // Arduino ESP32 framework - use existing timer functions
  #include <esp32-hal-timer.h>
  #define DSC_TIMER_MODE_ARDUINO
#else
  // ESP-IDF framework - use native ESP timer API
  #include <esp_timer.h>
  #include <esp_err.h>
  // Include portmacro.h first to ensure proper port definitions
  #include "freertos/portmacro.h"
  
  #include "freertos/FreeRTOS.h"
  #include "freertos/task.h"
  #define DSC_TIMER_MODE_ESP_IDF
#endif

namespace dsc_esp_timer {

// Forward declaration of callback type
typedef void (*timer_callback_t)(void);

// DSC Timer wrapper class for ESP-IDF compatibility
class DSCTimer {
private:
    #ifdef DSC_TIMER_MODE_ESP_IDF
    esp_timer_handle_t esp_timer_handle;
    timer_callback_t callback_func;
    portMUX_TYPE timer_mux;
    bool timer_initialized;
    
    // Static callback wrapper for ESP-IDF
    static void esp_timer_callback(void* arg);
    #endif
    
    #ifdef DSC_TIMER_MODE_ARDUINO
    // Arduino ESP32 implementation
    hw_timer_t* hw_timer_handle;
    portMUX_TYPE timer_mux;
    bool timer_initialized;
    #endif

public:
    DSCTimer();
    ~DSCTimer();
    
    // Initialize timer with callback
    bool begin(int timer_num, uint32_t divider, timer_callback_t callback);
    
    // Start/stop timer
    bool start();
    bool stop();
    
    // Set timer period in microseconds
    bool setAlarmValue(uint32_t microseconds);
    
    // Enable/disable timer alarm
    bool enableAlarm();
    bool disableAlarm();
    
    // Clean up timer
    void end();
    
    // Critical section management
    void enterCritical();
    void exitCritical();
    
    // Check if timer is initialized
    bool isInitialized() const;
};

// Global timer instance for DSC interface
extern DSCTimer dsc_global_timer;

// Helper functions that provide Arduino-like interface for ESP-IDF
bool dsc_timer_begin(int timer_num, uint32_t divider, timer_callback_t callback);
bool dsc_timer_start();
bool dsc_timer_stop();
bool dsc_timer_set_alarm(uint32_t microseconds);
bool dsc_timer_enable_alarm();
bool dsc_timer_disable_alarm();
void dsc_timer_end();
void dsc_timer_enter_critical();
void dsc_timer_exit_critical();
bool dsc_timer_is_initialized();

} // namespace dsc_esp_timer

#endif // ESP32 || ESP_PLATFORM

#endif // DSC_ESP_IDF_TIMER_FIX_H