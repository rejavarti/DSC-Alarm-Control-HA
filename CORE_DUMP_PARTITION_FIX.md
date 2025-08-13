# ESP32 Core Dump Partition Error Fix

## Issue Resolved

**Error**: `E (593) esp_core_dump_flash: No core dump partition found!`

**Impact**: This error occurred during ESP32 boot initialization and potentially contributed to subsequent memory allocation failures and system instability.

## Root Cause

The ESP32 configurations enabled core dump saving to flash memory (`CONFIG_ESP32_ENABLE_COREDUMP_TO_FLASH: y`) but used the default partition table (`CONFIG_PARTITION_TABLE_CUSTOM: n`) which doesn't include a dedicated core dump partition.

## Solution Applied

**Changed core dump destination from flash to UART:**

### Before (Problematic Configuration)
```yaml
CONFIG_ESP32_ENABLE_COREDUMP_TO_FLASH: y           # Save core dumps to flash
CONFIG_ESP32_COREDUMP_DATA_FORMAT_ELF: y           # ELF format for analysis
CONFIG_PARTITION_TABLE_CUSTOM: n                   # Use default partition table
```
**Result**: `No core dump partition found!` error during boot

### After (Fixed Configuration)
```yaml
CONFIG_ESP32_ENABLE_COREDUMP_TO_UART: y            # Save core dumps to UART output
CONFIG_ESP32_ENABLE_COREDUMP_TO_FLASH: n           # Disable flash dumps (no partition available)
CONFIG_ESP32_COREDUMP_DATA_FORMAT_ELF: y           # ELF format for analysis
CONFIG_PARTITION_TABLE_CUSTOM: n                   # Use default partition table
```
**Result**: No partition error, core dumps output to serial console

## Files Fixed

1. **`extras/ESPHome/DSCAlarm_ESP32_Classic.yaml`** - Main ESP32 Classic configuration
2. **`extras/ESPHome/DSCAlarm_ESP_IDF_5_3_LoadProhibited_Fix.yaml`** - ESP-IDF 5.3 fix configuration
3. **`ESPHOME_ESP32_LOADPROHIBITED_COMPLETE_FIX.md`** - Updated documentation

## Expected Boot Log Change

### Before Fix
```
[12:20:44]I (593) esp_core_dump_flash: Init core dump to flash
[12:20:44]E (593) esp_core_dump_flash: No core dump partition found!
[12:20:44]E (599) esp_core_dump_flash: No core dump partition found!
```

### After Fix
```
[12:20:44]I (593) esp_core_dump_uart: Init core dump to UART
[12:20:44]I (593) esp_core_dump_uart: Core dump to UART initialized
```

## Benefits of This Fix

✅ **Eliminates Boot Error**: No more "No core dump partition found!" error  
✅ **Maintains Crash Analysis**: Core dumps still captured via UART output  
✅ **No Partition Changes**: Works with existing default partition table  
✅ **Minimal Configuration**: Only 2 lines changed per configuration file  
✅ **Backward Compatible**: Doesn't affect existing functionality  
✅ **Easy Debugging**: Core dumps appear in serial console output  

## How to Use

1. Use the updated configurations:
   - `DSCAlarm_ESP32_Classic.yaml` for DSC Classic series panels
   - `DSCAlarm_ESP_IDF_5_3_LoadProhibited_Fix.yaml` for ESP-IDF 5.3+ compatibility

2. Core dumps will now appear in your serial console output when crashes occur

3. Use ESP32 exception decoder tools to analyze the UART-based core dumps

## Testing

All existing validation scripts pass:
- ✅ ESPHome LoadProhibited fix validation
- ✅ ESP-IDF 5.3 compatibility validation  
- ✅ Configuration syntax validation

## Alternative Solutions

If you prefer flash-based core dumps, you can:
1. Create a custom partition table with a core dump partition
2. Set `CONFIG_PARTITION_TABLE_CUSTOM: y`
3. Reference the custom partition table in your configuration

However, the UART-based solution is simpler and works immediately without partition table modifications.