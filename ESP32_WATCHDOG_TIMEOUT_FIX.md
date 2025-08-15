# ESP32 Watchdog Timeout Fix for DSC Keybus Interface

## Problem Summary
The ESP32 task watchdog timeout was occurring during DSC Classic series hardware initialization, causing the system to crash and restart. The error logs showed:

```
E (6170) task_wdt: Task watchdog got triggered. The following tasks/users did not reset the watchdog in time:
E (6170) task_wdt:  - IDLE0 (CPU 0)
E (6170) task_wdt: Tasks currently running:
E (6170) task_wdt: CPU 0: loopTask
E (6170) task_wdt: CPU 1: IDLE1
E (6170) task_wdt: Aborting.
```

## Root Cause Analysis
The watchdog timeout occurred because:
1. DSC hardware initialization includes blocking operations (timer setup, interrupt configuration)
2. Timer initialization includes retry loops with `delay(10)` calls
3. No watchdog resets were present during these critical initialization steps
4. The main loop task was blocked for too long without resetting the watchdog

## Solution Implementation

### Watchdog Reset Placement Strategy
Added strategic `esp_task_wdt_reset()` calls at key points during DSC initialization:

1. **Start of hardware initialization** - Reset watchdog before beginning
2. **Timer retry loops** - Reset during and after each retry attempt  
3. **Timer configuration** - Reset after timer setup is complete
4. **Interrupt attachment** - Reset before critical interrupt configuration
5. **End of initialization** - Final reset after successful completion
6. **Stabilization delays** - Reset during system stabilization periods
7. **Memory checks** - Reset before and after heap validation

### Files Modified

#### 1. `extras/ESPHome/components/dsc_keybus/dscClassic.cpp`
- Added ESP32 watchdog include: `#include <esp_task_wdt.h>`
- Added watchdog resets during timer initialization retry loops
- Added watchdog reset before interrupt attachment
- Added final watchdog reset after successful initialization

#### 2. `extras/ESPHome/components/dsc_keybus/dscKeybusInterface.cpp` 
- Added watchdog reset at start of PowerSeries interface initialization
- Added watchdog resets during timer retry loops
- Added watchdog reset after timer configuration
- Added watchdog reset before interrupt attachment
- Added final watchdog reset after successful initialization

#### 3. `extras/ESPHome/components/dsc_keybus/dsc_wrapper.cpp`
- Added watchdog resets before memory safety checks
- Added watchdog reset before calling hardware initialization
- Added watchdog reset after hardware initialization attempt

#### 4. `extras/ESPHome/components/dsc_keybus/dsc_keybus.cpp`
- Added watchdog resets during stabilization delay periods
- Added watchdog reset before critical `getDSC().begin()` call
- Added watchdog reset after hardware initialization attempt

### Platform Compatibility
All watchdog resets are properly guarded with ESP32-specific conditionals:
```cpp
#if defined(ESP32) || defined(ESP_PLATFORM)
esp_task_wdt_reset();
#endif
```

This ensures:
- ESP32 platforms get watchdog protection
- ESP8266 and other platforms are unaffected
- No compilation errors on non-ESP32 platforms

### Key Improvements

1. **Prevents Watchdog Timeouts**: Regular resets prevent the 60-second timeout
2. **Maintains DSC Timing**: Watchdog resets don't interfere with keybus timing
3. **Comprehensive Coverage**: All blocking initialization steps are protected
4. **Zero Functional Impact**: DSC functionality remains identical
5. **Platform Safe**: Only activates on ESP32 where needed

## Expected Results

With this fix implemented:
- ESP32 will no longer crash with watchdog timeouts during DSC initialization
- System will complete hardware initialization successfully
- DSC keybus interface will function normally
- No impact on ESP8266 or other platforms

## Testing Validation

- Compilation test passed for both Classic and PowerSeries interfaces
- Watchdog reset placement validated with test simulation
- ESP32-only conditional compilation verified
- All existing ESPHome validation checks pass

## Technical Notes

- Watchdog timeout configured to 60 seconds in YAML: `CONFIG_ESP_TASK_WDT_TIMEOUT_S: "60"`
- Watchdog resets are lightweight operations with minimal CPU overhead
- Timer initialization includes up to 3 retry attempts with 10ms delays each
- Stabilization periods can last 2-3 seconds, requiring periodic resets
- Memory validation and heap checks also require watchdog management

This fix addresses the specific issue reported in the problem statement while maintaining full compatibility and safety for all supported platforms.