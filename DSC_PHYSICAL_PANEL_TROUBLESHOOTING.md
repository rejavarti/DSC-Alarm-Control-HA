# DSC Physical Panel Connection Troubleshooting Guide

## Problem Description

User reports: **"The panel is already connected to the physical module and it all worked with the original dscKeybusInterface files. I don't need standalone mode engaged, I want to get this communicating properly."**

**Error Observed:**
```
[11:48:20][E][dsc_keybus:296]: DSC hardware initialization permanently failed after 5 attempts - stopping retries
[11:48:20][W][dsc_keybus:297]: If no DSC panel is connected, enable standalone_mode: true in your configuration
```

## Root Cause Analysis

The hardware initialization failure with a connected physical panel can be caused by several factors:

1. **Hardware Connection Issues** - Most common cause
2. **Timing/Interrupt Conflicts** - ESPHome vs Arduino framework differences  
3. **Power Supply Problems** - Insufficient or unstable power
4. **Component Configuration** - ESP-IDF vs Arduino differences
5. **Environmental Interference** - Electrical noise or grounding issues

## Solution Steps

### 1. Hardware Verification Checklist

#### A. Power Supply Verification
- [ ] **DSC Panel Power**: Verify 12V DC supply to DSC alarm panel
- [ ] **ESP32 Power**: Ensure stable 5V or 3.3V power supply to ESP32
- [ ] **Power Quality**: Check for power supply noise with oscilloscope if available
- [ ] **Grounding**: Verify common ground between DSC panel and ESP32

#### B. Wiring Connections (Critical)
```
DSC Keybus → ESP32 GPIO
=======================
Yellow (CLK) → GPIO 18 (clock_pin)
Green (DATA) → GPIO 19 (read_pin)  
Yellow (CMD) → GPIO 21 (write_pin)
Black (GND)  → ESP32 GND
Red (+12V)   → NOT CONNECTED (ESP32 uses 3.3V logic)

For Classic Series ONLY:
Brown (PC16) → GPIO 17 (pc16_pin) via 1kΩ resistor
```

#### C. Resistor Requirements (Essential)
**For PowerSeries Panels:**
- 33kΩ resistor between CLK (Yellow) and DATA (Green) lines
- 10kΩ resistor from DATA (Green) to GND (Black)

**For Classic Series Panels (your configuration):**
- Same resistors as PowerSeries PLUS
- 1kΩ resistor for PC16 connection (Brown wire)

#### D. Signal Integrity Checks
- [ ] **Connection Quality**: Ensure all solder joints are solid
- [ ] **Wire Length**: Keep connections under 6 inches when possible
- [ ] **Shielding**: Use shielded cable for longer runs
- [ ] **Interference**: Check for nearby high-power devices

### 2. Voltage Testing with Multimeter

**Expected Readings:**
```
Between DSC Yellow (CLK) and Black (GND): ~12V DC
Between DSC Green (DATA) and Black (GND): Variable voltage (indicates bus activity)
ESP32 GPIO pins: 3.3V logic levels (0V = LOW, 3.3V = HIGH)
```

**Troubleshooting Readings:**
- **0V on CLK line**: DSC panel not powered or keybus not active
- **Constant voltage on DATA**: No keybus communication/activity
- **5V on ESP32 GPIO**: Wrong voltage level - needs level shifting

### 3. DSC Panel Status Verification

#### A. Panel Operation Check
- [ ] **Panel Display**: Should show normal ready/armed status
- [ ] **Keypad Response**: Test with existing keypads (should work normally)
- [ ] **Bus Activity**: Other devices on keybus should function
- [ ] **Programming Mode**: Ensure panel is NOT in programming mode

#### B. Common Panel Issues
- **Bus Fault Indication**: Check panel display for bus fault codes
- **Zone Supervision**: Verify no zone trouble conditions
- **Communication Issues**: Test existing keypad communication

### 4. ESPHome Configuration Verification

Your current configuration looks correct:
```yaml
dsc_keybus:
  series_type: "Classic"        ✅ Correct for Classic panels
  clock_pin: "18"              ✅ Good GPIO choice
  read_pin: "19"               ✅ Good GPIO choice  
  write_pin: "21"              ✅ Good GPIO choice
  pc16_pin: "17"               ✅ Required for Classic series
  standalone_mode: false       ✅ Correct for physical panel
```

### 5. Advanced Debugging Steps

#### A. Enable Maximum Debug Logging
Add to your configuration:
```yaml
logger:
  level: DEBUG
  logs:
    dsc_keybus: DEBUG
    esp_timer: DEBUG
    system_api: DEBUG
```

