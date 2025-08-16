# ESP32 Boot Failure Recovery Tools

This directory contains automated tools to diagnose and fix ESP32 boot failures, specifically the "invalid header: 0x74696e69" error pattern.

## Quick Start

If your ESP32 is stuck in a boot loop with "invalid header" errors:

```bash
# Interactive helper (recommended for beginners)
python3 esp32_boot_helper.py

# Or run automatic recovery directly
python3 esphome_flash_recovery.py full-recover
```

## Available Tools

### 1. Interactive Helper (`esp32_boot_helper.py`)
**Best for beginners** - provides guided assistance based on your specific situation.

```bash
python3 esp32_boot_helper.py
```

### 2. Diagnostic Tool (`diagnose_boot_failure.py`)
Analyzes boot failure patterns and creates recovery scripts.

```bash
python3 diagnose_boot_failure.py
```

### 3. Flash Recovery Tool (`esphome_flash_recovery.py`)
ESPHome-specific flash recovery with auto-detection.

```bash
# Create minimal test configuration
python3 esphome_flash_recovery.py create-config

# Perform flash recovery (auto-detects port)
python3 esphome_flash_recovery.py recover

# Complete recovery with test firmware
python3 esphome_flash_recovery.py full-recover
```

## What These Tools Fix

### ✅ Flash Corruption Issues
- `invalid header: 0x74696e69` errors
- Continuous boot loops before ESP-IDF initialization
- Corrupted firmware uploads
- Incomplete flash programming

### ❌ Not Fixed by These Tools
- LoadProhibited crashes (0xcececece patterns) - see `ESPHOME_ESP32_LOADPROHIBITED_COMPLETE_FIX.md`
- Hardware failures
- Power supply issues (must be fixed separately)

## Generated Files

When you run the tools, they may create:

- `flash_recovery.sh` - Basic bash recovery script
- `esp32_boot_diagnostic.yaml` - Minimal diagnostic ESPHome configuration
- `esp32_recovery_test.yaml` - Recovery test configuration

## Manual Recovery (if tools fail)

1. **Complete flash erase:**
   ```bash
   esptool.py --chip esp32 --port /dev/ttyUSB0 erase_flash
   ```

2. **Upload minimal firmware:**
   ```bash
   esphome compile esp32_recovery_test.yaml
   esphome upload esp32_recovery_test.yaml
   ```

3. **Verify recovery:**
   ```bash
   esphome logs esp32_recovery_test.yaml
   ```

## Integration with DSC Alarm System

After successful recovery:

1. **Test with diagnostic configuration:**
   ```bash
   esphome upload extras/ESPHome/dscalarm-diagnostic.yaml
   ```

2. **Upload your production configuration:**
   ```bash
   esphome upload extras/ESPHome/DscAlarm.yaml
   ```

## Documentation

- `ESP32_INVALID_HEADER_BOOT_FAILURE_FIX.md` - Complete diagnosis and recovery guide
- `ESPHOME_ESP32_LOADPROHIBITED_COMPLETE_FIX.md` - LoadProhibited crash fixes
- `extras/ESPHome/README.md` - ESPHome configuration guide

## Troubleshooting

### Common Issues

1. **Port not detected:**
   ```bash
   ls /dev/tty* | grep -E "(USB|ACM)"
   python3 esphome_flash_recovery.py recover /dev/ttyUSB0
   ```

2. **Permission denied:**
   ```bash
   sudo usermod -a -G dialout $USER
   # Log out and back in
   ```

3. **ESPHome not found:**
   ```bash
   pip install esphome
   ```

### Getting Help

Run the interactive helper for guided assistance:
```bash
python3 esp32_boot_helper.py
```

## Success Indicators

After recovery, you should see:
- Normal ESP-IDF boot sequence
- ESPHome startup messages  
- WiFi AP "ESP32-Recovery-Test" visible
- Status LED on (if using recovery test config)
- No more "invalid header" errors

The ESP32 is ready for your DSC alarm configuration once these are confirmed.