# DSC Keybus Interface

This repository provides Home Assistant integration for DSC alarm systems via MQTT.

## Quick Start

1. **Configure Secrets**: Copy `secrets.yaml.example` to `secrets.yaml` and edit with your actual values
2. **Choose Configuration**: Use either `HA_Yaml.YAML` (basic) or `HA_Yaml_Enhanced.YAML` (recommended)
3. **Validate**: Run `python3 validate_config.py` to check your configuration
4. **Deploy**: Add the relevant sections to your Home Assistant `configuration.yaml`

## Security

All sensitive information (MQTT credentials, alarm codes, IP addresses) is now managed through `secrets.yaml`. See `SECRETS_GUIDE.md` for detailed setup instructions.

## Files

- `HA_Yaml.YAML` - Basic Home Assistant configuration
- `HA_Yaml_Enhanced.YAML` - Enhanced configuration with diagnostics and better device integration
- `secrets.yaml.example` - Template for secure credential storage
- `SECRETS_GUIDE.md` - Detailed secrets configuration guide
- `MQTT_STRUCTURE_GUIDE.md` - Important guidance on correct MQTT configuration structure
- `validate_config.py` - Configuration validation tool

## ⚠️ Important Configuration Notes

**MQTT Structure**: When copying configuration sections to your Home Assistant `configuration.yaml`, ensure each section (`alarm_control_panel:`, `sensor:`, `binary_sensor:`, `button:`, `automation:`) is at the **top level** of your configuration file. Do NOT nest them under the `mqtt:` section. See `MQTT_STRUCTURE_GUIDE.md` for detailed examples.
