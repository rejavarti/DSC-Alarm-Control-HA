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
  // Early watchdog configuration for ESP-IDF to prevent app_main() hanging
  // Must be done before any complex initialization that might cause stack overflow
  esp_err_t wdt_err = esp_task_wdt_init(30, true);  // 30 second timeout, enable panic
  if (wdt_err == ESP_OK) {
    esp_task_wdt_add(NULL);  // Add current task to watchdog
    ESP_LOGD(TAG, "ESP32 watchdog timer configured for setup (30s timeout)");
  } else {
    ESP_LOGW(TAG, "Failed to initialize watchdog timer: %s", esp_err_to_name(wdt_err));
  }
  
  // Additional early initialization protection for ESP-IDF
  ESP_LOGD(TAG, "ESP-IDF early initialization protection active");
  
  // Check available heap memory before proceeding
  size_t free_heap = esp_get_free_heap_size();
  if (free_heap < 50000) {  // Less than 50KB free
    ESP_LOGW(TAG, "Low heap memory detected: %zu bytes free", free_heap);
  } else {
    ESP_LOGD(TAG, "Available heap memory: %zu bytes", free_heap);
  }
#endif
  
  // Initialize the DSC wrapper (creates interface object but doesn't start hardware)
  getDSC().init(DSC_DEFAULT_CLOCK_PIN, DSC_DEFAULT_READ_PIN, DSC_DEFAULT_WRITE_PIN, DSC_DEFAULT_PC16_PIN);

#ifdef ESP32
  esp_task_wdt_reset();  // Reset watchdog after DSC initialization
#endif
  
  // Initialize system state
  for (auto *trigger : this->system_status_triggers_) {
    trigger->trigger(STATUS_OFFLINE);
  }
  
  this->force_disconnect_ = false;
  getDSC().resetStatus();

#ifdef ESP32
  esp_task_wdt_reset();  // Final watchdog reset before completing setup
  ESP_LOGD(TAG, "ESP32 setup watchdog resets completed successfully");
#endif

  ESP_LOGCONFIG(TAG, "DSC Keybus Interface setup complete (hardware init deferred)");
}

void DSCKeybusComponent::loop() {
#ifdef ESP32
  // Reset watchdog at the start of each loop iteration to prevent freezing
  // This matches the improvements made to the Arduino INO file
  esp_task_wdt_reset();
#endif

  // Initialize hardware on first loop iteration (system is fully ready)
  if (!getDSC().isHardwareInitialized()) {
    ESP_LOGD(TAG, "Initializing DSC Keybus hardware (timers, interrupts)...");
    getDSC().begin();
    ESP_LOGI(TAG, "DSC Keybus hardware initialization complete");

#ifdef ESP32
    esp_task_wdt_reset();  // Reset watchdog after hardware initialization
#endif
  }
  
  // Process keybus data only if hardware is initialized
  if (!this->force_disconnect_ && getDSC().isHardwareInitialized()) {
    getDSC().loop();

#ifdef ESP32
    // Reset watchdog after DSC processing to prevent timeout during heavy processing
    esp_task_wdt_reset();
#endif
    
    // Check for buffer overflow condition (similar to Arduino INO improvements)
    if (getDSC().getBufferOverflow()) {
      ESP_LOGW(TAG, "DSC Keybus buffer overflow detected - system may be too busy");
      getDSC().setBufferOverflow(false);
      
#ifdef ESP32
      esp_task_wdt_reset();  // Reset watchdog after error handling
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