# MQTT Entity Migration Guide

## ⚠️ IMPORTANT: Home Assistant YAML Configuration for MQTT Entities is Deprecated

Home Assistant has deprecated YAML configuration for MQTT entities (alarm_control_panel, sensor, binary_sensor, button). All MQTT entities must now be configured via:

1. **MQTT Discovery** (recommended - automatic)
2. **Home Assistant UI** (manual configuration)

## What Was Removed

The following MQTT entity configurations have been removed from the YAML files in this repository:

### From `HA_Yaml.YAML`:
- ❌ 1 alarm_control_panel (DSC 1500 Alarm Panel)
- ❌ 1 sensor (Security Partition 1)
- ❌ 8 binary_sensors (doors, motion, smoke, trouble, PGMs)
- ❌ 3 buttons (Fire, Aux, Panic alarms)

### From `HA_Yaml_Enhanced.YAML`:
- ❌ 1 enhanced alarm_control_panel with device info
- ❌ 4 diagnostic sensors (uptime, memory, WiFi, etc.)
- ❌ 11 enhanced binary_sensors with device classes
- ❌ 3 emergency buttons with safety warnings

### From `configuration.yaml.example`:
- ❌ All MQTT entity examples
- ✅ Kept MQTT broker configuration (still required)
- ✅ Kept automations (fully supported)
- ✅ Added template sensor/binary_sensor examples

## What Still Works in YAML

These configurations are **still fully supported** in YAML:

### ✅ Supported Platforms:
- `platform: template` (sensors, binary_sensors, etc.)
- `platform: time_date`
- `platform: integration`
- `platform: statistics`
- And other non-MQTT platforms

### ✅ Supported Sections:
- `mqtt:` (broker configuration only)
- `automation:`
- `script:`
- `scene:`
- `input_boolean:`
- `input_number:`
- `input_text:`
- `timer:`
- `counter:`

## Migration Options

### Option 1: ESPHome with MQTT Discovery (Recommended)

The **ESPHome configuration** in `extras/ESPHome/DscAlarm.yaml` already uses the correct approach:

```yaml
binary_sensor:
  - platform: template  # ✅ Still supported
    id: z1
    name: "Zone Front door"
    device_class: door
```

**Steps:**
1. Use the ESPHome configuration from `extras/ESPHome/DscAlarm.yaml`
2. Flash it to your ESP device
3. Enable MQTT Discovery in Home Assistant (usually automatic)
4. Entities will appear automatically in Home Assistant

### Option 2: Manual Configuration via Home Assistant UI

**Steps:**
1. Go to Settings > Devices & Services
2. Find the MQTT integration
3. Click "Configure" → "Add Entity"
4. Configure each entity manually using the settings below

### Option 3: MQTT Discovery Messages (Advanced)

Publish discovery config messages to topics like:
```
homeassistant/alarm_control_panel/dscPartition1/config
homeassistant/sensor/dscPartitionMessage1/config
homeassistant/binary_sensor/dscTrouble/config
```

## Entity Configuration Reference

Use these settings when manually configuring entities via the UI or discovery messages:

### Alarm Control Panel
```
Name: DSC 1500 Alarm Panel
Unique ID: dscPartition1
State Topic: dsc/Get/Partition1
Command Topic: dsc/Set
Availability Topic: dsc/Status
Code: [your DSC code]
```

### Sensors
```
Security Partition 1:
  Unique ID: dscPartitionMessage1
  State Topic: dsc/Get/Partition1/Message
  Availability Topic: dsc/Status
```

### Binary Sensors
```
Security Trouble:
  Unique ID: dscTrouble
  State Topic: dsc/Get/Trouble
  Device Class: problem
  Payload On: "1"
  Payload Off: "0"

Back Door:
  Unique ID: dscZone1  
  State Topic: dsc/Get/Zone1
  Device Class: door
  Payload On: "1"
  Payload Off: "0"

[Similar configuration for other zones...]
```

### Buttons (Use with Caution)
```
Fire Alarm:
  Unique ID: dscFire
  Command Topic: dsc/Set
  Payload Press: "f"
  
Aux Alarm:
  Unique ID: dscAux
  Command Topic: dsc/Set
  Payload Press: "a"
  
Panic Alarm:
  Unique ID: dscPanic
  Command Topic: dsc/Set
  Payload Press: "p"
```

## Template Examples (Still Supported in YAML)

You can still create template sensors and binary sensors in YAML that reference your MQTT entities:

```yaml
sensor:
  - platform: template
    sensors:
      dsc_system_status:
        friendly_name: "DSC System Status"
        value_template: >
          {% if is_state('alarm_control_panel.dsc_1500_alarm_panel', 'armed_away') %}
            Armed Away
          {% elif is_state('alarm_control_panel.dsc_1500_alarm_panel', 'disarmed') %}
            Disarmed
          {% else %}
            Unknown
          {% endif %}

binary_sensor:
  - platform: template
    sensors:
      any_door_open:
        friendly_name: "Any Door Open"
        value_template: >
          {{ is_state('binary_sensor.back_door', 'on') or 
             is_state('binary_sensor.front_door', 'on') }}
        device_class: door
```

## Next Steps

1. **Choose your migration path** (ESPHome recommended)
2. **Remove YAML entity configurations** from your Home Assistant config
3. **Keep MQTT broker settings** and automations in YAML
4. **Set up entities** using your chosen method
5. **Test the system** to ensure everything works
6. **Update any automations** that reference entity names if they changed

## Troubleshooting

### Entities Not Appearing
- Check MQTT broker connection
- Verify MQTT Discovery is enabled
- Check Home Assistant logs for errors
- Confirm topic names match exactly

### Entity Names Changed
- Update automations and scripts with new entity IDs
- Use the Developer Tools → States to find current entity names

### Need Help?
- Check Home Assistant MQTT Discovery documentation
- Review ESPHome MQTT component documentation  
- See repository documentation for more specific guidance

## Files Modified in This Update

- ✅ `HA_Yaml.YAML` - Removed MQTT entities, added migration guide
- ✅ `HA_Yaml_Enhanced.YAML` - Removed MQTT entities, kept MQTT broker config and automations  
- ✅ `configuration.yaml.example` - Updated with supported YAML examples only
- ✅ Created this migration guide

The ESPHome configuration in `extras/ESPHome/DscAlarm.yaml` was already correct and requires no changes.