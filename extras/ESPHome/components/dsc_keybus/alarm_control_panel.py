"""Alarm Control Panel platform for DSC Keybus Interface"""
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import alarm_control_panel
from esphome.const import CONF_ID

from . import DSCKeybusComponent, dsc_keybus_ns

DEPENDENCIES = ["dsc_keybus"]

# Define the alarm control panel class
DSCAlarmControlPanel = dsc_keybus_ns.class_(
    "DSCAlarmControlPanel", alarm_control_panel.AlarmControlPanel, cg.Component
)

# Configuration keys
CONF_PARTITION = "partition"

CONFIG_SCHEMA = alarm_control_panel.alarm_control_panel_schema(
    DSCAlarmControlPanel
).extend(
    {
        cv.GenerateID("dsc_keybus_id"): cv.use_id(DSCKeybusComponent),
        cv.Required(CONF_PARTITION): cv.int_range(min=1, max=8),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    """Generate C++ code for the alarm control panel"""
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await alarm_control_panel.register_alarm_control_panel(var, config)

    # Get reference to the parent DSC keybus component
    parent = await cg.get_variable(config["dsc_keybus_id"])
    cg.add(var.set_dsc_keybus_component(parent))
    cg.add(var.set_partition(config[CONF_PARTITION]))