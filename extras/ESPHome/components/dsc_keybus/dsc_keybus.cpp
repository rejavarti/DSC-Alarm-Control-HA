#include "dsc_keybus.h"
#include "esphome/core/log.h"
#include "dsc_wrapper.h"
#include "dsc_esp_idf_timer_fix.h"  // Enhanced ESP-IDF timer compatibility
#include <algorithm>

#ifdef ESP32
#include <esp_task_wdt.h>  // For ESP32 watchdog timer management
#include <esp_heap_caps.h>  // For heap memory management
#include <esp_err.h>        // For ESP error handling
#include <esp_system.h>     // For system functions
#include <esp_idf_version.h>  // For ESP-IDF version detection

// ESP-IDF 5.3.2+ specific includes for enhanced crash prevention
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 3, 0)
#include <esp_timer.h>      // For high-resolution timer
#include <esp_app_desc.h>   // For application descriptor
#define DSC_ESP_IDF_5_3_PLUS_COMPONENT
#endif

#endif

namespace esphome {
namespace dsc_keybus {

static const char *const TAG = "dsc_keybus";

void DSCKeybusComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up DSC Keybus Interface...");

#ifdef ESP32
  // Enhanced ESP-IDF 5.3.2+ LoadProhibited crash prevention
  // The 0xcececece pattern indicates static variables accessed during app_main()
  // This enhanced approach provides multiple layers of protection
  
  #ifdef DSC_ESP_IDF_5_3_PLUS_COMPONENT
  ESP_LOGD(TAG, "ESP-IDF 5.3.2+ detected - applying enhanced LoadProhibited crash prevention");
  
  // ESP-IDF 5.3+ specific variables are now declared in dsc_keybus.h
  // Initialize timestamp now that ESP timer system is safely available
  // Initialize timestamp if not already set (constructor couldn't safely do this)
  if (dsc_esp_idf_init_delay_timestamp == 0) {
    dsc_esp_idf_init_delay_timestamp = esp_timer_get_time() / 1000;
    ESP_LOGD(TAG, "Initialized ESP-IDF stabilization timestamp: %lu ms", dsc_esp_idf_init_delay_timestamp);
  }
  
  // Check if sufficient time has passed since static variable initialization  
  unsigned long current_time_ms = esp_timer_get_time() / 1000;
  if (current_time_ms - dsc_esp_idf_init_delay_timestamp < 2000) {  // 2 second minimum delay
    ESP_LOGD(TAG, "Insufficient stabilization time since ESP-IDF init - deferring setup");
    return;  // Wait longer for system stabilization
  }
  #endif
  
  // Critical: Delay ESP32 setup to prevent LoadProhibited crashes during app_main()
  // The 0xcececece pattern indicates static variables accessed before initialization
  ESP_LOGD(TAG, "Deferring ESP32 hardware setup to prevent LoadProhibited crashes");
  
  // Enhanced heap memory validation with stricter requirements for ESP-IDF 5.3.2+
  size_t free_heap = esp_get_free_heap_size();
  size_t min_heap = 30000;  // Default minimum
  
  #ifdef DSC_ESP_IDF_5_3_PLUS_COMPONENT
  min_heap = 50000;  // Stricter requirement for ESP-IDF 5.3.2+
  #endif
  
  if (free_heap < min_heap) {
    ESP_LOGW(TAG, "Insufficient heap memory detected: %zu bytes free (need %zu) - deferring setup", 
             free_heap, min_heap);
    return;  // Defer setup until more memory is available
  } else {
    ESP_LOGD(TAG, "Available heap memory: %zu bytes", free_heap);
  }
  
  // Additional ESP-IDF 5.3.2+ system readiness checks
  #ifdef DSC_ESP_IDF_5_3_PLUS_COMPONENT
  // Verify that the ESP timer system is fully operational
  esp_timer_handle_t test_timer = nullptr;
  esp_timer_create_args_t test_args = {
    .callback = nullptr,
    .arg = nullptr,
    .dispatch_method = ESP_TIMER_TASK,
    .name = "dsc_setup_test"
  };
  
  esp_err_t timer_test = esp_timer_create(&test_args, &test_timer);
  if (timer_test != ESP_OK || test_timer == nullptr) {
    ESP_LOGW(TAG, "ESP timer system not fully ready (error %d) - deferring setup", timer_test);
    return;  // Timer system not ready
  } else {
    esp_timer_delete(test_timer);  // Clean up test timer
    ESP_LOGD(TAG, "ESP timer system verified operational");
  }
  #endif
  
#endif
  
