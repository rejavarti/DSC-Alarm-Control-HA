"""DSC Keybus Interface Component for ESPHome"""
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.const import (
    CONF_ID,
    CONF_TRIGGER_ID,
)

# Define the component namespace and classes
dsc_keybus_minimal_ns = cg.esphome_ns.namespace("dsc_keybus")
DSCKeybusComponent = dsc_keybus_minimal_ns.class_("DSCKeybusComponent", cg.Component)

# Define trigger classes
SystemStatusChangeTrigger = dsc_keybus_minimal_ns.class_(
    "SystemStatusChangeTrigger", automation.Trigger.template(cg.std_string)
)
PartitionStatusChangeTrigger = dsc_keybus_minimal_ns.class_(
    "PartitionStatusChangeTrigger", automation.Trigger.template(cg.uint8, cg.std_string)
)
PartitionMsgChangeTrigger = dsc_keybus_minimal_ns.class_(
    "PartitionMsgChangeTrigger", automation.Trigger.template(cg.uint8, cg.std_string)
)
TroubleStatusChangeTrigger = dsc_keybus_minimal_ns.class_(
    "TroubleStatusChangeTrigger", automation.Trigger.template(cg.bool_)
)
FireStatusChangeTrigger = dsc_keybus_minimal_ns.class_(
    "FireStatusChangeTrigger", automation.Trigger.template(cg.uint8, cg.bool_)
)
ZoneStatusChangeTrigger = dsc_keybus_minimal_ns.class_(
    "ZoneStatusChangeTrigger", automation.Trigger.template(cg.uint8, cg.bool_)
)
ZoneAlarmChangeTrigger = dsc_keybus_minimal_ns.class_(
    "ZoneAlarmChangeTrigger", automation.Trigger.template(cg.uint8, cg.bool_)
)

# Configuration keys
CONF_SERIES_TYPE = "series_type"
CONF_PC16_PIN = "pc16_pin"
CONF_CLOCK_PIN = "clock_pin"
CONF_READ_PIN = "read_pin"
CONF_WRITE_PIN = "write_pin"
CONF_STANDALONE_MODE = "standalone_mode"
CONF_ON_SYSTEM_STATUS_CHANGE = "on_system_status_change"
CONF_ON_PARTITION_STATUS_CHANGE = "on_partition_status_change"
CONF_ON_PARTITION_MSG_CHANGE = "on_partition_msg_change"
CONF_ON_TROUBLE_STATUS_CHANGE = "on_trouble_status_change"
CONF_ON_FIRE_STATUS_CHANGE = "on_fire_status_change"
CONF_ON_ZONE_STATUS_CHANGE = "on_zone_status_change"
CONF_ON_ZONE_ALARM_CHANGE = "on_zone_alarm_change"

