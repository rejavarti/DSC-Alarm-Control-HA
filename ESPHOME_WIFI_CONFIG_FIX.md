# ESPHome WiFi Configuration Fix

## Issue Fixed
ESPHome 2025.7.5 validation error in `DSCAlarm_ESP_IDF_5_3_LoadProhibited_Fix.yaml`:

```
[connection_timeout] is an invalid option for [wifi]. Did you mean [reboot_timeout]?
connection_timeout: 60s
```

## Root Cause
The `connection_timeout` parameter was added to the WiFi configuration section, but this parameter is not valid in ESPHome. ESPHome uses `reboot_timeout` to handle WiFi connection timeouts.

## Solution Applied
1. Removed the invalid `connection_timeout: 60s` parameter from the WiFi configuration
2. Added explanatory comment that WiFi connection timeout is handled by the existing `reboot_timeout: 300s` parameter
3. No functional changes - the 300-second reboot timeout already provides adequate WiFi connection timeout functionality

## Files Modified
- `extras/ESPHome/DSCAlarm_ESP_IDF_5_3_LoadProhibited_Fix.yaml` (lines 173-175)

## Changes Made
```diff
- # CRITICAL FIX: Add connection timeout and retry settings to prevent watchdog timeout
- # during initial WiFi connection phase  
- connection_timeout: 60s    # Allow up to 60 seconds for initial connection
+ # Note: WiFi connection timeout is handled by reboot_timeout above (300s)
```

## Validation Results
✅ Configuration now passes ESPHome 2025.7.5 validation without errors
✅ WiFi timeout functionality preserved via existing `reboot_timeout: 300s`
✅ No other ESPHome configuration files in the repository contain this issue

## Impact
- **Zero functional impact**: WiFi connection timeout behavior remains unchanged
- **Improved compatibility**: Configuration now works with ESPHome 2025.7.5+  
- **Cleaner configuration**: Removed invalid parameter that was causing validation errors