#### B. Monitor Serial Output
```bash
# Flash debug configuration
esphome run user_dsc_config_fixed.yaml

# Monitor logs in real-time
esphome logs user_dsc_config_fixed.yaml
```

#### C. Key Log Messages to Watch For
```
SUCCESS PATTERN:
[INFO][dsc_keybus:XXX]: Setting up DSC Keybus Interface...
[INFO][dsc_keybus:XXX]: DSC Keybus hardware initialization complete

FAILURE PATTERN:
[ERROR][dsc_keybus:296]: DSC hardware initialization permanently failed after 5 attempts
```

### 6. Comparison with Original dscKeybusInterface

Since you mentioned it worked with the original library, here are key differences:

#### A. Framework Differences
| Aspect | Original Arduino | ESPHome ESP-IDF |
|--------|------------------|-----------------|
| **Timing** | Arduino timing functions | ESP-IDF FreeRTOS timing |
| **Interrupts** | Arduino interrupt handling | ESP-IDF interrupt system |
| **Memory** | Arduino heap management | ESP-IDF heap allocator |
| **Scheduling** | Cooperative multitasking | Preemptive multitasking |

#### B. Configuration Mapping
If your original Arduino code worked, compare these settings:

**Arduino Sketch Pin Assignments:**
```cpp
#define dscClockPin 18        // GPIO 18 → clock_pin: "18"  
#define dscReadPin  19        // GPIO 19 → read_pin: "19"
#define dscWritePin 21        // GPIO 21 → write_pin: "21"
#define dscPC16Pin  17        // GPIO 17 → pc16_pin: "17"
```

### 7. Minimal Test Configuration

Create a minimal configuration for testing:

```yaml
# Remove these temporarily for testing:
# - mqtt: (comment out entire section)
# - api: (comment out entire section)  
# - wifi: (keep only basic connection)

# Keep only essential for DSC testing:
dsc_keybus:
  id: dsc_interface
  access_code: "1234"  # Use a simple test code
  series_type: "Classic"
  clock_pin: "18"
  read_pin: "19"
  write_pin: "21"
  pc16_pin: "17"
  standalone_mode: false
  debug: 3  # Maximum debug output
```

### 8. Timing Adjustments

If timing issues persist, try these ESP-IDF adjustments:

```yaml
esp32:
  framework:
    type: esp-idf
    sdkconfig_options:
      # Enhanced timing for DSC communication
      CONFIG_FREERTOS_HZ: "1000"                    # 1ms tick rate
      CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ: "240"      # Maximum CPU speed
      CONFIG_PM_ENABLE: n                           # Disable power management
      CONFIG_ESP_TASK_WDT_TIMEOUT_S: "300"          # Longer watchdog timeout
```

### 9. Hardware Isolation Testing

#### A. Oscilloscope Analysis (if available)
Monitor the DSC keybus signals:
- **CLK line**: Should show regular clock pulses (~1kHz)
- **DATA line**: Should show data transmission packets
- **Both**: Should show clear 12V logic levels

#### B. Logic Analyzer Testing
Use a logic analyzer to capture:
- Clock timing accuracy
- Data packet structure  
- Communication protocol compliance

### 10. Progressive Testing Approach

1. **Start Simple**: Test with minimal configuration
2. **Verify Basics**: Confirm hardware connections with multimeter
3. **Check Signals**: Use oscilloscope if available
4. **Add Complexity**: Gradually add features back
5. **Monitor Logs**: Watch for specific failure points

## Quick Reference Commands

```bash
# Generate debug configuration
python3 dsc_hardware_troubleshooting.py user_dsc_config_fixed.yaml

# Flash debug configuration  
esphome run user_dsc_config_fixed_debug.yaml

# Monitor real-time logs
esphome logs user_dsc_config_fixed.yaml

# Validate configuration
esphome config user_dsc_config_fixed.yaml
```

## Success Indicators

You'll know the connection is working when you see:
```
[INFO][dsc_keybus:XXX]: Setting up DSC Keybus Interface...
[INFO][dsc_keybus:XXX]: DSC Keybus hardware initialization complete
[INFO][dsc_keybus:XXX]: Panel online
```

## Still Having Issues?

If problems persist after following this guide:

1. **Double-check hardware connections** - Most issues are wiring-related
2. **Verify resistor values** - Incorrect resistors cause communication failures  
3. **Test with original Arduino code** - Confirm hardware setup works
4. **Check for environmental interference** - Move away from high-power devices
5. **Try different GPIO pins** - Some pins may have conflicts

The key is methodical troubleshooting - start with the hardware basics and work up to software configuration.