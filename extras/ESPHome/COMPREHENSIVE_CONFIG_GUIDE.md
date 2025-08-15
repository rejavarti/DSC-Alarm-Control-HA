# DSC Alarm Comprehensive Configuration Guide

## Overview

The `DSCAlarm_Comprehensive_All_Fixes.yaml` configuration incorporates ALL fixes and features from the repository into a single, production-ready configuration file.

## What's Included

✅ **ESP-IDF 5.3.2 LoadProhibited Fix** - Complete crash prevention  
✅ **Memory Allocation Failure Prevention** - Heap optimization for ESP-IDF 5.3+  
✅ **Classic and PowerSeries Panel Support** - Automatic compatibility  
✅ **Live Alarm Functionality** - Emergency buttons (Fire, Aux, Panic)  
✅ **Complete Zone Monitoring** - Up to 16 zones with alarm status  
✅ **Multi-Partition Support** - Up to 4 partitions  
✅ **System Health Monitoring** - Memory, WiFi, connectivity diagnostics  
✅ **Watchdog Protection** - Prevents crashes and infinite loops  
✅ **Enhanced Stability** - Timer fixes, CPU frequency optimization  
✅ **Comprehensive Diagnostics** - Real-time system monitoring  

## Quick Start

### 1. Choose Your Panel Type

**For PowerSeries Panels** (PC1616, PC1832, PC1864, PC5005, PC5010, PC5015, PC5020):
```yaml
series_type: "PowerSeries"  # Keep this line as-is
# pc16_pin: ${pc16_pin}     # Keep this commented out
```

**For Classic Series Panels** (PC1500, PC1550, PC1832, PC1864, PC1616):
```yaml
series_type: "Classic"      # Change to "Classic"
pc16_pin: ${pc16_pin}       # Uncomment this line
```

### 2. Configure Your Secrets

Create or update `secrets.yaml`:
```yaml
# WiFi Configuration
wifi_ssid: "Your_WiFi_Network"
wifi_password: "your_wifi_password"

# DSC Configuration  
access_code: "1234"  # Your DSC access code

# ESPHome Configuration
api_encryption_key: "your_32_character_encryption_key_here"
ota_password: "your_ota_password"

# MQTT Configuration (optional)
mqtt_broker: "192.168.1.100"
mqtt_username: "your_mqtt_username"
mqtt_password: "your_mqtt_password"
```

### 3. Customize Zone Names

Update the zone configurations to match your actual setup:

```yaml
binary_sensor:
  - platform: template
    id: zone_1
    name: "Zone 1 - Front Door"      # ← Change this to your actual zone name
    device_class: door               # ← Change device_class as needed
    icon: "mdi:door"
```

**Device Classes:**
- `door` - Entry doors
- `window` - Windows  
- `motion` - Motion detectors
- `garage_door` - Garage doors
- `safety` - Glass break, smoke detectors
- `opening` - Generic opening sensors

### 4. Hardware Connections

**Standard Connections (All Panels):**
```
DSC Panel    ESP32 GPIO    Resistor
---------    ----------    --------
Clock   →    GPIO 18       33kΩ pullup to 3.3V
Data    →    GPIO 19       10kΩ pullup to 3.3V, 33kΩ to GND
Data    ←    GPIO 21       (for writing to panel)
```

**Additional for Classic Series:**
```
DSC Panel    ESP32 GPIO    Resistor
---------    ----------    --------  
PC-16   →    GPIO 17       1kΩ pullup to 3.3V
```

## Features Explanation

### LoadProhibited Crash Prevention
- **ESP-IDF 5.3.2 optimized** memory management
- **Enhanced stack sizes** for complex operations
- **Timer initialization fixes** preventing startup crashes
- **Memory allocation failure prevention** during WiFi initialization

### Live Alarm Functionality
```yaml
button:
  - platform: template
    name: "DSC Fire Alarm"     # Triggers fire alarm
  - platform: template  
    name: "DSC Aux Alarm"      # Triggers auxiliary alarm
  - platform: template
    name: "DSC Panic Alarm"    # Triggers panic alarm
```

### System Health Monitoring
- **Real-time memory tracking** with automatic recovery
- **WiFi connectivity monitoring** with reconnection
- **Stack overflow protection** with early warning
- **Memory fragmentation detection** and reporting

### Multi-Platform Support
- **ESP-IDF 5.3+** framework (recommended)
- **ESP32** microcontroller (required for ESP-IDF 5.3)
- **PowerSeries and Classic** DSC panels
- **Home Assistant integration** via ESPHome API or MQTT

## Advanced Configuration

### Memory Optimization
For systems with limited resources:
```yaml
logger:
  level: WARN  # Reduce logging overhead

sensor:
  - platform: template
    update_interval: 120s  # Reduce sensor update frequency
```

### Custom Zones
Add additional zones (supports up to 64):
```yaml
binary_sensor:
  - platform: template
    id: zone_17
    name: "Zone 17 - Custom Sensor"
    device_class: opening
```

### Multiple Partitions
Enable additional partitions:
```yaml
alarm_control_panel:
  - platform: dsc_keybus
    partition: 2
    name: "DSC Partition 2"
    id: partition_2_panel
```

## Troubleshooting

### LoadProhibited Crashes
If you experience crashes:
1. Verify ESP-IDF 5.3+ framework is selected
2. Check memory allocation settings in sdkconfig_options
3. Monitor free heap sensor for memory issues
4. Review logs for allocation failures

### Classic Series Issues  
For Classic series panels:
1. Uncomment `pc16_pin` configuration
2. Change `series_type` to "Classic"
3. Verify PC-16 wiring with 1kΩ resistor
4. Enable DSC panel PGM output for PC-16

### Memory Issues
Monitor these sensors:
- **DSC Free Heap** - Should stay above 25KB
- **DSC Largest Free Block** - Should stay above 5KB  
- **DSC Stack High Water Mark** - Should stay above 1KB

### WiFi Connection Issues
The configuration includes enhanced WiFi stability:
- **5-minute connection timeout** (prevents watchdog crashes)
- **Fallback AP mode** for configuration access
- **Automatic reconnection** with monitoring

## Integration with Home Assistant

### Automatic Discovery
When using ESPHome API, all entities are automatically discovered in Home Assistant.

### MQTT Integration
If using MQTT, entities will appear under:
- **Devices & Services** → **MQTT** → **DSC Alarm**

### Dashboard Cards
Example Lovelace card configuration:
```yaml
type: alarm-panel
entity: alarm_control_panel.dsc_partition_1
states:
  - arm_home
  - arm_away
  - arm_night
```

## File Structure
```
extras/ESPHome/
├── DSCAlarm_Comprehensive_All_Fixes.yaml  ← Use this configuration
├── DSCAlarm_ESP_IDF_5_3_LoadProhibited_Fix.yaml  ← ESP-IDF specific version
├── DscAlarm_Classic.yaml                   ← Classic series only
└── DscAlarm.yaml                          ← Basic configuration
```

## Support

For issues or questions:
1. Check the comprehensive documentation in the repository
2. Review the ESP-IDF 5.3 fix guidance documents
3. Monitor system health sensors for diagnostic information
4. Enable debug logging for detailed troubleshooting

This configuration represents the culmination of all fixes and improvements in the repository, providing the most stable and feature-complete DSC alarm integration available.