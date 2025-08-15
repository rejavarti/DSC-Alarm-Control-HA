# DSC Alarm Physical Connection Troubleshooting Guide

## Issue: "The alarm is connected right now to the module, so can you see why it is not connecting properly?"

This guide addresses the specific scenario where you have a **physical DSC alarm panel connected** to your ESP32 module, but the system is still showing hardware initialization failures.

## Quick Diagnostic Steps

### 1. **Use the Diagnostic Configuration**

A specialized diagnostic configuration has been created: `DscAlarm_Physical_Connection_Diagnostic.yaml`

This configuration includes:
- ✅ **Maximum debug output** (`debug: 3`) - Shows all DSC packets
- ✅ **Pin state monitoring** - Real-time GPIO pin state checking  
- ✅ **Connection diagnostics** - Automated wiring troubleshooting
- ✅ **Enhanced logging** - Detailed connection status every 30 seconds
- ✅ **Manual test buttons** - Force connection tests

**To use**:
```bash
cd extras/ESPHome
esphome run DscAlarm_Physical_Connection_Diagnostic.yaml
```

### 2. **Verify Physical Wiring**

**Critical ESP32 connections** (most common cause of connection failures):

| DSC Wire Color | DSC Signal | ESP32 GPIO | Required Resistor | Notes |
|----------------|------------|------------|-------------------|-------|
| **Yellow** | Clock | **GPIO 18** | **33kΩ** | Clock signal from panel |
| **Green** | Data | **GPIO 19** | **33kΩ** | Bidirectional data |
| **Black** | Ground | **GND** | None | Common ground |
| **Red** | +12V | **NOT CONNECTED** | None | ⚠️ Do NOT connect to ESP32 |

**For DSC Classic series ONLY**:
| DSC Wire Color | DSC Signal | ESP32 GPIO | Required Resistor | Notes |
|----------------|------------|------------|-------------------|-------|
| **Brown** | PC-16 | **GPIO 17** | **1kΩ** | Classic series only |

### 3. **Common Wiring Issues**

#### ❌ **Issue**: No resistors installed
- **Symptoms**: Pin states show erratic behavior, no communication
- **Solution**: Install 33kΩ resistors on Clock (GPIO 18) and Data (GPIO 19) lines

#### ❌ **Issue**: Wrong resistor values  
- **Symptoms**: Intermittent communication, corrupted data
- **Solution**: Use exactly 33kΩ resistors (not 10kΩ, not 22kΩ)

#### ❌ **Issue**: ESP32 connected to DSC 12V power
- **Symptoms**: ESP32 damage, erratic behavior, boot loops
- **Solution**: Power ESP32 separately via USB or 3.3V/5V supply

#### ❌ **Issue**: Poor connections or loose wires
- **Symptoms**: Intermittent connection, sporadic initialization failures
- **Solution**: Use solid connections, solder if necessary, avoid breadboards for permanent installations

#### ❌ **Issue**: Wrong GPIO pins configured
- **Symptoms**: No communication despite correct wiring  
- **Solution**: Verify configuration matches actual wiring

### 4. **DSC Panel Series Configuration**

#### **DSC Classic Series** (PC1500, PC1550, PC1832, etc.)
- **Required**: Add build flag `-DdscClassicSeries` 
- **Required**: Connect PC-16 line (Brown wire → GPIO 17 via 1kΩ resistor)
- **Configuration**: `series_type: "Classic"`

#### **DSC PowerSeries** (PC1555, PC1616, PC1864, etc.)  
- **Required**: No special build flags
- **Not needed**: PC-16 connection  
- **Configuration**: `series_type: "PowerSeries"`

### 5. **Power Supply Issues**

#### **DSC Panel Power**
- **Required**: DSC panel must have stable 12V power supply
- **Check**: Panel keypad should be functioning normally
- **Test**: Keypad backlight on, can arm/disarm normally

#### **ESP32 Power**
- **Recommended**: Separate 5V USB power supply for ESP32
- **Avoid**: Powering ESP32 from DSC 12V line (requires voltage regulation)
- **Check**: ESP32 status LED should be solid (not blinking rapidly)

### 6. **Diagnostic Log Analysis**

