#pragma once

// Prevent DSC library from defining static variables - we handle them separately
#define DSC_STATIC_VARIABLES_DEFINED

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "esphome/core/automation.h"

#ifdef ESP8266
#define DSC_DEFAULT_CLOCK_PIN D1  // esp8266: D1, D2, D8 (GPIO 5, 4, 15)
#define DSC_DEFAULT_READ_PIN D2   // esp8266: D1, D2, D8 (GPIO 5, 4, 15)
#define DSC_DEFAULT_WRITE_PIN D8  // esp8266: D1, D2, D8 (GPIO 5, 4, 15)
#else
#define DSC_DEFAULT_CLOCK_PIN 18  // ESP32
#define DSC_DEFAULT_READ_PIN 19   // ESP32
#define DSC_DEFAULT_WRITE_PIN 21  // ESP32
#endif

// DSC Classic Series PC-16 pin (required for Classic series panels)
// For ESP8266: Use D5 (GPIO 14) with 1kΩ and 33kΩ resistors - see wiring guide
// For ESP32: Use GPIO 17 with same resistor configuration
#ifdef ESP8266
#define DSC_DEFAULT_PC16_PIN D5   // NodeMCU D5 = GPIO 14
#else
#define DSC_DEFAULT_PC16_PIN 17   // ESP32 GPIO 17
#endif

// ESP-IDF 5.3.2+ LoadProhibited crash prevention variables
#if defined(ESP32) || defined(ESP_PLATFORM)
// External declarations for ESP-IDF 5.3+ and static variable initialization
extern volatile bool dsc_static_variables_initialized;

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 3, 0)
extern volatile bool dsc_esp_idf_timer_system_ready;
extern volatile unsigned long dsc_esp_idf_init_delay_timestamp;
#endif

// Manual initialization function for fallback
extern void dsc_manual_static_variables_init();
#endif

namespace esphome {
namespace dsc_keybus {

// Forward declarations
class DSCKeybusComponent;

// Trigger classes for automation
class SystemStatusChangeTrigger : public Trigger<std::string> {
 public:
  SystemStatusChangeTrigger(DSCKeybusComponent *parent) : parent_(parent) {}

 protected:
  DSCKeybusComponent *parent_;
};

class PartitionStatusChangeTrigger : public Trigger<uint8_t, std::string> {
 public:
  PartitionStatusChangeTrigger(DSCKeybusComponent *parent) : parent_(parent) {}

 protected:
  DSCKeybusComponent *parent_;
};

class PartitionMsgChangeTrigger : public Trigger<uint8_t, std::string> {
 public:
  PartitionMsgChangeTrigger(DSCKeybusComponent *parent) : parent_(parent) {}

 protected:
  DSCKeybusComponent *parent_;
};

class TroubleStatusChangeTrigger : public Trigger<bool> {
 public:
  TroubleStatusChangeTrigger(DSCKeybusComponent *parent) : parent_(parent) {}

 protected:
  DSCKeybusComponent *parent_;
};

class FireStatusChangeTrigger : public Trigger<uint8_t, bool> {
 public:
  FireStatusChangeTrigger(DSCKeybusComponent *parent) : parent_(parent) {}

 protected:
  DSCKeybusComponent *parent_;
};

class ZoneStatusChangeTrigger : public Trigger<uint8_t, bool> {
 public:
  ZoneStatusChangeTrigger(DSCKeybusComponent *parent) : parent_(parent) {}

 protected:
  DSCKeybusComponent *parent_;
};

class ZoneAlarmChangeTrigger : public Trigger<uint8_t, bool> {
 public:
  ZoneAlarmChangeTrigger(DSCKeybusComponent *parent) : parent_(parent) {}

 protected:
  DSCKeybusComponent *parent_;
};

  class DSCKeybusComponent : public Component {
 public:
  DSCKeybusComponent() = default;

  // Configuration methods
  void set_access_code(const std::string &code) { this->access_code_ = code; }
  void set_debug_level(uint8_t level) { this->debug_level_ = level; }
  void set_enable_05_messages(bool enable) { this->enable_05_messages_ = enable; }
  void set_clock_pin(uint8_t pin) { this->clock_pin_ = pin; }
  void set_read_pin(uint8_t pin) { this->read_pin_ = pin; }
  void set_write_pin(uint8_t pin) { this->write_pin_ = pin; }
  void set_pc16_pin(uint8_t pin) { this->pc16_pin_ = pin; }

