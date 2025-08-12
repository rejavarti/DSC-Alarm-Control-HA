#include "dsc_keybus.h"
#include "esphome/core/log.h"

// Use the minimal DSC Keybus Interface for ESPHome
#include "dscKeybusInterface_minimal.h"

namespace esphome {
namespace dsc_keybus {

static const char *const TAG = "dsc_keybus";

// Global DSC interface instance using minimal interface
dscKeybusInterfaceMinimal dsc(DSC_DEFAULT_CLOCK_PIN, DSC_DEFAULT_READ_PIN, DSC_DEFAULT_WRITE_PIN);

void DSCKeybusComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up DSC Keybus Interface...");
  
  // TODO: Register API services (requires USE_API_SERVICES)
  // this->register_service(&DSCKeybusComponent::set_alarm_state, "set_alarm_state", {"partition", "state", "code"});
  // this->register_service(&DSCKeybusComponent::alarm_disarm, "alarm_disarm", {"code"});
  // this->register_service(&DSCKeybusComponent::alarm_arm_home, "alarm_arm_home");
  // this->register_service(&DSCKeybusComponent::alarm_arm_night, "alarm_arm_night", {"code"});
  // this->register_service(&DSCKeybusComponent::alarm_arm_away, "alarm_arm_away");
  // this->register_service(&DSCKeybusComponent::alarm_trigger_panic, "alarm_trigger_panic");
  // this->register_service(&DSCKeybusComponent::alarm_trigger_fire, "alarm_trigger_fire");
  // this->register_service(&DSCKeybusComponent::alarm_keypress, "alarm_keypress", {"keys"});
  
  // Initialize system state
  for (auto *trigger : this->system_status_triggers_) {
    trigger->trigger(STATUS_OFFLINE);
  }
  
  this->force_disconnect_ = false;
  dsc.resetStatus();
  dsc.begin();
  
