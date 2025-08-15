# DSC Keybus Interface

This repository provides Home Assistant integration for DSC alarm systems via MQTT.

## ⚠️ CRITICAL NOTICE: YAML Configuration for MQTT Entities is Deprecated

**Home Assistant has deprecated YAML configuration for MQTT entities.** The YAML files in this repository have been updated to remove deprecated MQTT entity configurations.

**New approach required:**
1. **Use ESPHome configuration** (recommended): `extras/ESPHome/DscAlarm.yaml`
2. **Or configure entities manually** via Home Assistant UI (Settings > Devices & Services > MQTT)

See `MQTT_ENTITY_MIGRATION_GUIDE.md` for complete migration instructions.

## Quick Start

1. **Configure Secrets**: Copy `secrets.yaml.example` to `secrets.yaml` and edit with your actual values
2. **Choose Your Path**:
   - **Recommended**: Use ESPHome configuration (`extras/ESPHome/DscAlarm.yaml`) for automatic MQTT Discovery
   - **Alternative**: Use Home Assistant UI to manually configure MQTT entities
3. **YAML Configuration**: Only use the provided YAML files for template sensors, automations, and scripts (no longer for MQTT entities)
4. **Validate**: Run `python3 validate_config.py` to check your configuration
5. **Clean Build** (if needed): Run `./clean_build.sh` to clean ESPHome build caches
6. **Deploy**: Add supported sections (templates, automations) to your `configuration.yaml`

## Script Execution Guide

### Python Scripts
```bash
python3 validate_config.py        # Configuration validation
python3 test_watchdog_timeout_fix.py  # Test fixes
```

### Bash Scripts
```bash
./clean_build.sh                  # Clean ESPHome build directories
# OR
bash clean_build.sh
```

⚠️ **Important**: Don't run bash scripts with Python (`python3 clean_build.sh` will fail)

## Security

All sensitive information (MQTT credentials, alarm codes, IP addresses) is now managed through `secrets.yaml`. See `SECRETS_GUIDE.md` for detailed setup instructions.

## ESPHome Configuration (Recommended)

The `extras/ESPHome/DscAlarm.yaml` configuration provides:
- **Single ESP32/ESP8266 Device** - Complete integration with just one device
- **Automatic MQTT Discovery** - No YAML configuration needed in Home Assistant
- **Complete Device Integration** - All sensors grouped under one device
- **Enhanced Diagnostics** - WiFi signal, uptime, free memory, connection status
- **DSC Series Support** - Both PowerSeries AND Classic series panels fully supported with automatic PC-16 pin configuration
- **Standalone Testing Mode** - Test configuration without connected DSC panel (`standalone_mode: true`)
- **Emergency Controls** - Fire, Aux, and Panic alarm buttons
- **Zone Monitoring** - Individual sensors for doors, windows, motion detectors
- **System Status** - Partition states, trouble indicators, fire alarms
- **Over-the-Air Updates** - Update firmware wirelessly
- **ESP-IDF 5.3.2+ LoadProhibited Crash Fix** - Advanced protection against system crashes

## 🛡️ ESP-IDF 5.3 LoadProhibited Fix

**QUESTION: Can I still use the ESP-IDF 5.3 LoadProhibited fix with all these changes?**

**ANSWER: YES! You should definitely still use it.**

The ESP-IDF 5.3.2 LoadProhibited fix is the **most comprehensive and current solution**.

## 🎯 NEW: Comprehensive Configuration with ALL Fixes

**For users who want EVERYTHING in one configuration file:**

**File**: `extras/ESPHome/DSCAlarm_Comprehensive_All_Fixes.yaml`

This new configuration incorporates **ALL fixes and features** from the repository:
- ✅ **ESP-IDF 5.3.2 LoadProhibited crash fix** (complete crash prevention)
- ✅ **Memory allocation failure prevention** (heap optimization)  
- ✅ **Classic AND PowerSeries support** (automatic compatibility)
- ✅ **Live alarm functionality** (Fire, Aux, Panic emergency buttons)
- ✅ **Complete zone monitoring** (up to 16 zones with alarm status)
- ✅ **Multi-partition support** (up to 4 partitions)
- ✅ **System health monitoring** (memory, WiFi, connectivity diagnostics)
- ✅ **Watchdog protection** (prevents crashes and infinite loops)
- ✅ **Enhanced stability fixes** (timer fixes, CPU optimization)

**Usage**:
```yaml
# Simply use this file as your ESPHome configuration
<<: !include extras/ESPHome/DSCAlarm_Comprehensive_All_Fixes.yaml

# Customize for your setup:
# 1. Set series_type: "PowerSeries" or "Classic" 
# 2. Configure zone names for your actual zones
# 3. Add your secrets (WiFi, access codes)
```

See `extras/ESPHome/COMPREHENSIVE_CONFIG_GUIDE.md` for detailed setup instructions.

The ESP-IDF 5.3.2 LoadProhibited fix benefits:
- ✅ **Includes all previous fixes** and extends them with advanced protections
- ✅ **Works seamlessly** with all repository improvements and recent changes
- ✅ **Prevents multiple crash types**: 0xcececece LoadProhibited, memory allocation failures, timer system crashes
- ✅ **Enhanced for ESP-IDF 5.3.2+** with specific optimizations and compatibility layers

