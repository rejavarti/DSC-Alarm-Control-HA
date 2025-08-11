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
5. **Deploy**: Add supported sections (templates, automations) to your `configuration.yaml`

## Security

All sensitive information (MQTT credentials, alarm codes, IP addresses) is now managed through `secrets.yaml`. See `SECRETS_GUIDE.md` for detailed setup instructions.

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
- `extras/ESPHome/DscAlarm.yaml` - **RECOMMENDED**: ESPHome configuration for MQTT Discovery

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
