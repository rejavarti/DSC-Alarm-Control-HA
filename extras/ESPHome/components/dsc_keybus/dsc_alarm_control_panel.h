#pragma once

#include "esphome/core/component.h"
#include "esphome/components/alarm_control_panel/alarm_control_panel.h"
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
  void arm_away() override;
  void arm_home() override;
  void arm_night() override;
  void disarm(const std::string &code) override;

 protected:
  DSCKeybusComponent *parent_{nullptr};
  uint8_t partition_{1};

  void update_state_from_dsc();
  alarm_control_panel::AlarmControlPanelState get_current_state();
};

}  // namespace dsc_keybus
}  // namespace esphome