  // Component lifecycle
  void setup() override;
  void loop() override;
  void dump_config() override;

  // Service methods
  void set_alarm_state(uint8_t partition, const std::string &state, const std::string &code = "");
  void alarm_disarm(const std::string &code);
  void alarm_arm_home();
  void alarm_arm_night(const std::string &code = "");
  void alarm_arm_away();
  void alarm_trigger_panic();
  void alarm_trigger_fire();
  void alarm_keypress(const std::string &keys);
  void disconnect_keybus();

  // Status methods - wrapper for DSCWrapper methods to maintain YAML compatibility
  bool getKeybusConnected();
  bool getBufferOverflow();

  // Trigger registration methods
  void add_system_status_trigger(SystemStatusChangeTrigger *trigger) {
    this->system_status_triggers_.push_back(trigger);
  }
  void add_partition_status_trigger(PartitionStatusChangeTrigger *trigger) {
    this->partition_status_triggers_.push_back(trigger);
  }
  void add_partition_msg_trigger(PartitionMsgChangeTrigger *trigger) {
    this->partition_msg_triggers_.push_back(trigger);
  }
  void add_trouble_status_trigger(TroubleStatusChangeTrigger *trigger) {
    this->trouble_status_triggers_.push_back(trigger);
  }
  void add_fire_status_trigger(FireStatusChangeTrigger *trigger) {
    this->fire_status_triggers_.push_back(trigger);
  }
  void add_zone_status_trigger(ZoneStatusChangeTrigger *trigger) {
    this->zone_status_triggers_.push_back(trigger);
  }
  void add_zone_alarm_trigger(ZoneAlarmChangeTrigger *trigger) {
    this->zone_alarm_triggers_.push_back(trigger);
  }

  // Status constants
  static constexpr const char *STATUS_PENDING = "pending";
  static constexpr const char *STATUS_ARM = "armed_away";
  static constexpr const char *STATUS_STAY = "armed_home";
  static constexpr const char *STATUS_NIGHT = "armed_night";
  static constexpr const char *STATUS_OFF = "disarmed";
  static constexpr const char *STATUS_ONLINE = "online";
  static constexpr const char *STATUS_OFFLINE = "offline";
  static constexpr const char *STATUS_TRIGGERED = "triggered";
  static constexpr const char *STATUS_READY = "ready";
  static constexpr const char *STATUS_NOT_READY = "unavailable";
  static constexpr const char *MSG_ZONE_BYPASS = "zone_bypass_entered";
  static constexpr const char *MSG_ARMED_BYPASS = "armed_custom_bypass";
  static constexpr const char *MSG_NO_ENTRY_DELAY = "no_entry_delay";
  static constexpr const char *MSG_NONE = "no_messages";

 protected:
  std::string access_code_{""};
  uint8_t debug_level_{0};
  bool enable_05_messages_{true};
  bool force_disconnect_{false};
  
  // Pin configuration - use default pins if not specified
  uint8_t clock_pin_{DSC_DEFAULT_CLOCK_PIN};
  uint8_t read_pin_{DSC_DEFAULT_READ_PIN};
  uint8_t write_pin_{DSC_DEFAULT_WRITE_PIN};
  uint8_t pc16_pin_{DSC_DEFAULT_PC16_PIN};

  // Trigger lists
  std::vector<SystemStatusChangeTrigger *> system_status_triggers_;
  std::vector<PartitionStatusChangeTrigger *> partition_status_triggers_;
  std::vector<PartitionMsgChangeTrigger *> partition_msg_triggers_;
  std::vector<TroubleStatusChangeTrigger *> trouble_status_triggers_;
  std::vector<FireStatusChangeTrigger *> fire_status_triggers_;
  std::vector<ZoneStatusChangeTrigger *> zone_status_triggers_;
  std::vector<ZoneAlarmChangeTrigger *> zone_alarm_triggers_;

  // Internal state
  uint8_t zone_;
  uint8_t last_status_[8]{0}; // Support up to 8 partitions

  // Helper methods
  bool is_numeric_code(const std::string &code);
  const char* status_text(uint8_t status_code);
};

}  // namespace dsc_keybus
}  // namespace esphome