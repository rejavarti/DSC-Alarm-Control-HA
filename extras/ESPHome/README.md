# ESPHome Configuration Files - Quick Guide

## 📁 Choose Your Configuration File

### For DSC PowerSeries Panels
**Use: `DscAlarm.yaml`**
- PC1555MX, PC5015, PC1616, PC1832, PC1864, PC1565, PC5010, PC5015, PC5020
- Default configuration - no changes needed

### For DSC Classic Series Panels  
**Use: `DscAlarm_Classic.yaml`** ⭐ **RECOMMENDED**
- PC1500, PC1550, PC1832, PC1864, PC1616, PC1500MX, PC1550MX
- Pre-configured with `-DdscClassicSeries` build flag
- No manual editing required

**Alternative**: Use `DscAlarm.yaml` and manually uncomment:
```yaml
build_flags:
  - -DdscClassicSeries
```

## 🔧 Additional Requirements for Classic Series
- Extra 1kΩ resistor for PC-16 connection:
  - **ESP8266**: Connect to D5 (GPIO 14)
  - **ESP32**: Connect to GPIO 17  
- PGM output configuration in DSC panel
- All standard PowerSeries wiring (33kΩ and 10kΩ resistors)

## 📋 Both Configurations Include
- Automatic MQTT Discovery for Home Assistant
- All diagnostic sensors (WiFi RSSI, uptime, free memory)
- Emergency alarm buttons (Fire, Aux, Panic)
- Zone monitoring with proper device classes
- System health and connectivity monitoring
- Over-the-air updates

## ❓ Not Sure Which Panel You Have?
Check your DSC panel model number:
- **PC15xx or PC1500/PC1550**: Classic Series → Use `DscAlarm_Classic.yaml`
- **PC1555MX or newer PC5xxx**: PowerSeries → Use `DscAlarm.yaml`