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

uint32_t DSCAlarmControlPanel::get_supported_features() const {
  return alarm_control_panel::ACP_FEAT_ARM_HOME | 
         alarm_control_panel::ACP_FEAT_ARM_AWAY | 
         alarm_control_panel::ACP_FEAT_ARM_NIGHT;
}

bool DSCAlarmControlPanel::get_requires_code() const {
  return true;  // DSC panels typically require a code
}

bool DSCAlarmControlPanel::get_requires_code_to_arm() const {
  return false;  // DSC panels can arm without a code
}

void DSCAlarmControlPanel::control(const alarm_control_panel::AlarmControlPanelCall &call) {
  if (call.get_state()) {
    auto state = *call.get_state();
    auto code = call.get_code().value_or("");
    
    switch (state) {
      case alarm_control_panel::ACP_STATE_DISARMED:
        ESP_LOGD(TAG, "Disarming partition %d", this->partition_);
        if (this->parent_ != nullptr) {
          this->parent_->alarm_disarm(code);
        }
        break;
        
      case alarm_control_panel::ACP_STATE_ARMED_HOME:
        ESP_LOGD(TAG, "Arming home partition %d", this->partition_);
        if (this->parent_ != nullptr) {
          this->parent_->alarm_arm_home();
        }
        break;
        
      case alarm_control_panel::ACP_STATE_ARMED_AWAY:
        ESP_LOGD(TAG, "Arming away partition %d", this->partition_);
        if (this->parent_ != nullptr) {
          this->parent_->alarm_arm_away();
        }
        break;
        
      case alarm_control_panel::ACP_STATE_ARMED_NIGHT:
        ESP_LOGD(TAG, "Arming night partition %d", this->partition_);
        if (this->parent_ != nullptr) {
          this->parent_->alarm_arm_night(code);
        }
        break;
        
      default:
        ESP_LOGW(TAG, "Unsupported state requested: %d", static_cast<int>(state));
        break;
    }
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
  // For now, return DISARMED as default when keybus is connected
  if (this->parent_ != nullptr && this->parent_->getKeybusConnected()) {
    return alarm_control_panel::ACP_STATE_DISARMED;
  }
  return alarm_control_panel::ACP_STATE_DISARMED;
}

}  // namespace dsc_keybus
}  // namespace esphome