**For ESP-IDF users**: Use `DSCAlarm_ESP_IDF_5_3_LoadProhibited_Fix.yaml` - it's the gold standard.
**For Arduino users**: Standard configurations already include the necessary basic protections.

📖 **Complete guidance**: `ESP_IDF_5_3_LOADPROHIBITED_FIX_GUIDANCE.md`

**📋 Complete Setup Documentation:**
- **`ESPHOME_COMPLETE_SETUP_GUIDE.md`** - **NEW**: Comprehensive hardware and software setup guide
- **`DSC_STANDALONE_MODE_GUIDE.md`** - **NEW**: Testing without connected panel
- **`ESPHOME_MIGRATION_GUIDE.md`** - Step-by-step migration from MQTT YAML
- **`ESP_IDF_5_3_LOADPROHIBITED_FIX_GUIDANCE.md`** - **NEW**: Complete guide on ESP-IDF 5.3 LoadProhibited fix usage

## Files

- `HA_Yaml.YAML` - ⚠️ **UPDATED**: Now contains migration guide and template examples (MQTT entities removed)
- `HA_Yaml_Enhanced.YAML` - ⚠️ **UPDATED**: MQTT broker config and automations only (MQTT entities removed)  
- `LEGACY_HA_Yaml.YAML` - **NEW**: Legacy MQTT entities for Home Assistant <2023.8 only (⚠️ with warnings)
- `configuration.yaml.example` - Template showing supported YAML configurations only
- `secrets.yaml.example` - Template for secure credential storage
- `MQTT_ENTITY_MIGRATION_GUIDE.md` - **NEW**: Complete migration guide for deprecated MQTT entities
- `SECRETS_GUIDE.md` - Detailed secrets configuration guide
- `MQTT_STRUCTURE_GUIDE.md` - Important guidance on correct MQTT configuration structure
- `MQTT_CONFIGURATION_TROUBLESHOOTING.md` - Troubleshooting guide for common configuration errors
- `validate_config.py` - Configuration validation tool
- `extras/ESPHome/DscAlarm.yaml` - **RECOMMENDED**: Enhanced ESPHome configuration for MQTT Discovery with diagnostic sensors
- `extras/ESPHome/DSCAlarm_ESP_IDF_5_3_LoadProhibited_Fix.yaml` - **ESP-IDF USERS**: Advanced ESP-IDF 5.3.2+ configuration with LoadProhibited crash prevention
- `ESPHOME_COMPLETE_SETUP_GUIDE.md` - **NEW**: Complete hardware and software setup documentation (answers hardware requirements, wiring, step-by-step process)
- `ESPHOME_MIGRATION_GUIDE.md` - **UPDATED**: Migration guide from deprecated MQTT YAML entities
- `ESP_IDF_5_3_LOADPROHIBITED_FIX_GUIDANCE.md` - **NEW**: Definitive guide on ESP-IDF 5.3 LoadProhibited fix compatibility and usage
- `extras/ESPHome/DSCAlarm_Comprehensive_All_Fixes.yaml` - **NEW**: All-in-one configuration with every fix and feature
- `extras/ESPHome/COMPREHENSIVE_CONFIG_GUIDE.md` - **NEW**: Setup guide for the comprehensive configuration

## What Changed

**YAML files no longer contain MQTT entities** due to Home Assistant deprecation:
- ❌ Removed: alarm_control_panel, sensor, binary_sensor, button MQTT configurations
- ✅ Kept: MQTT broker settings, automations, scripts, template sensors
- ✅ Added: Migration guidance and examples

**Migration required** - see `MQTT_ENTITY_MIGRATION_GUIDE.md`

## 🚨 Legacy Support for Older Home Assistant Versions

**`LEGACY_HA_Yaml.YAML`** contains the original MQTT entity configurations for users running Home Assistant versions **before 2023.8**. 

⚠️ **CRITICAL WARNINGS:**
- **DO NOT** use with Home Assistant 2023.8+ (will cause errors)
- **ONLY** for Home Assistant 2022.x and older
- **Plan to upgrade** to current HA and use MQTT Discovery instead

If you're on an older HA version and need the legacy entities, use that file with extreme caution and plan your migration path.

## ⚠️ Important Configuration Notes

**YAML Limitations**: MQTT entities (alarm panels, sensors, binary sensors, buttons) can no longer be configured in YAML. Use:
1. ESPHome with MQTT Discovery (automatic)
2. Home Assistant UI (manual)

**Still Supported in YAML**: Template sensors, automations, scripts, input helpers, and other non-MQTT platforms.

## Common Issues

### MQTT Entities Not Working
**Problem**: YAML MQTT entities are deprecated in Home Assistant.
**Solution**: See `MQTT_ENTITY_MIGRATION_GUIDE.md` for migration to ESPHome or UI configuration.

### Configuration Errors
If you're getting errors like:
- `'automation' is an invalid option for 'mqtt'`
- `'mqtt' is an invalid option for 'mqtt'`

See `MQTT_CONFIGURATION_TROUBLESHOOTING.md` for detailed solutions and examples.
