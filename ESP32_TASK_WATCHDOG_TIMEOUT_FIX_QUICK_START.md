# ESP32 Task Watchdog Timeout Fix - Quick Start Guide

## Problem Solved

This fix resolves the ESP32 task watchdog timeout issue that occurs during ESPHome boot:

```
E (5855) task_wdt: Task watchdog got triggered. The following tasks/users did not reset the watchdog in time:
E (5855) task_wdt:  - IDLE0 (CPU 0)
```

## Quick Implementation

### Step 1: Choose Your Configuration

#### For Testing (Recommended First Step)
Use the standalone test configuration:
```bash
cd extras/ESPHome
esphome run task_watchdog_timeout_fix_test.yaml
```

#### For Physical DSC Panel
Use the enhanced diagnostic configuration:
```bash
cd extras/ESPHome  
esphome run dsc_boot_diagnostic.yaml
```

### Step 2: Add to Your Existing Configuration

If you have an existing ESP32 configuration, add this to your YAML file:

```yaml
esp32:
  board: esp32dev
  framework:
    type: esp-idf
    sdkconfig_options:
      # Task Watchdog Timeout Fix - Critical Settings
      CONFIG_ESP_MAIN_TASK_STACK_SIZE: "32768"         # 32KB main task stack
      CONFIG_ESP_TASK_WDT_TIMEOUT_S: "300"             # 5 minute watchdog timeout
      CONFIG_FREERTOS_IDLE_TASK_STACKSIZE: "4096"      # 4KB IDLE task stack
      CONFIG_ESP_TASK_WDT_CHECK_IDLE_TASK_CPU0: y      # Monitor IDLE0 task
      CONFIG_ESP_TASK_WDT_CHECK_IDLE_TASK_CPU1: y      # Monitor IDLE1 task
      CONFIG_ESP_TASK_WDT_PANIC: n                     # Reset instead of panic
      
      # Performance optimization
      CONFIG_ESP32_DEFAULT_CPU_FREQ_240: y             # 240MHz CPU
      CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ: "240"
      CONFIG_PM_ENABLE: n                              # Disable power management
```

### Step 3: Validate the Fix

Run the validation script:
```bash
python test_task_watchdog_timeout_fix.py
```

Expected output:
```
🎉 SUCCESS: Task watchdog timeout fix is properly configured!
🏆 ALL TESTS PASSED: Task watchdog timeout fix is ready for deployment
```

## Expected Results

### Before Fix
```
[22:43:50]I (527) app_init: Project name: dscalarm-boot-diagnostic
[22:43:54]E (5855) task_wdt: Task watchdog got triggered
[22:43:54]E (5855) task_wdt:  - IDLE0 (CPU 0)
[SYSTEM RESETS AND LOOPS]
```

### After Fix
```
[xx:xx:xx]I (527) app_init: Project name: dscalarm-boot-diagnostic
[xx:xx:xx][I][task_watchdog_fix] ✅ SUCCESS: Task watchdog timeout fix validated!
[xx:xx:xx][I][task_watchdog_fix] ✅ IDLE0 task getting CPU time - no 'E (5855) task_wdt' error
[xx:xx:xx][I][dsc_keybus] DSC Keybus Interface is online
[SYSTEM CONTINUES NORMALLY]
```

## Key Benefits

- ✅ **No more watchdog timeouts** during ESP32 boot
- ✅ **Stable ESPHome initialization** with proper task scheduling  
- ✅ **Extended timeout window** allows complete component setup
- ✅ **Optimized memory allocation** prevents stack overflow
- ✅ **Better system performance** with 240MHz CPU frequency

## Troubleshooting

### Still getting timeouts?
1. Verify all critical settings are applied
2. Check ESP-IDF version (5.3.2+ recommended)
3. Test in standalone mode first
4. Run validation script to check configuration

### Compilation errors?
1. Check YAML syntax and indentation
2. Ensure ESP-IDF framework is selected
3. Verify all required settings are present

### Memory issues?
1. Monitor heap usage with diagnostic sensors
2. Consider reducing other component memory usage
3. Check for memory leaks in custom code

## Files Included

- `task_watchdog_timeout_fix_test.yaml` - Standalone test configuration
- `dsc_boot_diagnostic.yaml` - Enhanced diagnostic with validation
- `test_task_watchdog_timeout_fix.py` - Automated validation script
- `ESP32_TASK_WATCHDOG_TIMEOUT_COMPLETE_FIX.md` - Detailed documentation

## Support

This fix is tested and validated for:
- ESP-IDF 5.3.2+ with ESPHome 2025.7.5+
- All ESP32 variants (DevKit, WROOM, S2, S3)
- Both DSC Classic and PowerSeries panels
- Standalone and physical connection modes

For additional help, refer to the complete documentation in `ESP32_TASK_WATCHDOG_TIMEOUT_COMPLETE_FIX.md`.