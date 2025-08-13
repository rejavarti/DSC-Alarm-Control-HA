#include "dsc_keybus.h"
#include "esphome/core/log.h"
#include "dsc_wrapper.h"
#include <algorithm>

#ifdef ESP32
#include <esp_task_wdt.h>  // For ESP32 watchdog timer management
#include <esp_heap_caps.h>  // For heap memory management
#include <esp_err.h>        // For ESP error handling
#include <esp_system.h>     // For system functions
#endif

namespace esphome {
namespace dsc_keybus {

static const char *const TAG = "dsc_keybus";

void DSCKeybusComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up DSC Keybus Interface...");

#ifdef ESP32
  // Critical: Delay ESP32 setup to prevent LoadProhibited crashes during app_main()
  // The 0xcececece pattern indicates static variables accessed before initialization
  ESP_LOGD(TAG, "Deferring ESP32 hardware setup to prevent LoadProhibited crashes");
  
  // Check available heap memory before proceeding
  size_t free_heap = esp_get_free_heap_size();
  if (free_heap < 30000) {  // Less than 30KB free
    ESP_LOGW(TAG, "Low heap memory detected: %zu bytes free", free_heap);
  } else {
    ESP_LOGD(TAG, "Available heap memory: %zu bytes", free_heap);
  }
#endif
  
  // ONLY initialize the DSC wrapper object (NO hardware initialization yet)
  // This creates the interface object but doesn't start timers/interrupts
  getDSC().init(DSC_DEFAULT_CLOCK_PIN, DSC_DEFAULT_READ_PIN, DSC_DEFAULT_WRITE_PIN, DSC_DEFAULT_PC16_PIN);
  
  // Initialize system state
  for (auto *trigger : this->system_status_triggers_) {
    trigger->trigger(STATUS_OFFLINE);
  }
  
  this->force_disconnect_ = false;
  getDSC().resetStatus();

  ESP_LOGCONFIG(TAG, "DSC Keybus Interface setup complete (hardware init deferred to loop())");
}

void DSCKeybusComponent::loop() {
  // Initialize hardware on first loop iteration when ESP32 system is fully ready
  // This prevents LoadProhibited crashes (0xcececece) during app_main() startup
  if (!getDSC().isHardwareInitialized()) {
    ESP_LOGD(TAG, "System fully ready - initializing DSC Keybus hardware...");
    
#ifdef ESP32
    // Additional safety check for ESP32 - ensure heap is stable
    size_t free_heap = esp_get_free_heap_size();
    if (free_heap < 20000) {  // Less than 20KB free
      ESP_LOGW(TAG, "Low heap during hardware init: %zu bytes - delaying initialization", free_heap);
      return;  // Defer hardware initialization until heap is stable
    }
#endif
    
    // Small delay to ensure system is completely stable before hardware init
    // This prevents the LoadProhibited crash pattern
    static uint32_t init_attempt_time = 0;
    uint32_t current_time = millis();
    
    if (init_attempt_time == 0) {
      init_attempt_time = current_time;
      ESP_LOGD(TAG, "Scheduling hardware initialization after system stabilization");
      return;  // Wait for next loop iteration
    }
    
    // Wait at least 1000ms after first attempt to ensure system stability
    if (current_time - init_attempt_time < 1000) {
      return;  // Still waiting for system to stabilize
    }
    
    ESP_LOGD(TAG, "System stabilized - initializing DSC Keybus hardware (timers, interrupts)...");
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
#endif
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