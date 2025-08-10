# DSC Alarm Control HA - Configuration Troubleshooting Guide

## Common MQTT Configuration Errors and Solutions

This guide addresses the most common configuration validation errors when setting up the DSC Alarm Control system with Home Assistant.

### Error: `'automation' is an invalid option for 'mqtt'`

**Problem:** This error occurs when the `automation:` section is incorrectly nested under the `mqtt:` section instead of being at the root level of your configuration.yaml.

**❌ Incorrect Structure:**
```yaml
mqtt:
  broker: 192.168.1.100
  port: 1883
  username: !secret mqtt_username
  password: !secret mqtt_password
  automation:  # ← This is WRONG! automation should not be under mqtt
    - alias: "DSC System Health Alert"
      trigger:
        - platform: state
          entity_id: binary_sensor.dsc_system_health
```

**✅ Correct Structure:**
```yaml
mqtt:
  broker: 192.168.1.100
  port: 1883
  username: !secret mqtt_username
  password: !secret mqtt_password

automation:  # ← This should be at the root level
  - alias: "DSC System Health Alert"
    trigger:
      - platform: state
        entity_id: binary_sensor.dsc_system_health
```

### Error: `'mqtt' is an invalid option for 'mqtt'`

**Problem:** This error occurs when there's a duplicate `mqtt:` section nested under another `mqtt:` section.

**❌ Incorrect Structure:**
```yaml
mqtt:
  broker: 192.168.1.100
  port: 1883
  mqtt:  # ← This is WRONG! Duplicate mqtt key
    discovery: true
    client_id: homeassistant_dsc
```

**✅ Correct Structure:**
```yaml
mqtt:
  broker: 192.168.1.100
  port: 1883
  discovery: true
  client_id: homeassistant_dsc
```

## Complete Correct Configuration Structure

Here's the proper way to structure your Home Assistant configuration.yaml for the DSC Alarm system:

```yaml
# Home Assistant Configuration for DSC Alarm System

# Default Home Assistant configuration
default_config:

# MQTT Configuration - TOP LEVEL
mqtt:
  broker: !secret mqtt_broker_ip
  port: 1883
  username: !secret mqtt_username
  password: !secret mqtt_password
  client_id: homeassistant_dsc
  keepalive: 60
  discovery: true
  birth_message:
    topic: "homeassistant/status"
    payload: "online"
  will_message:
    topic: "homeassistant/status" 
    payload: "offline"

# Alarm Control Panel - TOP LEVEL
alarm_control_panel:
  - name: "DSC 1500 Alarm Panel"
    unique_id: dsc_partition_1
    state_topic: "dsc/Get/Partition1"
    command_topic: "dsc/Set"
    code: !secret dsc_alarm_code
    # ... additional configuration

# Sensors - TOP LEVEL
sensor:
  - name: "Security Partition 1 Status"
    unique_id: dsc_partition_1_status
    state_topic: "dsc/Get/Partition1/Message"
    # ... additional configuration

# Binary Sensors - TOP LEVEL
binary_sensor:
  - name: "Security System Trouble"
    unique_id: dsc_trouble
    state_topic: "dsc/Get/Trouble"
    # ... additional configuration

# Automations - TOP LEVEL
automation:
  - alias: "DSC System Health Alert"
    trigger:
      - platform: state
        entity_id: binary_sensor.dsc_system_health
        to: "on"
    action:
      - service: notify.persistent_notification
        # ... additional configuration
```

## Step-by-Step Fix Instructions

1. **Open your configuration.yaml file**
   - Locate your Home Assistant configuration directory
   - Open the `configuration.yaml` file in a text editor

2. **Check the current structure**
   - Look for any sections nested under `mqtt:` that shouldn't be there
   - Common misplaced sections: `automation`, `alarm_control_panel`, `sensor`, `binary_sensor`

3. **Move misplaced sections to root level**
   - Cut any incorrectly nested sections
   - Paste them at the root level of the file (same indentation as `mqtt:`)

4. **Remove duplicate sections**
   - Check for duplicate `mqtt:` keys
   - Merge any duplicate configurations

5. **Validate your configuration**
   - Run the configuration checker: `hass --script check_config`
   - Or use the DSC validator: `python3 validate_config.py`

## Configuration Validation Commands

### Using Home Assistant Built-in Checker
```bash
# From your Home Assistant directory
hass --script check_config
```

### Using DSC-Specific Validator
```bash
# From the DSC Alarm repository
python3 validate_config.py
```

### Using Python YAML Parser
```bash
# Quick syntax check
python3 -c "import yaml; yaml.safe_load(open('configuration.yaml'))"
```

## Common Mistakes to Avoid

### ❌ Don't Do This:
```yaml
mqtt:
  broker: 192.168.1.100
  automation:        # Wrong - should be at root level
    - alias: ...
  alarm_control_panel:  # Wrong - should be at root level  
    - name: ...
  mqtt:              # Wrong - duplicate key
    discovery: true
```

### ✅ Do This Instead:
```yaml
mqtt:
  broker: 192.168.1.100
  discovery: true

automation:          # Correct - at root level
  - alias: ...

alarm_control_panel: # Correct - at root level
  - name: ...
```

## Additional Help

### File Templates
- Use `configuration.yaml.example` as a complete working template
- Copy relevant sections from `HA_Yaml_Enhanced.YAML` for advanced features
- Ensure proper indentation (2 spaces per level, no tabs)

### Secrets Management
Create a `secrets.yaml` file for sensitive information:
```yaml
# secrets.yaml
mqtt_broker_ip: "192.168.1.100"
mqtt_username: "your_mqtt_user"
mqtt_password: "your_mqtt_password"
dsc_alarm_code: "1234"
esp32_ip: "192.168.1.200"
```

### Testing Changes
1. Always backup your configuration before making changes
2. Test configuration syntax before restarting Home Assistant
3. Check Home Assistant logs for any remaining errors
4. Verify MQTT connectivity and DSC device communication

## Getting Help
If you continue to experience issues:
1. Run `python3 validate_config.py` for detailed diagnostics
2. Check the Home Assistant logs for specific error messages
3. Verify your MQTT broker is running and accessible
4. Ensure your DSC ESP32 device is connected and publishing data

## Related Documentation
- [MQTT Integration Documentation](https://www.home-assistant.io/integrations/mqtt/)
- [Home Assistant Configuration Validation](https://www.home-assistant.io/docs/configuration/)
- [YAML Syntax Guide](https://yaml.org/spec/1.2/spec.html)