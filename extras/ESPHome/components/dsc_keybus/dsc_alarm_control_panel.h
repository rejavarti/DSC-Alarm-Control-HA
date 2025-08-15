#pragma once

#include "esphome/core/component.h"
#include "esphome/components/alarm_control_panel/alarm_control_panel.h"
#include "esphome/components/alarm_control_panel/alarm_control_panel_call.h"
#include "dsc_keybus.h"

namespace esphome {
namespace dsc_keybus {

class DSCAlarmControlPanel : public alarm_control_panel::AlarmControlPanel, public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  void set_dsc_keybus_component(DSCKeybusComponent *parent) { this->parent_ = parent; }
  void set_partition(uint8_t partition) { this->partition_ = partition; }

  // AlarmControlPanel interface implementation
  uint32_t get_supported_features() const override;
  bool get_requires_code() const override;
  bool get_requires_code_to_arm() const override;

 protected:
  DSCKeybusComponent *parent_{nullptr};
  uint8_t partition_{1};

  void control(const alarm_control_panel::AlarmControlPanelCall &call) override;
  void update_state_from_dsc();
  alarm_control_panel::AlarmControlPanelState get_current_state();
};

}  // namespace dsc_keybus
}  // namespace esphome