When running the diagnostic configuration, look for these key log messages:

#### **✅ Good Signs (Communication Working)**:
```
[INFO] DSC Keybus hardware initialization complete
[INFO] DSC packet received: [05][01][XX][XX]...
[INFO] Keybus Connected: YES
[INFO] Pin State - Clock: HIGH, Data: HIGH
```

#### **❌ Bad Signs (Connection Issues)**:
```
[ERROR] DSC hardware initialization permanently failed after 5 attempts
[WARNING] Keybus Connected: NO  
[WARNING] Both pins LOW - check power to DSC panel
[WARNING] Clock pin LOW - possible wiring issue
```

### 7. **Step-by-Step Troubleshooting Process**

#### **Step 1**: Check DSC Panel Power
1. Verify DSC panel is powered and functioning
2. Test keypad - should show normal status displays
3. Confirm panel can arm/disarm normally

#### **Step 2**: Verify ESP32 Power and WiFi  
1. ESP32 connects to WiFi successfully
2. ESPHome API accessible from Home Assistant  
3. Status LED behavior is normal

#### **Step 3**: Physical Wiring Verification
1. **Clock line**: DSC Yellow → 33kΩ resistor → ESP32 GPIO 18
2. **Data line**: DSC Green → 33kΩ resistor → ESP32 GPIO 19  
3. **Ground**: DSC Black → ESP32 GND (direct connection)
4. **PC-16** (Classic only): DSC Brown → 1kΩ resistor → ESP32 GPIO 17

#### **Step 4**: Configuration Verification
1. Confirm `series_type` matches your actual DSC panel
2. For Classic: Verify `-DdscClassicSeries` build flag is enabled
3. Pin assignments in config match physical wiring
4. `standalone_mode: false` for physical panel connection

#### **Step 5**: Use Diagnostic Configuration
1. Flash `DscAlarm_Physical_Connection_Diagnostic.yaml`
2. Monitor logs for detailed diagnostic information
3. Check pin state sensors in Home Assistant
4. Use manual test buttons to verify connection

### 8. **Advanced Troubleshooting**

#### **Oscilloscope Testing** (if available)
- Clock line should show regular ~1kHz pulses when panel is active
- Data line should show varying digital signals during communication
- Voltage levels should be 0-5V logic levels (not 12V)

#### **Multimeter Testing**
- Clock pin (GPIO 18): Should read ~3.3V when idle
- Data pin (GPIO 19): Should read ~3.3V when idle  
- Ground continuity: ESP32 GND to DSC Black wire should be <1Ω
- Power isolation: DSC Red (+12V) should NOT be connected to ESP32

#### **Panel Compatibility Check**
- Verify your DSC panel model supports keybus communication
- Some very old or basic panels may not support keybus interface
- Check panel manual for keybus connector pinout

### 9. **Expected Behavior When Working**

Once properly connected, you should see:

```
[INFO] ESPHome version 2025.x.x compiled on...
[INFO] Connecting to WiFi...
[INFO] WiFi connected! IP: 192.168.x.x
[INFO] DSC Keybus hardware initialization starting...
[INFO] DSC packet received: [05][01][XX][XX]...
[INFO] DSC hardware initialization complete
[INFO] System status: Ready
[INFO] Partition 1 status: Ready
```

### 10. **If All Else Fails**

If you've verified all wiring and configuration but still can't establish communication:

1. **Try a different ESP32 board** - Hardware defect possibility
2. **Test with multimeter** - Verify voltage levels and continuity  
3. **Check DSC panel model compatibility** - Ensure keybus support
4. **Review panel documentation** - Verify keybus connector pinout
5. **Consider using a different keybus interface library** - Some panels have quirks

### 11. **Getting Additional Help**

When requesting help, include:
1. **DSC panel model number** (e.g., PC1555MX, PC1832, etc.)
2. **Complete diagnostic logs** from the diagnostic configuration
3. **Photos of physical wiring** showing connections and resistors
4. **ESP32 board model** (e.g., ESP32-DevKitC, NodeMCU-32S, etc.)
5. **Power supply details** for both DSC panel and ESP32

This diagnostic approach should identify the specific cause of your physical connection issue and provide clear steps to resolve it.