  ESP_LOGCONFIG(TAG, "DSC Keybus Interface setup complete");
}

void DSCKeybusComponent::loop() {
  if (!this->force_disconnect_ && dsc.loop()) {
    if (this->debug_level_ == 1 && (dsc.panelData[0] == 0x05 || dsc.panelData[0] == 0x27)) {
      ESP_LOGD(TAG, "Panel data: %02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X",
               dsc.panelData[0], dsc.panelData[1], dsc.panelData[2], dsc.panelData[3],
               dsc.panelData[4], dsc.panelData[5], dsc.panelData[6], dsc.panelData[7],
               dsc.panelData[8], dsc.panelData[9], dsc.panelData[10], dsc.panelData[11]);
    }
    
    if (this->debug_level_ > 2) {
      ESP_LOGD(TAG, "Panel data: %02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X",
               dsc.panelData[0], dsc.panelData[1], dsc.panelData[2], dsc.panelData[3],
               dsc.panelData[4], dsc.panelData[5], dsc.panelData[6], dsc.panelData[7],
               dsc.panelData[8], dsc.panelData[9], dsc.panelData[10], dsc.panelData[11]);
    }
  }

  if (dsc.statusChanged) {
    dsc.statusChanged = false;
    
    // Handle buffer overflow
    if (dsc.bufferOverflow) {
      ESP_LOGE(TAG, "Keybus buffer overflow");
      dsc.bufferOverflow = false;
    }

    // Handle keybus connection changes
    if (dsc.keybusChanged) {
      dsc.keybusChanged = false;
      for (auto *trigger : this->system_status_triggers_) {
        if (dsc.keybusConnected) {
          trigger->trigger(STATUS_ONLINE);
        } else {
          trigger->trigger(STATUS_OFFLINE);
        }
      }
    }

    // Handle access code prompt
    if (dsc.accessCodePrompt && dsc.writeReady && this->is_numeric_code(this->access_code_)) {
      dsc.accessCodePrompt = false;
      dsc.write(this->access_code_.c_str());
      if (this->debug_level_ > 0) {
        ESP_LOGD(TAG, "Access code prompt handled");
      }
    }

    // Handle power and battery troubles
    if (dsc.powerChanged && this->enable_05_messages_) {
      dsc.powerChanged = false;
      if (dsc.powerTrouble) {
        for (auto *trigger : this->partition_msg_triggers_) {
          trigger->trigger(1, "AC power failure");
        }
      }
    }
    
    if (dsc.batteryChanged && this->enable_05_messages_) {
      dsc.batteryChanged = false;
      if (dsc.batteryTrouble) {
        for (auto *trigger : this->partition_msg_triggers_) {
          trigger->trigger(1, "Battery trouble");
        }
      }
    }

    // Handle keypad alarms
    if (dsc.keypadFireAlarm && this->enable_05_messages_) {
      dsc.keypadFireAlarm = false;
      for (auto *trigger : this->partition_msg_triggers_) {
        trigger->trigger(1, "Keypad Fire Alarm");
      }
    }
    
    if (dsc.keypadPanicAlarm && this->enable_05_messages_) {
      dsc.keypadPanicAlarm = false;
      for (auto *trigger : this->partition_msg_triggers_) {
        trigger->trigger(1, "Keypad Panic Alarm");
      }
    }

    // Handle trouble status
    if (dsc.troubleChanged) {
      dsc.troubleChanged = false;
      for (auto *trigger : this->trouble_status_triggers_) {
        trigger->trigger(dsc.trouble);
      }
    }

    if (this->debug_level_ > 0) {
      ESP_LOGD(TAG, "Panel command data: %02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X",
               dsc.panelData[0], dsc.panelData[1], dsc.panelData[2], dsc.panelData[3],
               dsc.panelData[4], dsc.panelData[5], dsc.panelData[6], dsc.panelData[7],
               dsc.panelData[8], dsc.panelData[9]);
    }

    // Process partition status changes
    for (uint8_t partition = 0; partition < dscPartitions; partition++) {
      if (dsc.disabled[partition]) continue;

      if (this->debug_level_ > 0) {
        ESP_LOGD(TAG, "Partition %d data: %02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X",
                 partition, dsc.lights[partition], dsc.status[partition], dsc.armed[partition],
                 dsc.armedAway[partition], dsc.armedStay[partition], dsc.noEntryDelay[partition],
                 dsc.fire[partition], dsc.armedChanged[partition], dsc.exitDelay[partition],
                 dsc.readyChanged[partition], dsc.ready[partition], dsc.alarmChanged[partition],
                 dsc.alarm[partition]);
      }

      // Handle status changes
      if (this->last_status_[partition] != dsc.status[partition]) {
        this->last_status_[partition] = dsc.status[partition];
        if (this->enable_05_messages_) {
          char msg[50];
          snprintf(msg, sizeof(msg), "%02X: %s", dsc.status[partition], this->status_text(dsc.status[partition]));
          for (auto *trigger : this->partition_msg_triggers_) {
            trigger->trigger(partition + 1, msg);
          }
        }
      }

      // Handle alarm status changes
      if (dsc.alarmChanged[partition]) {
        dsc.alarmChanged[partition] = false;
        if (dsc.alarm[partition]) {
          dsc.readyChanged[partition] = false;
          dsc.armedChanged[partition] = false;
          for (auto *trigger : this->partition_status_triggers_) {
            trigger->trigger(partition + 1, STATUS_TRIGGERED);
          }
        }
      }

      // Handle armed/disarmed status changes
      if (dsc.armedChanged[partition]) {
        dsc.armedChanged[partition] = false;
        std::string status;
        if (dsc.armed[partition]) {
          if ((dsc.armedAway[partition] || dsc.armedStay[partition]) && dsc.noEntryDelay[partition]) {
            status = STATUS_NIGHT;
          } else if (dsc.armedStay[partition]) {
            status = STATUS_STAY;
          } else {
            status = STATUS_ARM;
          }
        } else {
          status = STATUS_OFF;
        }
        for (auto *trigger : this->partition_status_triggers_) {
          trigger->trigger(partition + 1, status);
        }
      }

      // Handle exit delay changes
      if (dsc.exitDelayChanged[partition]) {
        dsc.exitDelayChanged[partition] = false;
        std::string status;
        if (dsc.exitDelay[partition]) {
          status = STATUS_PENDING;
        } else if (!dsc.exitDelay[partition] && !dsc.armed[partition]) {
          status = STATUS_OFF;
        } else if (!dsc.exitDelay[partition] && dsc.armed[partition]) {
          if ((dsc.armedAway[partition] || dsc.armedStay[partition]) && dsc.noEntryDelay[partition]) {
            status = STATUS_NIGHT;
          } else if (dsc.armedStay[partition]) {
            status = STATUS_STAY;
          } else {
            status = STATUS_ARM;
          }
        }
        for (auto *trigger : this->partition_status_triggers_) {
          trigger->trigger(partition + 1, status);
        }
      }

      // Handle ready status changes
      if (dsc.readyChanged[partition]) {
        dsc.readyChanged[partition] = false;
        std::string status;
        if (dsc.ready[partition]) {
          status = STATUS_OFF;
        } else if (!dsc.armed[partition]) {
          status = STATUS_NOT_READY;
        }
        for (auto *trigger : this->partition_status_triggers_) {
          trigger->trigger(partition + 1, status);
        }
      }

      // Handle fire alarm status changes
      if (dsc.fireChanged[partition]) {
        dsc.fireChanged[partition] = false;
        for (auto *trigger : this->fire_status_triggers_) {
          trigger->trigger(partition + 1, dsc.fire[partition]);
        }
      }
    }

    // Handle zone status changes
    if (dsc.openZonesStatusChanged) {
      dsc.openZonesStatusChanged = false;
      for (uint8_t zone_group = 0; zone_group < dscZones; zone_group++) {
        for (uint8_t zone_bit = 0; zone_bit < 8; zone_bit++) {
          if (bitRead(dsc.openZonesChanged[zone_group], zone_bit)) {
            bitWrite(dsc.openZonesChanged[zone_group], zone_bit, 0);
            uint8_t zone = zone_bit + 1 + (zone_group * 8);
            bool is_open = bitRead(dsc.openZones[zone_group], zone_bit);
            for (auto *trigger : this->zone_status_triggers_) {
              trigger->trigger(zone, is_open);
            }
          }
        }
      }
    }

    // Handle zone alarm status changes
    if (dsc.alarmZonesStatusChanged) {
      dsc.alarmZonesStatusChanged = false;
      for (uint8_t zone_group = 0; zone_group < dscZones; zone_group++) {
        for (uint8_t zone_bit = 0; zone_bit < 8; zone_bit++) {
          if (bitRead(dsc.alarmZonesChanged[zone_group], zone_bit)) {
            bitWrite(dsc.alarmZonesChanged[zone_group], zone_bit, 0);
            uint8_t zone = zone_bit + 1 + (zone_group * 8);
            bool is_alarm = bitRead(dsc.alarmZones[zone_group], zone_bit);
            for (auto *trigger : this->zone_alarm_triggers_) {
              trigger->trigger(zone, is_alarm);
            }
          }
        }
      }
    }
  }
}

void DSCKeybusComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "DSC Keybus Interface:");
  ESP_LOGCONFIG(TAG, "  Debug Level: %d", this->debug_level_);
  ESP_LOGCONFIG(TAG, "  Enable 05 Messages: %s", this->enable_05_messages_ ? "YES" : "NO");
  ESP_LOGCONFIG(TAG, "  Access Code Set: %s", this->access_code_.empty() ? "NO" : "YES");
