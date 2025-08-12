#include "dsc_keybus.h"
#include "esphome/core/log.h"
#include "dsc_wrapper.h"
#include <algorithm>

namespace esphome {
namespace dsc_keybus {

static const char *const TAG = "dsc_keybus";

void DSCKeybusComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up DSC Keybus Interface...");
  
  // Initialize the DSC wrapper (creates interface object but doesn't start hardware)
  getDSC().init(DSC_DEFAULT_CLOCK_PIN, DSC_DEFAULT_READ_PIN, DSC_DEFAULT_WRITE_PIN, DSC_DEFAULT_PC16_PIN);
  
  // Initialize system state
  for (auto *trigger : this->system_status_triggers_) {
    trigger->trigger(STATUS_OFFLINE);
  }
  
  this->force_disconnect_ = false;
  getDSC().resetStatus();

  ESP_LOGCONFIG(TAG, "DSC Keybus Interface setup complete (hardware init deferred)");
}

void DSCKeybusComponent::loop() {
  // Initialize hardware on first loop iteration (system is fully ready)
  if (!getDSC().isHardwareInitialized()) {
    ESP_LOGD(TAG, "Initializing DSC Keybus hardware (timers, interrupts)...");
    try {
      getDSC().begin();
      ESP_LOGI(TAG, "DSC Keybus hardware initialization complete");
    } catch (...) {
      ESP_LOGE(TAG, "Failed to initialize DSC Keybus hardware - will retry next loop");
      return; // Retry on next loop iteration
    }
  }
  
  // Process keybus data only if hardware is initialized
  if (!this->force_disconnect_ && getDSC().isHardwareInitialized()) {
    getDSC().loop();
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