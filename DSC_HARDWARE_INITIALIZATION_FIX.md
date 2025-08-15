# DSC Keybus Hardware Initialization Fix

## Problem Solved ✅
**Error**: `[E][dsc_keybus:546]: DSC Keybus hardware initialization has failed - component non-functional`

**Root Cause**: The system was incorrectly identifying normal ESP32 boot times (8.8 seconds) as restart loops and preventing hardware initialization.

## Solution Implemented

### 🔧 Key Changes Made

1. **Improved Restart Loop Detection** (`dsc_wrapper.cpp`)
   - Changed threshold from 10 seconds to 5 seconds
   - Requires multiple rapid attempts (≥3) within 1 second to flag as restart loop
   - Normal boot scenarios (8-10 seconds) now allowed

2. **Enhanced Diagnostics** (`dsc_wrapper.cpp`) 
   - Added detailed logging before hardware initialization
   - Shows attempt number, system uptime, and free heap memory
   - Helps distinguish software vs hardware issues

3. **Better Error Messages** (`dsc_keybus.cpp`)
   - Troubleshooting information with specific GPIO pins
   - Hardware checklist (power, resistors, connections)
   - Actionable steps for users

### 🧪 Test Results

**Before Fix:**
```
System uptime: 8791ms → ❌ BLOCKED (false positive)
Result: "DSC Keybus hardware initialization has failed"
```

**After Fix:**
```  
System uptime: 8791ms → ✅ ALLOWED (normal boot)
Result: Hardware initialization can proceed normally
```

**Actual Restart Loop (Still Detected):**
```
System uptime: 2000ms + 3 rapid attempts → ❌ BLOCKED (true positive) 
Result: Prevents LoadProhibited crash loops
```

## For Users

### If You Still See Hardware Initialization Failures:

The fix addresses software false positives. If initialization still fails, check:

1. **GPIO Connections**:
   - Clock pin (usually GPIO 18 on ESP32)
   - Read pin (usually GPIO 19 on ESP32)  
   - Write pin (usually GPIO 21 on ESP32)

2. **Hardware Requirements**:
   - DSC panel powered on
   - 33kΩ resistor (Clock to +12V)
   - 10kΩ resistor (Read to +12V)
   - Stable ESP32 power supply

3. **ESPHome Configuration**:
   - Correct pin assignments in YAML
   - Proper access code configured
   - ESP-IDF framework if using ESP32

### Expected Behavior After Fix:
- ✅ Normal 8-15 second boot times work
- ✅ Hardware initialization proceeds normally
- ✅ Better error messages when real hardware issues occur
- ✅ Still protects against actual crash loops

## Installation

The fix is already applied to the ESPHome component files. Simply:

1. Use the updated `dsc_keybus` component
2. Flash your ESP32 with ESPHome  
3. Monitor logs for successful initialization
4. If issues persist, check the hardware troubleshooting steps above