#if defined(dscClassicSeries)
  ESP_LOGCONFIG(TAG, "  DSC Series: Classic");
#else
  ESP_LOGCONFIG(TAG, "  DSC Series: PowerSeries");
#endif
}

// Service implementations
void DSCKeybusComponent::set_alarm_state(uint8_t partition, const std::string &state, const std::string &code) {
  std::string alarm_code = code;
  if (code.length() != 4 || !this->is_numeric_code(code)) {
    alarm_code = "";
  }
  
  if (partition > 0) partition = partition - 1; // Adjust to 0-based range

  // Arm stay
  if (state == "S" && !dsc.armed[partition] && !dsc.exitDelay[partition]) {
    dsc.writePartition = partition + 1;
    dsc.write('s');
  }
  // Arm away
  else if (state == "A" && !dsc.armed[partition] && !dsc.exitDelay[partition]) {
    dsc.writePartition = partition + 1;
    dsc.write('w');
  }
  // Arm night
  else if (state == "N" && !dsc.armed[partition] && !dsc.exitDelay[partition]) {
    dsc.writePartition = partition + 1;
    dsc.write('n');
    if (alarm_code.length() == 4 && !this->is_numeric_code(this->access_code_)) {
      dsc.write(alarm_code.c_str());
    }
  }
  // Fire command
  else if (state == "F") {
    dsc.writePartition = partition + 1;
    dsc.write('f');
  }
  // Panic command
  else if (state == "P") {
    dsc.writePartition = partition + 1;
    dsc.write('p');
  }
  // Disarm
  else if (state == "D" && (dsc.armed[partition] || dsc.exitDelay[partition])) {
    dsc.writePartition = partition + 1;
    if (alarm_code.length() == 4) {
      dsc.write(alarm_code.c_str());
    }
  }
}

void DSCKeybusComponent::alarm_disarm(const std::string &code) {
  this->set_alarm_state(1, "D", code);
}

