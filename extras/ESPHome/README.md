# ESPHome Configuration Files - Quick Guide

## 🛡️ NEW: ESP32 Watchdog Timer Improvements
**Latest Update**: ESP32 ESPHome configurations now include comprehensive watchdog timer protection to prevent freezing during .bin installation and operation. This resolves installation hanging issues that were previously only fixed in the Arduino INO file.

**Key Benefits:**
- ✅ Prevents ESP32 freezing during ESPHome .bin installation  
- ✅ Eliminates system hanging during heavy alarm traffic
- ✅ Automatic recovery from DSC communication issues
- ✅ Identical stability improvements as Arduino INO users
- ✅ No configuration changes required - automatically active on ESP32

📖 **[Read Full Details: ESPHOME_WATCHDOG_IMPROVEMENTS.md](../../ESPHOME_WATCHDOG_IMPROVEMENTS.md)**

## 📁 Choose Your Configuration File

### ESP8266 Configurations
#### For DSC PowerSeries Panels
**Use: `DscAlarm.yaml`**
- PC1555MX, PC5015, PC1616, PC1832, PC1864, PC1565, PC5010, PC5015, PC5020
- Default configuration - no changes needed

#### For DSC Classic Series Panels  
**Use: `DscAlarm_Classic.yaml`** ⭐ **RECOMMENDED**
- PC1500, PC1550, PC1832, PC1864, PC1616, PC1500MX, PC1550MX
- Pre-configured with `-DdscClassicSeries` build flag
- No manual editing required

### ESP32 Configurations
#### For DSC PowerSeries Panels
**Use: `DscAlarm_ESP32.yaml`**
- PC1555MX, PC5015, PC1616, PC1832, PC1864, PC1565, PC5010, PC5015, PC5020
- ESP32 pin assignments (GPIO 18, 19, 21)

#### For DSC Classic Series Panels
**Use: `DscAlarm_Classic_ESP32.yaml`** ⭐ **RECOMMENDED**
- PC1500, PC1550, PC1832, PC1864, PC1616, PC1500MX, PC1550MX
- Pre-configured with `-DdscClassicSeries` build flag
- ESP32 pin assignments + PC-16 support

## ⚠️ Important YAML Syntax for ESP32
When using ESP32 configurations, **build_flags must be under `esphome:` section, NOT under `esp32:` section**:

✅ **CORRECT:**
```yaml
esphome:
  name: dscalarm
  build_flags:
    - -DdscClassicSeries

esp32:
  board: esp32dev
  framework:
    type: esp-idf
```

❌ **INCORRECT (causes validation error):**
```yaml
esp32:
  board: esp32dev
  framework:
    type: esp-idf
  build_flags:          # <- This is invalid!
    - -DdscClassicSeries
```

## 🔧 Additional Requirements for Classic Series
- Extra 1kΩ resistor for PC-16 connection:
  - **ESP8266**: Connect to D5 (GPIO 14)
  - **ESP32**: Connect to GPIO 17  
- PGM output configuration in DSC panel
- All standard PowerSeries wiring (33kΩ and 10kΩ resistors)

## 📌 Pin Assignments

### ESP8266 Pins
- **Clock**: D1 (GPIO 5)
- **Read**: D2 (GPIO 4)  
- **Write**: D8 (GPIO 15)
- **PC-16** (Classic only): D5 (GPIO 14)

### ESP32 Pins
- **Clock**: GPIO 18
- **Read**: GPIO 19
- **Write**: GPIO 21
- **PC-16** (Classic only): GPIO 17

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