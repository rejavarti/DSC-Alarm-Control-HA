#include "dsc_esp_idf_timer_fix.h"

#if defined(ESP32) || defined(ESP_PLATFORM)

// Ensure FreeRTOS includes are available for ESP-IDF mode
#ifndef ARDUINO
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif

namespace dsc_esp_timer {

// Global timer instance
DSCTimer dsc_global_timer;

#ifdef DSC_TIMER_MODE_ESP_IDF

// ESP-IDF implementation
void DSCTimer::esp_timer_callback(void* arg) {
    DSCTimer* timer_instance = static_cast<DSCTimer*>(arg);
    if (timer_instance && timer_instance->callback_func) {
        timer_instance->callback_func();
    }
}

DSCTimer::DSCTimer() 
    : esp_timer_handle(nullptr), callback_func(nullptr), timer_initialized(false) {
    timer_mux = portMUX_INITIALIZER_UNLOCKED;
}

DSCTimer::~DSCTimer() {
    end();
}

bool DSCTimer::begin(int timer_num, uint32_t divider, timer_callback_t callback) {
    if (timer_initialized) {
        end(); // Clean up previous timer
    }
    
    callback_func = callback;
    
    esp_timer_create_args_t timer_args = {
        .callback = esp_timer_callback,
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "dsc_timer"
    };
    
    esp_err_t err = esp_timer_create(&timer_args, &esp_timer_handle);
    if (err == ESP_OK && esp_timer_handle != nullptr) {
        timer_initialized = true;
        return true;
    }
    
    return false;
}

bool DSCTimer::start() {
    if (!timer_initialized || esp_timer_handle == nullptr) {
        return false;
    }
    
    // ESP timer starts automatically when alarm is set
    return true;
}

bool DSCTimer::stop() {
    if (!timer_initialized || esp_timer_handle == nullptr) {
        return false;
    }
    
    esp_err_t err = esp_timer_stop(esp_timer_handle);
    return (err == ESP_OK || err == ESP_ERR_INVALID_STATE); // ESP_ERR_INVALID_STATE means already stopped
}

bool DSCTimer::setAlarmValue(uint32_t microseconds) {
    if (!timer_initialized || esp_timer_handle == nullptr) {
        return false;
    }
    
    esp_err_t err = esp_timer_start_periodic(esp_timer_handle, microseconds);
    return (err == ESP_OK);
}

bool DSCTimer::enableAlarm() {
    // In ESP-IDF, alarm is enabled when setAlarmValue is called
    return timer_initialized;
}

bool DSCTimer::disableAlarm() {
    return stop();
}

void DSCTimer::end() {
    if (esp_timer_handle != nullptr) {
        esp_timer_stop(esp_timer_handle);
        esp_timer_delete(esp_timer_handle);
        esp_timer_handle = nullptr;
    }
    timer_initialized = false;
    callback_func = nullptr;
}

void DSCTimer::enterCritical() {
    if (timer_initialized) {
        portENTER_CRITICAL(&timer_mux);
    }
}

void DSCTimer::exitCritical() {
    if (timer_initialized) {
        portEXIT_CRITICAL(&timer_mux);
    }
}

bool DSCTimer::isInitialized() const {
    return timer_initialized && esp_timer_handle != nullptr;
}

#else // DSC_TIMER_MODE_ARDUINO

// Arduino ESP32 implementation
DSCTimer::DSCTimer() 
    : hw_timer_handle(nullptr), timer_initialized(false) {
    timer_mux = portMUX_INITIALIZER_UNLOCKED;
}

DSCTimer::~DSCTimer() {
    end();
}

bool DSCTimer::begin(int timer_num, uint32_t divider, timer_callback_t callback) {
    if (timer_initialized) {
        end(); // Clean up previous timer
    }
    
    hw_timer_handle = timerBegin(timer_num, divider, true);
    if (hw_timer_handle != nullptr) {
        timerAttachInterrupt(hw_timer_handle, callback, true);
        timer_initialized = true;
        return true;
    }
    
    return false;
}

bool DSCTimer::start() {
    if (!timer_initialized || hw_timer_handle == nullptr) {
        return false;
    }
    
    timerStart(hw_timer_handle);
    return true;
}

bool DSCTimer::stop() {
    if (!timer_initialized || hw_timer_handle == nullptr) {
        return false;
    }
    
    timerStop(hw_timer_handle);
    return true;
}

bool DSCTimer::setAlarmValue(uint32_t microseconds) {
    if (!timer_initialized || hw_timer_handle == nullptr) {
        return false;
    }
    
    // Convert microseconds to timer ticks (assuming 80MHz base clock)
    uint32_t ticks = microseconds * 80;
    timerAlarmWrite(hw_timer_handle, ticks, true);
    return true;
}

bool DSCTimer::enableAlarm() {
    if (!timer_initialized || hw_timer_handle == nullptr) {
        return false;
    }
    
    timerAlarmEnable(hw_timer_handle);
    return true;
}

bool DSCTimer::disableAlarm() {
    if (!timer_initialized || hw_timer_handle == nullptr) {
        return false;
    }
    
    timerAlarmDisable(hw_timer_handle);
    return true;
}

void DSCTimer::end() {
    if (hw_timer_handle != nullptr) {
        timerAlarmDisable(hw_timer_handle);
        timerEnd(hw_timer_handle);
        hw_timer_handle = nullptr;
    }
    timer_initialized = false;
}

void DSCTimer::enterCritical() {
    if (timer_initialized) {
        portENTER_CRITICAL(&timer_mux);
    }
}

void DSCTimer::exitCritical() {
    if (timer_initialized) {
        portEXIT_CRITICAL(&timer_mux);
    }
}

bool DSCTimer::isInitialized() const {
    return timer_initialized && hw_timer_handle != nullptr;
}

#endif // DSC_TIMER_MODE_ESP_IDF vs DSC_TIMER_MODE_ARDUINO

// Helper functions that provide Arduino-like interface
bool dsc_timer_begin(int timer_num, uint32_t divider, timer_callback_t callback) {
    return dsc_global_timer.begin(timer_num, divider, callback);
}

bool dsc_timer_start() {
    return dsc_global_timer.start();
}

bool dsc_timer_stop() {
    return dsc_global_timer.stop();
}

bool dsc_timer_set_alarm(uint32_t microseconds) {
    return dsc_global_timer.setAlarmValue(microseconds);
}

bool dsc_timer_enable_alarm() {
    return dsc_global_timer.enableAlarm();
}

bool dsc_timer_disable_alarm() {
    return dsc_global_timer.disableAlarm();
}

void dsc_timer_end() {
    dsc_global_timer.end();
}

void dsc_timer_enter_critical() {
    dsc_global_timer.enterCritical();
}

void dsc_timer_exit_critical() {
    dsc_global_timer.exitCritical();
}

bool dsc_timer_is_initialized() {
    return dsc_global_timer.isInitialized();
}

} // namespace dsc_esp_timer

#endif // ESP32 || ESP_PLATFORM