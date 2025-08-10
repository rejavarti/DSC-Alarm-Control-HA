# MQTT Configuration Structure Guide

## ❌ INCORRECT Structure (will cause errors)

```yaml
# DON'T DO THIS - This will cause configuration errors
mqtt:
  - broker: !secret mqtt_broker_ip
    username: !secret mqtt_username
    password: !secret mqtt_password
    # ERROR: These sections should NOT be nested under mqtt
    alarm_control_panel:
      - name: "DSC 1500 Alarm Panel"
    sensor:
      - name: "Security Partition 1"
    automation:
      - alias: "Some automation"
```

This structure will result in errors like:
```
Invalid config for 'mqtt' at configuration.yaml, line XX: 'automation' is an invalid option for 'mqtt'
Invalid config for 'mqtt' at configuration.yaml, line XX: 'alarm_control_panel' is an invalid option for 'mqtt'
```

## ✅ CORRECT Structure 

```yaml
# MQTT broker configuration (top-level)
mqtt:
  broker: !secret mqtt_broker_ip
  port: 1883
  username: !secret mqtt_username
  password: !secret mqtt_password
  client_id: homeassistant_dsc
  discovery: true

# MQTT devices configuration (separate top-level sections)
alarm_control_panel:
  - platform: mqtt  # This tells HA to use MQTT integration
    name: "DSC 1500 Alarm Panel"
    state_topic: "dsc/Get/Partition1"
    # ... other alarm config

sensor:
  - platform: mqtt  # This tells HA to use MQTT integration  
    name: "Security Partition 1"
    state_topic: "dsc/Get/Partition1/Message"
    # ... other sensor config

binary_sensor:
  - platform: mqtt  # This tells HA to use MQTT integration
    name: "Security Trouble"
    state_topic: "dsc/Get/Trouble"
    # ... other binary sensor config

button:
  - platform: mqtt  # This tells HA to use MQTT integration
    name: "Fire Alarm"
    command_topic: "dsc/Set"
    # ... other button config

# Automations (separate top-level section)
automation:
  - alias: "DSC System Health Alert"
    trigger:
      platform: state
    # ... automation config
```

## Key Points

1. **MQTT broker settings** go in the `mqtt:` section
2. **MQTT devices** go in their respective sections (`alarm_control_panel:`, `sensor:`, etc.) 
3. Each MQTT device should specify `platform: mqtt` (though this is often implicit)
4. **Never nest device sections under the `mqtt:` section**
5. Each section should be at the **top level** of your configuration.yaml