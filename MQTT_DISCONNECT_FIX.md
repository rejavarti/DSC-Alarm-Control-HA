# MQTT Disconnect Fix

## Problem
When accessing the configuration webpage on the DSC Alarm Control system, MQTT would disconnect and the webpage would not load properly. This occurred because the system would enter "configuration mode" and exclusively handle web server requests while completely stopping MQTT operations.

## Root Cause
In the `HomeAssistant-MQTT.ino` file, the main `loop()` function had an exclusive handling pattern:

**Before (Broken):**
```cpp
void loop() {
  // If in configuration mode, handle the web server
  if (configMode) {
    configServer.handleClient();
    return;  // This prevents MQTT from being handled!
  }

  mqttHandle();
  // ... rest of loop
}
```

When `configMode` was true (i.e., when someone accessed the config webpage), the function would:
1. Handle web server requests
2. Return immediately 
3. **Never call `mqttHandle()`**

This caused MQTT connections to timeout after ~15 seconds due to missing keep-alive packets.

## Solution
The fix ensures that MQTT operations continue even when the configuration portal is being accessed:

**After (Fixed):**
```cpp
void loop() {
  // Always handle MQTT operations to maintain connection
  mqttHandle();
  
  // If in configuration mode, handle the web server
  if (configMode) {
    configServer.handleClient();
    return;  // Skip DSC operations in config mode, but MQTT continues
  }
  // ... rest of loop
}
```

Now the function:
1. **Always calls `mqttHandle()` first** - maintaining the MQTT connection
2. Handles web server requests when in config mode
3. Skips DSC operations in config mode (which is appropriate)

## Benefits
- ✅ MQTT connections remain stable when accessing config portal
- ✅ Configuration webpage loads properly without timeouts
- ✅ No interruption to Home Assistant integration
- ✅ Minimal code change with maximum impact
- ✅ Maintains all existing functionality

## Testing
The fix was validated with:
1. Syntax compilation testing
2. Behavioral simulation showing MQTT keep-alives continue during config mode
3. Code review confirming the change addresses the root cause

This simple reordering of operations solves the MQTT disconnection issue while preserving all other functionality.