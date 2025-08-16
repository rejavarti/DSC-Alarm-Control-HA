#pragma once

#include "esphome/core/component.h"
#include "dsc_keybus.h"

namespace esphome {
namespace dsc_keybus {

class DSCAlarmControlPanel : public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  void set_dsc_keybus_component(DSCKeybusComponent *parent) { this->parent_ = parent; }
  void set_partition(uint8_t partition) { this->partition_ = partition; }

 protected:
  DSCKeybusComponent *parent_{nullptr};
  uint8_t partition_{1};
};

}  // namespace dsc_keybus
}  // namespace esphome