  // ONLY initialize the DSC wrapper object (NO hardware initialization yet)
  // This creates the interface object but doesn't start timers/interrupts
  getDSC().init(this->clock_pin_, this->read_pin_, this->write_pin_, this->pc16_pin_);
  
  // Initialize system state
  for (auto *trigger : this->system_status_triggers_) {
    trigger->trigger(STATUS_OFFLINE);
  }
  
  this->force_disconnect_ = false;
  getDSC().resetStatus();

  ESP_LOGCONFIG(TAG, "DSC Keybus Interface setup complete (hardware init deferred to loop())");
}

void DSCKeybusComponent::loop() {
  // Enhanced initialization for ESP-IDF 5.3.2+ to prevent LoadProhibited crashes
  // The 0xcececece pattern indicates hardware initialization attempted too early
  if (!getDSC().isHardwareInitialized()) {
    ESP_LOGD(TAG, "System fully ready - initializing DSC Keybus hardware...");
    
#ifdef ESP32
    #ifdef DSC_ESP_IDF_5_3_PLUS_COMPONENT
    // Additional ESP-IDF 5.3.2+ specific readiness checks
    // ESP-IDF 5.3+ specific variables are now declared in dsc_keybus.h
    
    // Initialize timestamp if not set yet (safety fallback)
    if (dsc_esp_idf_init_delay_timestamp == 0) {
      dsc_esp_idf_init_delay_timestamp = esp_timer_get_time() / 1000;
      ESP_LOGD(TAG, "Late initialization of ESP-IDF stabilization timestamp: %lu ms", dsc_esp_idf_init_delay_timestamp);
    }
    
    // Ensure adequate stabilization time for ESP-IDF 5.3.2+
    unsigned long current_time_ms = esp_timer_get_time() / 1000;
    if (current_time_ms - dsc_esp_idf_init_delay_timestamp < 3000) {  // 3 second minimum for hardware init
      ESP_LOGD(TAG, "ESP-IDF 5.3.2+ stabilization period not complete - delaying hardware init");
      return;  // Wait longer for complete system stabilization
    }
    #endif
    
    // Enhanced heap stability check with stricter requirements
    size_t free_heap = esp_get_free_heap_size();
    size_t min_heap = 20000;  // Default minimum
    
    #ifdef DSC_ESP_IDF_5_3_PLUS_COMPONENT
    min_heap = 35000;  // Stricter requirement for ESP-IDF 5.3.2+ hardware init
    #endif
    
    if (free_heap < min_heap) {
      ESP_LOGW(TAG, "Insufficient heap for hardware init: %zu bytes free (need %zu) - delaying", 
               free_heap, min_heap);
      return;  // Defer hardware initialization until heap is stable
    }
#endif
    
    // Enhanced stabilization timing with longer delays for ESP-IDF 5.3.2+
    static uint32_t init_attempt_time = 0;
    uint32_t current_time = millis();
    uint32_t required_delay = 1000;  // Default 1 second
    
    #ifdef DSC_ESP_IDF_5_3_PLUS_COMPONENT
    required_delay = 2000;  // 2 seconds for ESP-IDF 5.3.2+
    #endif
    
    if (init_attempt_time == 0) {
      init_attempt_time = current_time;
      ESP_LOGD(TAG, "Scheduling hardware initialization after %u ms system stabilization", required_delay);
      return;  // Wait for next loop iteration
    }
    
    // Wait for the required stabilization period
    if (current_time - init_attempt_time < required_delay) {
      return;  // Still waiting for system to stabilize
    }
    
    ESP_LOGD(TAG, "System stabilized - initializing DSC Keybus hardware (timers, interrupts)...");
    
    #ifdef DSC_ESP_IDF_5_3_PLUS_COMPONENT
    // Pre-initialize the enhanced timer system for ESP-IDF 5.3.2+
    ESP_LOGD(TAG, "Pre-initializing ESP-IDF 5.3.2+ timer system for DSC interface");
    if (!dsc_esp_timer::dsc_timer_is_initialized()) {
      // Attempt to initialize the enhanced timer system
      bool timer_ready = dsc_esp_timer::dsc_timer_begin(1, 80, nullptr);
      if (!timer_ready) {
        ESP_LOGW(TAG, "Failed to pre-initialize ESP-IDF timer system - retrying next loop");
        init_attempt_time = 0;  // Reset to retry
        return;
      }
      dsc_esp_timer::dsc_timer_end();  // Clean up test initialization
      ESP_LOGD(TAG, "ESP-IDF timer system pre-initialization successful");
    }
    #endif
    
    getDSC().begin();
    ESP_LOGI(TAG, "DSC Keybus hardware initialization complete");
  }
  
  // Process keybus data only if hardware is initialized and not force disconnected
  if (!this->force_disconnect_ && getDSC().isHardwareInitialized()) {
    getDSC().loop();
    
    // Check for buffer overflow condition (similar to Arduino INO improvements)
    if (getDSC().getBufferOverflow()) {
      ESP_LOGW(TAG, "DSC Keybus buffer overflow detected - system may be too busy");
      getDSC().setBufferOverflow(false);
    }
    
    // Handle keybus connection status changes
    if (getDSC().getKeybusChanged()) {
      getDSC().setKeybusChanged(false);
      if (getDSC().getKeybusConnected()) {
        ESP_LOGI(TAG, "DSC Keybus connected");
        for (auto *trigger : this->system_status_triggers_) {
          trigger->trigger(STATUS_ONLINE);
        }
      } else {
        ESP_LOGW(TAG, "DSC Keybus disconnected");
        for (auto *trigger : this->system_status_triggers_) {
          trigger->trigger(STATUS_OFFLINE);
        }
      }
    }
  }
  
  // TODO: Implement full status processing using wrapper methods
  // This is a minimal implementation to test the wrapper pattern
}

// Placeholder methods - will be implemented after wrapper pattern is verified
void DSCKeybusComponent::set_alarm_state(uint8_t partition, const std::string &state, const std::string &code) {
  ESP_LOGW(TAG, "set_alarm_state not yet implemented with wrapper pattern");
}

void DSCKeybusComponent::alarm_disarm(const std::string &code) {
  ESP_LOGW(TAG, "alarm_disarm not yet implemented with wrapper pattern");
}

void DSCKeybusComponent::alarm_arm_home() {
  ESP_LOGW(TAG, "alarm_arm_home not yet implemented with wrapper pattern");
}

void DSCKeybusComponent::alarm_arm_night(const std::string &code) {
  ESP_LOGW(TAG, "alarm_arm_night not yet implemented with wrapper pattern");
}

void DSCKeybusComponent::alarm_arm_away() {
  ESP_LOGW(TAG, "alarm_arm_away not yet implemented with wrapper pattern");
}

void DSCKeybusComponent::alarm_trigger_panic() {
  ESP_LOGW(TAG, "alarm_trigger_panic not yet implemented with wrapper pattern");
}

void DSCKeybusComponent::alarm_trigger_fire() {
  ESP_LOGW(TAG, "alarm_trigger_fire not yet implemented with wrapper pattern");
}

void DSCKeybusComponent::alarm_keypress(const std::string &keys) {
  getDSC().write(keys.c_str());
}

void DSCKeybusComponent::disconnect_keybus() {
  getDSC().stop();
  this->force_disconnect_ = true;
}

// Status methods - wrapper for DSCWrapper methods to maintain YAML compatibility
bool DSCKeybusComponent::getKeybusConnected() {
  return getDSC().getKeybusConnected();
}

bool DSCKeybusComponent::getBufferOverflow() {
  return getDSC().getBufferOverflow();
}

// Helper methods
bool DSCKeybusComponent::is_numeric_code(const std::string &code) {
  return !code.empty() && std::all_of(code.begin(), code.end(), ::isdigit);
}

const char* DSCKeybusComponent::status_text(uint8_t status_code) {
  // TODO: Implement proper status text mapping
  return "unknown";
}

void DSCKeybusComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "DSC Keybus:");
  ESP_LOGCONFIG(TAG, "  Debug level: %u", this->debug_level_);
  ESP_LOGCONFIG(TAG, "  Access code configured: %s", this->access_code_.empty() ? "NO" : "YES");
  ESP_LOGCONFIG(TAG, "  Enable 05 messages: %s", YESNO(this->enable_05_messages_));
}

}  // namespace dsc_keybus
}  // namespace esphome