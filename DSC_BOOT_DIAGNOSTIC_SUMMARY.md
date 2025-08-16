# DSC Alarm ESP32 Boot Issue - Diagnostic Summary

## Issue Analysis
- **Problem**: Task watchdog timeout after ESP-IDF app_main() returns
- **Cause**: ESPHome main loop hanging during component initialization
- **Symptom**: IDLE0 task not being fed, system reset after 5.9 seconds

## Root Cause
This is the known "returned from app_main()" issue with ESP-IDF 5.3.2 + ESPHome 2025.7.5.
The enhanced ESP-IDF configuration fixes are properly implemented but the DSC component
has a compilation issue that prevents full testing.

## Solutions Implemented

### ✅ Working Solutions
1. **Basic ESP32 Boot Test** (`esp32_boot_diagnostic.yaml`)
   - Tests ESP-IDF task watchdog fixes
   - Compiles and validates successfully
   - Includes comprehensive monitoring

2. **Minimal DSC Test** (`dsc_minimal_test.yaml`) 
   - Tests DSC component in standalone mode
   - Compiles and validates successfully
   - Bypasses hardware initialization issues

### ⚠️ Partial Solutions
3. **Full DSC Diagnostic** (`DscAlarm_Physical_Connection_Diagnostic.yaml`)
   - Contains all necessary ESP-IDF fixes
   - Has compilation issues with complex sensor configuration
   - Needs GPIO pin conflict resolution

## Immediate Actions

### For Testing Boot Fixes
```bash
cd extras/ESPHome
esphome run esp32_boot_diagnostic.yaml
```

### For Testing DSC Component
```bash
cd extras/ESPHome
esphome run dsc_minimal_test.yaml
```

### For Hardware Troubleshooting
1. Use standalone mode first: `standalone_mode: true`
2. Verify basic boot stability
3. Then test with physical panel: `standalone_mode: false`

## Expected Results

### With Boot Fixes Applied
- No task watchdog timeout
- Successful transition from app_main() to ESPHome loop
- System uptime > 30 seconds without resets
- Memory and WiFi diagnostics working

### DSC Connection Diagnostics
- Enhanced logging every 30 seconds
- Real-time connection status monitoring
- Detailed wiring troubleshooting messages
- Manual connection test buttons

## Hardware Connections (When Ready)
- DSC Yellow (Clock) → ESP32 GPIO 18 (via 33kΩ resistor)
- DSC Green (Data) → ESP32 GPIO 19 (via 33kΩ resistor)
- DSC Black (Ground) → ESP32 GND
- DSC Brown (PC-16, Classic only) → ESP32 GPIO 17 (via 1kΩ resistor)

## Next Steps
1. Test basic boot diagnostic to confirm watchdog fixes
2. Test minimal DSC configuration in standalone mode
3. Resolve GPIO pin conflicts in full diagnostic
4. Test with physical DSC panel when ready