void DSCKeybusComponent::alarm_arm_home() {
  this->set_alarm_state(1, "S");
}

void DSCKeybusComponent::alarm_arm_night(const std::string &code) {
  this->set_alarm_state(1, "N", code);
}

void DSCKeybusComponent::alarm_arm_away() {
  this->set_alarm_state(1, "A");
}

void DSCKeybusComponent::alarm_trigger_fire() {
  this->set_alarm_state(1, "F");
}

void DSCKeybusComponent::alarm_trigger_panic() {
  this->set_alarm_state(1, "P");
}

void DSCKeybusComponent::alarm_keypress(const std::string &keys) {
  if (this->debug_level_ > 0) {
    ESP_LOGD(TAG, "Writing keys: %s", keys.c_str());
  }
  dsc.write(keys.c_str());
}

void DSCKeybusComponent::disconnect_keybus() {
  dsc.stop();
  dsc.keybusConnected = false;
  dsc.statusChanged = false;
  this->force_disconnect_ = true;
}

// Helper methods
bool DSCKeybusComponent::is_numeric_code(const std::string &code) {
  if (code.empty() || isspace(code[0])) return false;
  char *end_ptr;
  strtol(code.c_str(), &end_ptr, 10);
  return (*end_ptr == 0);
}

const char* DSCKeybusComponent::status_text(uint8_t status_code) {
  switch (status_code) {
    case 0x01: return "Ready";
    case 0x02: return "Stay zones open";
    case 0x03: return "Zones open";
    case 0x04: return "Armed stay";
    case 0x05: return "Armed away";
    case 0x06: return "No entry delay";
    case 0x07: return "Failed to arm";
    case 0x08: return "Exit delay";
    case 0x09: return "No entry delay";
    case 0x0B: return "Quick exit";
    case 0x0C: return "Entry delay";
    case 0x0D: return "Alarm memory";
    case 0x10: return "Keypad lockout";
    case 0x11: return "Alarm";
    case 0x14: return "Auto-arm";
    case 0x15: return "Arm with bypass";
    case 0x16: return "No entry delay";
    case 0x17: return "Power failure";
    case 0x22: return "Alarm memory";
    case 0x33: return "Busy";
    case 0x3D: return "Disarmed";
    case 0x3E: return "Disarmed";
    case 0x40: return "Keypad blanked";
    case 0x8A: return "Activate zones";
    case 0x8B: return "Quick exit";
    case 0x8E: return "Invalid option";
    case 0x8F: return "Invalid code";
    case 0x9E: return "Enter * code";
    case 0x9F: return "Access code";
    case 0xA0: return "Zone bypass";
    case 0xA1: return "Trouble menu";
    case 0xA2: return "Alarm memory";
    case 0xA3: return "Door chime on";
    case 0xA4: return "Door chime off";
    case 0xA5: return "Master code";
    case 0xA6: return "Access codes";
    case 0xA7: return "Enter new code";
    case 0xA9: return "User function";
    case 0xAA: return "Time and Date";
    case 0xAB: return "Auto-arm time";
    case 0xAC: return "Auto-arm on";
    case 0xAD: return "Auto-arm off";
    case 0xAF: return "System test";
    case 0xB0: return "Enable DLS";
    case 0xB2: return "Command output";
    case 0xB7: return "Installer code";
    case 0xB8: return "Enter * code";
    case 0xB9: return "Zone tamper";
    case 0xBA: return "Zones low batt.";
    case 0xC6: return "Zone fault menu";
    case 0xC8: return "Service required";
    case 0xD0: return "Keypads low batt";
    case 0xD1: return "Wireless low bat";
    case 0xE4: return "Installer menu";
    case 0xE5: return "Keypad slot";
    case 0xE6: return "Input: 2 digits";
    case 0xE7: return "Input: 3 digits";
    case 0xE8: return "Input: 4 digits";
    case 0xEA: return "Code: 2 digits";
    case 0xEB: return "Code: 4 digits";
    case 0xEC: return "Input: 6 digits";
    case 0xED: return "Input: 32 digits";
    case 0xEE: return "Input: option";
    case 0xF0: return "Function key 1";
    case 0xF1: return "Function key 2";
    case 0xF2: return "Function key 3";
    case 0xF3: return "Function key 4";
    case 0xF4: return "Function key 5";
    case 0xF8: return "Keypad program";
    case 0xFF: return "Disabled";
    default: return "Unknown";
  }
}

}  // namespace dsc_keybus
}  // namespace esphome