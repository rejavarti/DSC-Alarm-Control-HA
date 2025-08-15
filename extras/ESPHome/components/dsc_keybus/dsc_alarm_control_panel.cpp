#include "dsc_alarm_control_panel.h"
#include "esphome/core/log.h"

namespace esphome {
namespace dsc_keybus {

static const char *const TAG = "dsc_alarm_control_panel";

void DSCAlarmControlPanel::setup() {
  if (this->parent_ == nullptr) {
    ESP_LOGE(TAG, "DSC Keybus component is required");
    this->mark_failed();
    return;
  }
  
  ESP_LOGCONFIG(TAG, "Setting up DSC Alarm Control Panel for partition %d", this->partition_);
}

void DSCAlarmControlPanel::loop() {
  // Update state based on DSC status
  update_state_from_dsc();
}

void DSCAlarmControlPanel::dump_config() {
  ESP_LOGCONFIG(TAG, "DSC Alarm Control Panel:");
  ESP_LOGCONFIG(TAG, "  Partition: %d", this->partition_);
}

void DSCAlarmControlPanel::arm_away() {
  ESP_LOGD(TAG, "Arming away partition %d", this->partition_);
  if (this->parent_ != nullptr) {
    this->parent_->alarm_arm_away();
  }
}

void DSCAlarmControlPanel::arm_home() {
  ESP_LOGD(TAG, "Arming home partition %d", this->partition_);
  if (this->parent_ != nullptr) {
    this->parent_->alarm_arm_home();
  }
}

void DSCAlarmControlPanel::arm_night() {
  ESP_LOGD(TAG, "Arming night partition %d", this->partition_);
  if (this->parent_ != nullptr) {
    this->parent_->alarm_arm_night();
  }
}

void DSCAlarmControlPanel::disarm(const std::string &code) {
  ESP_LOGD(TAG, "Disarming partition %d", this->partition_);
  if (this->parent_ != nullptr) {
    this->parent_->alarm_disarm(code);
  }
}

void DSCAlarmControlPanel::update_state_from_dsc() {
  // For now, we'll set a default state
  // TODO: Implement proper state tracking based on DSC status
  if (this->parent_ != nullptr && this->parent_->getKeybusConnected()) {
    // Set state based on DSC keybus status
    auto current_state = get_current_state();
    this->publish_state(current_state);
  }
}

alarm_control_panel::AlarmControlPanelState DSCAlarmControlPanel::get_current_state() {
  // TODO: Implement proper state mapping from DSC status
  // For now, return READY as default when keybus is connected
  if (this->parent_ != nullptr && this->parent_->getKeybusConnected()) {
    return alarm_control_panel::ACP_STATE_DISARMED;
  }
  return alarm_control_panel::ACP_STATE_UNAVAILABLE;
}

}  // namespace dsc_keybus
}  // namespace esphome