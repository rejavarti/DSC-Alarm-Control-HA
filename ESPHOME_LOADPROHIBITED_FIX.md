# ESPHome LoadProhibited Crash Fix

## Overview

This fix addresses ESP32 "Guru Meditation Error: Core 1 panic'ed (LoadProhibited)" crashes that occurred during DSC Keybus Interface initialization in ESPHome configurations. The crashes were caused by uninitialized static variables being accessed by interrupt service routines (ISRs) before proper object construction completed.

## Problem Symptoms

**Before the fix**, ESPHome users experienced:

```
Guru Meditation Error: Core  1 panic'ed (LoadProhibited). Exception was unhandled.
Core  1 register dump:
A2      : 0xa5a5a5a5  A3      : 0xa5a5a5a1  ...
EXCVADDR: 0xa5a5a5a5
```

The memory address `0xa5a5a5a5` is a typical uninitialized memory pattern indicating that ISRs were accessing garbage data.

## Solution Applied

### 1. ESPHome Component Static Variable Initialization

All static variables in the ESPHome component are now explicitly initialized to safe values in `extras/ESPHome/components/dsc_keybus/dsc_static_variables.cpp`:

#### DSC Classic Series Variables
```cpp
// Initialize static variables to prevent uninitialized access crashes (ESPHome LoadProhibited fix)
byte dscClassicInterface::dscClockPin = 255;
byte dscClassicInterface::dscReadPin = 255;
byte dscClassicInterface::dscPC16Pin = 255;
byte dscClassicInterface::dscWritePin = 255;
// ... all variables explicitly initialized with safe defaults
```

#### DSC PowerSeries Variables
```cpp
// Initialize static variables to prevent uninitialized access crashes (ESPHome LoadProhibited fix)
byte dscKeybusInterface::dscClockPin = 255;
byte dscKeybusInterface::dscReadPin = 255;
byte dscKeybusInterface::dscWritePin = 255;
// ... all variables explicitly initialized with safe defaults
```

#### DSC Keypad Interface Variables
Both `dscKeypadInterface` and `dscClassicKeypadInterface` classes also have all static variables properly initialized.

### 2. ESPHome Component Safety Features

The ESPHome component includes additional safety measures:

```cpp
// Watchdog timer management to prevent freezing
#ifdef ESP32
#include <esp_task_wdt.h>
// Reset watchdog during critical operations
esp_task_wdt_reset();
#endif
```

### 3. ESP-IDF Configuration

The test configurations include ESP-IDF optimizations:

```yaml
esp32:
  framework:
    type: esp-idf
    sdkconfig_options:
      CONFIG_ESP_MAIN_TASK_STACK_SIZE: "16384"  # Increase stack size
      CONFIG_ESP_TASK_WDT_TIMEOUT_S: "30"       # 30 second timeout
      CONFIG_ESP_TASK_WDT_EN: y                 # Enable watchdog
```

## Compatibility

### Supported Configurations

✅ **DSC PowerSeries Panels** (ESP32, ESP8266)
- PC1555MX, PC5015, PC1616, PC1832, PC1864, etc.

✅ **DSC Classic Series Panels** (ESP32, ESP8266) 
- PC1500, PC1550, PC1832, PC1864, PC1616, etc.
- Requires `series_type: Classic` and proper PC-16 wiring

### ESPHome YAML Configuration

#### PowerSeries Example
```yaml
dsc_keybus:
  access_code: "1234"
  debug: 0
  series_type: PowerSeries  # Default
```

#### Classic Series Example
```yaml
dsc_keybus:
  access_code: "1234" 
  debug: 0
  series_type: Classic
  pc16_pin: 17  # GPIO pin for PC-16 connection
```

## Testing

### Validation Scripts

The repository includes test configurations:
- `test_compile_esp32.yaml` - ESP32 with ESP-IDF
- `test_simple_powerseries.yaml` - Basic PowerSeries setup
- `test_simple_classic.yaml` - Basic Classic setup

### Memory Safety Checks

The component performs runtime validation:
```cpp
// Check available heap memory
size_t free_heap = esp_get_free_heap_size();
if (free_heap < 50000) {  // Less than 50KB free
  ESP_LOGW(TAG, "Low heap memory detected: %zu bytes free", free_heap);
}
```

## Migration

### For Existing ESPHome Users

**No configuration changes required!** The fix is applied at the component level and maintains full backward compatibility.

Your existing ESPHome YAML configurations will continue to work exactly as before, but now without LoadProhibited crashes.

### Recommended ESP-IDF Settings

For maximum stability, use these ESP-IDF configurations in your YAML:

```yaml
esp32:
  framework:
    type: esp-idf
    sdkconfig_options:
      CONFIG_ESP_MAIN_TASK_STACK_SIZE: "16384"
      CONFIG_ESP_TASK_WDT_TIMEOUT_S: "30"
      CONFIG_ESP_TASK_WDT_EN: y
```

## Impact

### Benefits
- ✅ **Eliminates LoadProhibited crashes** on ESP32 during DSC interface initialization
- ✅ **Improves startup reliability** with comprehensive memory safety
- ✅ **Zero configuration changes** required for existing setups
- ✅ **Maintains full backward compatibility** with existing YAML configurations

### Performance
- **Zero runtime overhead** - only affects initialization phase
- **No functional changes** - all DSC features work identically
- **Same memory footprint** - variables were always allocated, now just initialized

## Technical Details

### Root Cause Analysis

The LoadProhibited crashes occurred due to a race condition:
1. ESPHome component started DSC hardware initialization
2. Timer interrupts and ISRs were attached before static variables were initialized
3. ISR functions accessed uninitialized static variables containing garbage patterns like `0xa5a5a5a5`
4. Memory access violation triggered LoadProhibited exception

### Solution Architecture

1. **Explicit Static Initialization**: All static variables initialized with safe default values
2. **Deferred Hardware Init**: Hardware interrupts attached only after variable initialization
3. **Watchdog Management**: ESP32 watchdog timer properly managed during initialization
4. **Memory Validation**: Runtime heap and memory safety checks

The fix ensures that by the time any ISR can execute, all static variables contain valid, initialized data instead of random garbage values.