# ESP32 "Invalid Header" Boot Failure - Complete Diagnosis and Fix

## Issue Summary

ESP32 devices experiencing continuous boot loops with the error pattern:
```
[19:06:40]rst:0x10 (RTCWDT_RTC_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
[19:06:40]invalid header: 0x74696e69
[19:06:40]invalid header: 0x74696e69
[19:06:40]invalid header: 0x74696e69
```

## Root Cause Analysis

### Error Pattern Breakdown

- **Reset Cause**: `0x10 (RTCWDT_RTC_RESET)` - RTC Watchdog Timer Reset
- **Boot Mode**: `0x13 (SPI_FAST_FLASH_BOOT)` - Attempting to boot from SPI flash
- **Invalid Header**: `0x74696e69` - This hex value translates to ASCII "tini"

### Technical Diagnosis

The "invalid header" error with the specific pattern `0x74696e69` indicates **flash memory corruption**. The ESP32 bootloader expects to find a valid firmware image header at specific flash addresses, but instead finds corrupted data.

The "tini" pattern suggests:
1. **Flash corruption during firmware upload**
2. **Incomplete or interrupted flash programming**
3. **Hardware-level flash memory issues**
4. **Power supply instability during programming**

This is **different** from the LoadProhibited crashes (0xcececece patterns) that are addressed by other fixes in this repository.

## Solution Implementation

### Automatic Recovery Tools

This repository now includes automated tools to diagnose and fix this specific boot failure:

#### 1. Boot Failure Diagnostic Tool
```bash
python3 diagnose_boot_failure.py
```

**Features:**
- Analyzes boot failure patterns
- Identifies flash corruption vs other issues
- Creates recovery scripts and configurations
- Provides step-by-step recovery instructions

#### 2. ESPHome Flash Recovery Tool
```bash
python3 esphome_flash_recovery.py full-recover
```

**Features:**
- Auto-detects ESP32 port
- Performs complete flash erase
- Creates minimal test configuration
- Compiles and uploads recovery firmware
- Monitors boot process for verification

### Manual Recovery Procedure

If you prefer manual recovery or the automated tools don't work:

#### Step 1: Complete Flash Erase
```bash
esptool.py --chip esp32 --port /dev/ttyUSB0 erase_flash
```

#### Step 2: Create Minimal Test Configuration
Use the auto-generated `esp32_boot_diagnostic.yaml` or create a minimal config:

```yaml
esphome:
  name: esp32-recovery-test

esp32:
  board: esp32dev
  framework:
    type: esp-idf

wifi:
  ap:
    ssid: "ESP32-Recovery"
    password: "recovery123"

logger:
  level: INFO

api:
ota:
  - platform: esphome
```

#### Step 3: Compile and Upload
```bash
esphome compile esp32_recovery_test.yaml
esphome upload esp32_recovery_test.yaml
```

#### Step 4: Verify Recovery
```bash
esphome logs esp32_recovery_test.yaml
```

Expected successful boot sequence:
```
I (31) boot: ESP-IDF v5.3.2 2nd stage bootloader
I (527) app_init: Project name: esp32-recovery-test
I (527) cpu_start: Pro cpu start user code
[INFO][app:102] ESPHome version 2025.7.5 compiled...
[INFO][app:104] Project esp32-recovery-test version 1.0.0
```

## Prevention Measures

### 1. Stable Power Supply
- Use quality USB cables and power sources
- Avoid USB hubs when possible
- Consider external power during programming

### 2. Reliable Programming Environment
- Use stable USB connections
- Avoid interrupting upload processes
- Monitor for upload errors and retry if needed

### 3. ESPHome Best Practices
- Always validate configurations before upload
- Use `esphome compile` to check for errors
- Enable verbose logging during development

### 4. Hardware Considerations
- Check for loose connections
- Verify ESP32 board quality
- Consider capacitor decoupling if power issues persist

## Integration with Existing DSC Alarm Configurations

After successful recovery using the minimal test configuration, you can proceed to upload your DSC alarm configuration:

### 1. Start with Diagnostic Mode
Use the existing `dscalarm-diagnostic.yaml` configuration first to ensure the ESP32 is stable.

### 2. Progress to Full Configuration
Once diagnostic mode works reliably, upload your production configuration:
- `DscAlarm.yaml` - Standard configuration
- `DSCAlarm_ESP_IDF_5_3_LoadProhibited_Fix.yaml` - Enhanced with LoadProhibited fixes
- `DSCAlarm_Comprehensive_All_Fixes.yaml` - Most robust configuration

### 3. Monitor for Stability
After uploading the DSC configuration, monitor for:
- Successful boot sequence
- DSC hardware initialization
- Stable operation without crashes

## Validation Results

✅ **Flash corruption detection** - Automated hex pattern analysis  
✅ **Recovery script generation** - Creates bash and Python recovery tools  
✅ **Minimal test configuration** - Validates ESP32 basic functionality  
✅ **ESPHome integration** - Uses native ESPHome toolchain  
✅ **Auto-port detection** - Finds ESP32 automatically  
✅ **Comprehensive documentation** - Step-by-step recovery guide  

## Files Created

This solution adds the following files to the repository:

- `diagnose_boot_failure.py` - Main diagnostic tool
- `esphome_flash_recovery.py` - ESPHome-specific recovery tool
- `flash_recovery.sh` - Basic bash recovery script (auto-generated)
- `esp32_boot_diagnostic.yaml` - Minimal diagnostic configuration (auto-generated)
- `esp32_recovery_test.yaml` - Recovery test configuration (auto-generated)
- `ESP32_INVALID_HEADER_BOOT_FAILURE_FIX.md` - This documentation

## Troubleshooting

### If Recovery Tools Fail

1. **Port Detection Issues**
   ```bash
   ls /dev/tty* | grep -E "(USB|ACM)"
   ```
   Manually specify port: `python3 esphome_flash_recovery.py recover /dev/ttyUSB0`

2. **Permission Issues**
   ```bash
   sudo usermod -a -G dialout $USER
   # Log out and back in
   ```

3. **Driver Issues**
   - Install CH340/CP2102 drivers if needed
   - Check device manager (Windows) or dmesg (Linux)

4. **Hardware Issues**
   - Try different USB port/cable
   - Check ESP32 power LED
   - Verify EN/RST button functionality

### If ESP32 Still Won't Boot

The flash corruption may be severe or there could be hardware issues:

1. **Try Different ESP32 Board**
2. **External Power Supply** (3.3V regulated)
3. **Professional Recovery Tools** (if available)
4. **Hardware Replacement** (if board is damaged)

## Success Indicators

After successful recovery, you should see:

1. **No more "invalid header" errors**
2. **Normal ESP-IDF boot sequence**
3. **ESPHome startup messages**
4. **WiFi connectivity** (in AP mode initially)
5. **API responsiveness**

The ESP32 is ready for your DSC alarm configuration once these indicators are confirmed.