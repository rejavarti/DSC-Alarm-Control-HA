# ESPHome Migration Guide for DSC Alarm System

## Overview

This guide helps you migrate from MQTT YAML entities (deprecated in Home Assistant 2023.8+) to ESPHome with automatic MQTT Discovery. Your current diagnostic data and functionality will be preserved and enhanced.

## DSC Series Compatibility

This ESPHome configuration supports **BOTH** DSC series:

### ✅ DSC PowerSeries
- PC1555MX, PC5015, PC1616, PC1832, PC1864, etc.
- Default configuration (no changes needed)

### ✅ DSC Classic Series  
- PC1500, PC1550, PC1832, PC1864, PC1616, etc.
- Requires uncommenting `classic_series_define` in the configuration
- Requires PC-16 wiring configuration

**Note**: This uses a LOCAL dscKeybusInterface implementation, not the external Dilbert66/esphome-dsckeybus project (which doesn't support Classic series).

## Your Current Setup

Based on your MQTT diagnostics, you have:
- ✅ DSC Keybus Interface working
- ✅ MQTT broker connected
- ✅ System diagnostics (Uptime, FreeHeap, WiFi RSSI, etc.)
- ✅ Status monitoring (SystemHealthy, KeybusConnected, BufferOverflow)

## Migration Steps

### Step 1: Install ESPHome (if not already installed)

1. In Home Assistant, go to **Settings** → **Add-ons**
2. Search for "ESPHome" and install it
3. Start the ESPHome add-on

### Step 2: Prepare Your Device

1. Ensure your ESP8266/ESP32 device is accessible
2. Note down your current access code and WiFi credentials
3. Back up your current configuration (optional but recommended)

### Step 3: Create ESPHome Configuration

1. Open ESPHome dashboard (usually at `http://your-ha-ip:6052`)
2. Click **"+ New Device"**
3. Choose **"Continue"** and name it `dscalarm`
4. Select your device type (ESP8266/ESP32)
5. Replace the generated configuration with the enhanced `extras/ESPHome/DscAlarm.yaml`

#### For DSC Classic Series Users:
If you have a Classic series panel (PC1500, PC1550, etc.), you need to:
1. Uncomment the `build_flags: - -DdscClassicSeries` lines in the esphome section
2. Ensure you have the PC-16 wiring configuration connected
3. Verify your panel model is compatible with Classic series protocol

#### For DSC PowerSeries Users:
- No additional configuration needed - the default settings work for PowerSeries panels

### Step 4: Configure Secrets

Create or update your `secrets.yaml` file in ESPHome:

```yaml
# WiFi credentials
wifi_ssid: "YourWiFiNetwork"
wifi_password: "YourWiFiPassword"

# ESPHome API
api_password: "your_api_password"
ota_password: "your_ota_password"

# DSC Access Code
access_code: "1234"  # Your DSC system code

# MQTT Configuration (for discovery)
mqtt_broker: "192.168.1.100"  # Your MQTT broker IP
mqtt_username: "mqtt_user"    # Your MQTT username
mqtt_password: "mqtt_pass"    # Your MQTT password
```

### Step 5: Flash the Device

1. Connect your ESP device via USB
2. In ESPHome dashboard, click **"Install"** on your dscalarm device
3. Choose **"Plug into the computer running ESPHome Dashboard"**
4. Select the appropriate port and flash

### Step 6: Verify MQTT Discovery

After flashing, your device will automatically:
- Connect to your MQTT broker
- Publish discovery messages to Home Assistant
- Create all entities automatically (no YAML needed!)

### Step 7: Check Home Assistant Integration

1. Go to **Settings** → **Devices & Services**
2. Look for **"ESPHome"** integration
3. You should see your "dscalarm" device
4. Click on it to see all auto-discovered entities:

#### Sensors Created Automatically:
- **DSC Alarm Panel** (alarm_control_panel) - Main system control
- **Zone sensors** - All your door/window/motion sensors
- **System diagnostics** - Uptime, WiFi signal, free memory
- **Partition status** - Current system status and messages
- **Emergency buttons** - Fire, Aux, Panic alarms
- **Connectivity** - Keybus connection status

### Step 8: Remove Old YAML Configuration (Optional)

Once ESPHome is working:
1. Remove old MQTT entity YAML from your `configuration.yaml`
2. Keep MQTT broker configuration (still needed)
3. Restart Home Assistant to clean up old entities

## Advantages of ESPHome Migration

### ✅ What You Gain:
- **Automatic Discovery** - No more YAML configuration
- **Better Device Management** - Grouped entities under one device
- **Enhanced Diagnostics** - More detailed system information
- **OTA Updates** - Update firmware wirelessly
- **Improved Reliability** - Built-in reconnection handling
- **Future Proof** - Compatible with all HA versions

### 🔧 Enhanced Features:
- **WiFi Reconnection Tracking** - Monitor connection stability
- **System Health Monitoring** - Better diagnostic information
- **Structured Device Classes** - Proper sensor categories
- **State Classes** - Better long-term statistics
- **Version Tracking** - Know your firmware version

## Wiring Information

### DSC PowerSeries Wiring (Default)
```
DSC Aux(+) --- 5v voltage regulator --- ESP32/ESP8266 5v pin
DSC Aux(-) --- ESP32/ESP8266 Ground

DSC Yellow --- 33k ohm resistor ---|--- dscClockPin (GPIO 18)
                                   +--- 10k ohm resistor --- Ground

DSC Green ---- 33k ohm resistor ---|--- dscReadPin (GPIO 19)  
                                   +--- 10k ohm resistor --- Ground
```

### DSC Classic Series Wiring (Additional Requirements)
For Classic series panels, you also need PC-16 configuration:
```
DSC PGM ---+-- 1k ohm resistor --- DSC Aux(+)
           |
           +-- 33k ohm resistor ---|--- dscPC16Pin (GPIO 17)
                                   +--- 10k ohm resistor --- Ground
```

**Note**: Classic series requires PGM configured for PC-16 output in your panel programming.

## Troubleshooting

### Device Not Connecting
1. Check WiFi credentials in secrets.yaml
2. Verify your ESP device is powered and within range
3. Check ESPHome logs for connection errors

### MQTT Not Working
1. Verify MQTT broker settings in secrets.yaml
2. Ensure MQTT broker allows connections from ESP device IP
3. Check Home Assistant MQTT integration is working

### Missing Entities
1. Check ESPHome device logs
2. Verify MQTT discovery is enabled (`discovery: true`)
3. Restart Home Assistant if needed

### Classic Series Not Connecting
1. Verify you uncommented the `build_flags: - -DdscClassicSeries` lines
2. Check PC-16 wiring is correctly connected to GPIO 17
3. Ensure your panel's PGM is programmed for PC-16 output
4. Verify panel model is actually Classic series (PC1500, PC1550, etc.)

### Old Entities Still Visible
1. Go to **Settings** → **Devices & Services** → **Entities**
2. Filter by "Unavailable" entities
3. Remove old MQTT entities manually if needed

## Support

- **ESPHome Documentation**: https://esphome.io/
- **DSC Keybus Project**: https://github.com/Dilbert66/esphome-dsckeybus
- **Home Assistant MQTT**: https://www.home-assistant.io/integrations/mqtt/

## Summary

This migration preserves all your current functionality while providing better integration with Home Assistant. Your diagnostic data (Uptime, FreeHeap, WiFi RSSI, etc.) will continue to work, plus you'll get additional system monitoring capabilities.

The ESPHome approach is the recommended solution for Home Assistant 2023.8+ users, providing automatic entity discovery without any YAML configuration needed.