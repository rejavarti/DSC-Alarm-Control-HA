# Secrets Configuration Guide

This guide explains how to properly configure sensitive information for the DSC Alarm Control System Home Assistant integration using the secrets.yaml file.

## Why Use Secrets?

The `secrets.yaml` file allows you to:
- Keep sensitive information like passwords and access codes out of your main configuration files
- Prevent accidental exposure of credentials when sharing your configuration
- Centralize all sensitive configuration in one secure location
- Follow Home Assistant security best practices

## Setup Instructions

### 1. Create Your Secrets File

Copy the example file to create your actual secrets file:
```bash
cp secrets.yaml.example secrets.yaml
```

### 2. Edit Your Values

Open `secrets.yaml` and replace all example values with your actual configuration:

```yaml
# MQTT Configuration
mqtt_broker_ip: "192.168.1.100"          # Your MQTT broker IP
mqtt_username: "your_mqtt_username"       # Your MQTT username  
mqtt_password: "your_secure_password"     # Your MQTT password

# DSC Alarm System
dsc_alarm_code: "1234"                    # Your DSC panel access code

# Hardware Configuration  
esp32_ip: "http://192.168.1.101"         # Your ESP32 device IP
```

### 3. File Permissions

Ensure your secrets file has appropriate permissions:
```bash
chmod 600 secrets.yaml
```

### 4. Verify Configuration

Run the validation script to ensure everything is configured correctly:
```bash
python3 validate_config.py
```

## Security Best Practices

1. **Never commit secrets.yaml**: The file is already added to `.gitignore` to prevent accidental commits
2. **Use strong passwords**: Especially for MQTT authentication
3. **Enable MQTT TLS**: Use encrypted connections in production
4. **Regular rotation**: Periodically change passwords and access codes
5. **Backup securely**: Store backups of your secrets file in a secure location

## Troubleshooting

### YAML Syntax Errors
If you see syntax errors, ensure:
- No tabs are used (use spaces only)
- Quotes are properly matched
- Indentation is consistent

### Missing Secrets
If Home Assistant reports missing secrets:
- Verify the secret name matches exactly (case-sensitive)
- Ensure the secrets.yaml file is in your Home Assistant config directory
- Check file permissions allow Home Assistant to read the file

### Connection Issues
If MQTT or device connections fail:
- Verify IP addresses are correct and devices are accessible
- Test MQTT credentials using an MQTT client
- Check network connectivity and firewall settings

## Advanced Configuration

### Multiple Alarm Panels
For multiple DSC panels, add additional secrets:
```yaml
dsc_alarm_code_panel1: "1234"
dsc_alarm_code_panel2: "5678"
```

### Development vs Production
Use different secrets files for different environments by organizing them appropriately in your Home Assistant configuration structure.

## Support

If you encounter issues:
1. Run `python3 validate_config.py` to check for configuration problems
2. Check Home Assistant logs for specific error messages
3. Verify all network connectivity and device accessibility
4. Review the enhanced YAML configuration comments for additional guidance