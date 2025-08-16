"""Alarm Control Panel platform for DSC Keybus Interface"""
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

from . import DSCKeybusComponent, dsc_keybus_ns

DEPENDENCIES = ["dsc_keybus"]

# Define a simple alarm control panel class that doesn't depend on ESPHome's alarm_control_panel component
DSCAlarmControlPanel = dsc_keybus_ns.class_(
    "DSCAlarmControlPanel", cg.Component
)

# Configuration keys
CONF_PARTITION = "partition"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(DSCAlarmControlPanel),
    cv.GenerateID("dsc_keybus_id"): cv.use_id(DSCKeybusComponent),
    cv.Required(CONF_PARTITION): cv.int_range(min=1, max=8),
}).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    """Generate C++ code for the alarm control panel"""
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    # Get reference to the parent DSC keybus component
    parent = await cg.get_variable(config["dsc_keybus_id"])
    cg.add(var.set_dsc_keybus_component(parent))
    cg.add(var.set_partition(config[CONF_PARTITION]))