# Component configuration schema
CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(DSCKeybusComponent),
    cv.Optional("access_code", default=""): cv.string,
    cv.Optional("debug", default=0): cv.int_range(0, 3),
    cv.Optional("enable_05_messages", default=True): cv.boolean,
    cv.Optional(CONF_SERIES_TYPE, default="Classic"): cv.one_of("PowerSeries", "Classic"),
    cv.Optional(CONF_PC16_PIN): cv.int_range(0, 40),  # Pin for Classic series PC-16 line
    cv.Optional(CONF_CLOCK_PIN): cv.int_range(0, 40),  # DSC Clock pin
    cv.Optional(CONF_READ_PIN): cv.int_range(0, 40),   # DSC Data read pin  
    cv.Optional(CONF_WRITE_PIN): cv.int_range(0, 40),  # DSC Data write pin
    cv.Optional(CONF_STANDALONE_MODE, default=False): cv.boolean,  # Enable standalone mode for testing without panel
    cv.Optional(CONF_ON_SYSTEM_STATUS_CHANGE): automation.validate_automation(
        {
            cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(SystemStatusChangeTrigger),
        }
    ),
    cv.Optional(CONF_ON_PARTITION_STATUS_CHANGE): automation.validate_automation(
        {
            cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(PartitionStatusChangeTrigger),
        }
    ),
    cv.Optional(CONF_ON_PARTITION_MSG_CHANGE): automation.validate_automation(
        {
            cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(PartitionMsgChangeTrigger),
        }
    ),
    cv.Optional(CONF_ON_TROUBLE_STATUS_CHANGE): automation.validate_automation(
        {
            cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(TroubleStatusChangeTrigger),
        }
    ),
    cv.Optional(CONF_ON_FIRE_STATUS_CHANGE): automation.validate_automation(
        {
            cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(FireStatusChangeTrigger),
        }
    ),
    cv.Optional(CONF_ON_ZONE_STATUS_CHANGE): automation.validate_automation(
        {
            cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(ZoneStatusChangeTrigger),
        }
    ),
    cv.Optional(CONF_ON_ZONE_ALARM_CHANGE): automation.validate_automation(
        {
            cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(ZoneAlarmChangeTrigger),
        }
    ),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    """Generate C++ code for the component"""
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    # Set configuration parameters
    cg.add(var.set_access_code(config["access_code"]))
    cg.add(var.set_debug_level(config["debug"]))
    cg.add(var.set_enable_05_messages(config["enable_05_messages"]))
    cg.add(var.set_standalone_mode(config[CONF_STANDALONE_MODE]))
    
    # Set pin configurations if specified
    if CONF_CLOCK_PIN in config:
        cg.add(var.set_clock_pin(config[CONF_CLOCK_PIN]))
    if CONF_READ_PIN in config:
        cg.add(var.set_read_pin(config[CONF_READ_PIN]))
    if CONF_WRITE_PIN in config:
        cg.add(var.set_write_pin(config[CONF_WRITE_PIN]))
    if CONF_PC16_PIN in config:
        cg.add(var.set_pc16_pin(config[CONF_PC16_PIN]))
    
    # Set series type and enable appropriate defines
    series_type = config[CONF_SERIES_TYPE]
    if series_type.upper() == "CLASSIC":
        cg.add_define("dscClassicSeries")
        if CONF_PC16_PIN in config:
            cg.add_define("DSC_CLASSIC_PC16_PIN", config[CONF_PC16_PIN])
    else:  # PowerSeries
        cg.add_define("dscPowerSeries")
    
    # Prevent multiple definition of static variables - critical for ESPHome builds
    cg.add_define("DSC_STATIC_VARIABLES_DEFINED")
    
    # CRITICAL: Enable LoadProhibited crash prevention flags automatically
    # These flags activate the comprehensive fix for ESP32 0xcececece pattern crashes
    cg.add_define("DSC_LOADPROHIBITED_CRASH_FIX")        # Main crash fix flag
    cg.add_define("DSC_ESP_IDF_5_3_PLUS")                # ESP-IDF 5.3+ optimizations  
    cg.add_define("DSC_ESP_IDF_5_3_PLUS_COMPONENT")      # Component-specific ESP-IDF 5.3+ flag
    cg.add_define("DSC_ENHANCED_MEMORY_SAFETY")          # Enhanced memory safety checks
    cg.add_define("DSC_TIMER_MODE_ESP_IDF")              # Use ESP-IDF timer mode for compatibility
    
    # Set up triggers
    for conf in config.get(CONF_ON_SYSTEM_STATUS_CHANGE, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        cg.add(var.add_system_status_trigger(trigger))
        await automation.build_automation(trigger, [(cg.std_string, "status")], conf)
    
    for conf in config.get(CONF_ON_PARTITION_STATUS_CHANGE, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        cg.add(var.add_partition_status_trigger(trigger))
        await automation.build_automation(trigger, [(cg.uint8, "partition"), (cg.std_string, "status")], conf)
    
    for conf in config.get(CONF_ON_PARTITION_MSG_CHANGE, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        cg.add(var.add_partition_msg_trigger(trigger))
        await automation.build_automation(trigger, [(cg.uint8, "partition"), (cg.std_string, "msg")], conf)
    
    for conf in config.get(CONF_ON_TROUBLE_STATUS_CHANGE, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        cg.add(var.add_trouble_status_trigger(trigger))
        await automation.build_automation(trigger, [(cg.bool_, "trouble")], conf)
    
    for conf in config.get(CONF_ON_FIRE_STATUS_CHANGE, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        cg.add(var.add_fire_status_trigger(trigger))
        await automation.build_automation(trigger, [(cg.uint8, "partition"), (cg.bool_, "fire")], conf)
    
    for conf in config.get(CONF_ON_ZONE_STATUS_CHANGE, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        cg.add(var.add_zone_status_trigger(trigger))
        await automation.build_automation(trigger, [(cg.uint8, "zone"), (cg.bool_, "open")], conf)
    
    for conf in config.get(CONF_ON_ZONE_ALARM_CHANGE, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        cg.add(var.add_zone_alarm_trigger(trigger))
        await automation.build_automation(trigger, [(cg.uint8, "zone"), (cg.bool_, "open")], conf)