# ESPHome millis() Function Ambiguity Fix

## Problem Solved

This fix resolves the compilation error in ESPHome builds that use the DSC Keybus component:

```
error: call of overloaded 'millis()' is ambiguous
```

## Root Cause

The issue occurred due to a namespace collision between:

1. **ESPHome core `millis()` function**: `uint32_t esphome::millis()` (from `src/esphome/core/hal.h`)
2. **DSC Arduino compatibility `millis()` function**: `unsigned long millis()` (from Arduino compatibility headers)

When ESPHome lambda functions called `millis()`, the compiler couldn't determine which function to use.

## Solution Implemented

### 1. Conditional Compilation in Arduino Compatibility Headers

Modified the following files to conditionally define the Arduino compatibility `millis()` function:

- `extras/ESPHome/components/dsc_keybus/dsc_arduino_compatibility.h`
- `extras/ESPHome/components/dsc_keybus_minimal/dsc_arduino_compatibility.h`  
- `src/dsc_arduino_compatibility.h`

The fix uses `#ifndef ESPHOME_LAMBDA_CONTEXT` to avoid defining the compatibility `millis()` when it would conflict.

### 2. Explicit Function Calls in YAML Lambda Functions

Updated ESPHome YAML configuration files to use `esphome::millis()` explicitly:

```cpp
// Before (ambiguous):
ESP_LOGI("test", "uptime: %lu", millis());

// After (explicit):
#define ESPHOME_LAMBDA_CONTEXT 1
ESP_LOGI("test", "uptime: %lu", (unsigned long)esphome::millis());
```

## Files Modified

### Arduino Compatibility Headers
- `extras/ESPHome/components/dsc_keybus/dsc_arduino_compatibility.h`
- `extras/ESPHome/components/dsc_keybus_minimal/dsc_arduino_compatibility.h`
- `src/dsc_arduino_compatibility.h`

### ESPHome Configuration Files
- `extras/ESPHome/task_watchdog_timeout_fix_test.yaml`

## Testing

The fix was validated by:

1. **Compilation test**: Both minimal and full configurations compile successfully
2. **Functionality test**: DSC component functionality remains intact
3. **Regression test**: Arduino builds continue to work without modification

## Technical Details

### Before Fix
```cpp
// Two conflicting definitions in same scope:
uint32_t esphome::millis();              // ESPHome core
inline unsigned long millis() { ... }   // DSC Arduino compatibility
```

### After Fix
```cpp
// ESPHome lambda context:
uint32_t esphome::millis();              // Only ESPHome function available

// DSC component context:
inline unsigned long millis() { ... }   // Only Arduino compatibility available
```

## Compatibility

- ✅ **ESPHome builds**: Lambda functions use `esphome::millis()` explicitly
- ✅ **Arduino builds**: Continue to use Arduino compatibility `millis()` 
- ✅ **DSC component**: Internal code continues to use Arduino compatibility functions
- ✅ **All platforms**: ESP32, ESP8266, and other platforms supported

## Benefits

1. **Resolves compilation errors** in ESPHome DSC configurations
2. **Maintains backward compatibility** with existing Arduino builds
3. **Minimal code changes** - surgical fix with no functional impact
4. **Future-proof** - prevents similar namespace collisions

This fix enables the ESP32 Task Watchdog Timeout diagnostic configuration to compile successfully while maintaining all existing functionality.