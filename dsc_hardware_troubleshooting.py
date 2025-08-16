#!/usr/bin/env python3
"""
DSC Hardware Connection Troubleshooting Script

This script helps diagnose hardware initialization failures when a physical DSC panel
is connected but the ESP32 cannot communicate with it.

Based on user feedback: "The panel is already connected to the physical module. 
and it all worked with the original dscKeybusInterface files"
"""

import os
import sys
import yaml
import re

def analyze_raw_config(content):
    """Analyze ESPHome configuration by parsing raw content"""
    issues_found = []
    recommendations = []
    
    # Extract pin assignments from substitutions
    clock_pin = extract_value(content, r'clock_pin:\s*["\']?(\d+)["\']?')
    read_pin = extract_value(content, r'read_pin:\s*["\']?(\d+)["\']?')
    write_pin = extract_value(content, r'write_pin:\s*["\']?(\d+)["\']?')
    pc16_pin = extract_value(content, r'pc16_pin:\s*["\']?(\d+)["\']?')
    
    print(f"📌 Pin Configuration:")
    print(f"   Clock Pin (CLK): {clock_pin}")
    print(f"   Read Pin (DATA): {read_pin}")
    print(f"   Write Pin (YELLOW): {write_pin}")
    print(f"   PC16 Pin (Classic only): {pc16_pin}")
    
    # Check for pin conflicts
    pins = [clock_pin, read_pin, write_pin]
    if pc16_pin:
        pins.append(pc16_pin)
    
    valid_pins = [int(p) for p in pins if p and p.isdigit()]
    
    if len(valid_pins) != len(set(valid_pins)):
        issues_found.append("Pin conflict detected - same pin used for multiple functions")
    
    # Check for reserved pins
    reserved_pins = [0, 1, 6, 7, 8, 9, 10, 11]
    for pin in valid_pins:
        if pin in reserved_pins:
            issues_found.append(f"Pin {pin} is reserved and may cause issues")
    
    # Check standalone mode
    standalone_match = re.search(r'standalone_mode:\s*(true|false)', content)
    if standalone_match:
        standalone_mode = standalone_match.group(1) == 'true'
        if standalone_mode:
            issues_found.append("standalone_mode is set to 'true' but you have a physical panel connected")
            recommendations.append("Set standalone_mode: false for physical DSC panel connections")
        else:
            print("✅ Standalone mode correctly disabled for physical panel")
    
    # Check series type
    series_match = re.search(r'series_type:\s*["\']?(\w+)["\']?', content)
    if series_match:
        series_type = series_match.group(1)
        print(f"🔧 Panel Type: {series_type}")
        
        if series_type == "Classic" and not pc16_pin:
            issues_found.append("Classic series panel requires pc16_pin configuration")
            recommendations.append("Add pc16_pin: (GPIO number) for Classic series panels")
    
    # Check ESP-IDF framework
    if 'type: esp-idf' in content:
        print("✅ Using ESP-IDF framework (recommended for DSC)")
        
        # Check CPU frequency
        cpu_freq_match = re.search(r'CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ:\s*["\']?(\d+)["\']?', content)
        if cpu_freq_match:
            cpu_freq = int(cpu_freq_match.group(1))
            if cpu_freq < 240:
                issues_found.append(f"CPU frequency {cpu_freq}MHz may be too low for DSC timing")
                recommendations.append("Set CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ: '240' for optimal DSC timing")
        
        # Check power management
        if 'CONFIG_PM_ENABLE: y' in content:
            issues_found.append("Power management should be disabled for consistent DSC timing")
            recommendations.append("Set CONFIG_PM_ENABLE: n in sdkconfig_options")
    else:
        issues_found.append("Arduino framework detected - ESP-IDF recommended for DSC")
        recommendations.append("Consider switching to ESP-IDF framework for better DSC compatibility")
    
    # Check external component path
    if 'extras/ESPHome/components' in content:
        print("✅ DSC component path looks correct")
    else:
        issues_found.append("DSC component path may be incorrect")
        recommendations.append("Ensure external component path points to extras/ESPHome/components")
    
    print("\n" + "=" * 60)
    
    if issues_found:
        print("❌ CONFIGURATION ISSUES FOUND:")
        for i, issue in enumerate(issues_found, 1):
            print(f"   {i}. {issue}")
    else:
        print("✅ No major configuration issues detected")
    
    if recommendations:
        print("\n💡 RECOMMENDATIONS:")
        for i, rec in enumerate(recommendations, 1):
            print(f"   {i}. {rec}")
    
    return len(issues_found) == 0

def extract_value(content, pattern):
    """Extract a value using regex pattern"""
    match = re.search(pattern, content)
    return match.group(1) if match else None

def check_configuration(config_file):
    """Check DSC configuration for common hardware issues"""
    print("🔍 ANALYZING DSC CONFIGURATION")
    print("=" * 60)
    
    if not os.path.exists(config_file):
        print(f"❌ Configuration file not found: {config_file}")
        return False
    
    try:
        with open(config_file, 'r') as f:
            content = f.read()
            
        # For analysis purposes, we'll parse the raw content instead of YAML
        # since ESPHome configs have !secret directives that YAML can't parse
        print("✅ Configuration file found and readable")
        
        # Extract key values using regex since YAML parsing fails on !secret
        config = {}
        
        # Extract substitutions and DSC config by parsing raw content
        return analyze_raw_config(content)
        
    except Exception as e:
        print(f"❌ Error reading configuration: {e}")
        return False
    
    issues_found = []
    recommendations = []
    
    # Check DSC keybus configuration
    dsc_config = config.get('dsc_keybus', {})
    
    # 1. Check pin assignments
    clock_pin = dsc_config.get('clock_pin')
    read_pin = dsc_config.get('read_pin')  
    write_pin = dsc_config.get('write_pin')
    pc16_pin = dsc_config.get('pc16_pin')

def check_hardware_connections():
    """Provide hardware connection troubleshooting guide"""
    print("\n🔌 HARDWARE CONNECTION TROUBLESHOOTING")
    print("=" * 60)
    
    print("""
📋 CHECKLIST FOR PHYSICAL DSC PANEL CONNECTIONS:

1. POWER SUPPLY:
   ✓ DSC alarm panel is powered ON
   ✓ ESP32 has stable 5V or 3.3V power supply
   ✓ No power supply noise or instability

2. WIRING CONNECTIONS:
   ✓ Clock (CLK) - Yellow wire from DSC keybus
   ✓ Data (DATA) - Green wire from DSC keybus  
   ✓ Ground (GND) - Black wire from DSC keybus
   ✓ All connections are secure and properly soldered

3. RESISTOR VALUES (CRITICAL):
   For PowerSeries panels:
   ✓ 33kΩ resistor between CLK and DATA lines
   ✓ 10kΩ resistor from DATA to ground
   
   For Classic series panels:
   ✓ Same resistors as PowerSeries PLUS
   ✓ 1kΩ resistor for PC16 connection

4. SIGNAL INTEGRITY:
   ✓ No loose connections or cold solder joints
   ✓ Wires are not too long (keep under 6 inches if possible)
   ✓ No interference from other devices

5. DSC PANEL STATUS:
   ✓ Panel is not in programming mode
   ✓ Panel keybus is active (other keypads work)
   ✓ No bus faults showing on panel display

VOLTAGE MEASUREMENTS (with multimeter):
- DSC keybus should show ~12V DC between yellow and black
- Data line should show varying voltage (indicates activity)
- ESP32 GPIO pins should read 3.3V logic levels
""")

def check_original_dsckeybusinterface():
    """Check for differences from original dscKeybusInterface"""
    print("\n🔄 ORIGINAL dscKeybusInterface COMPATIBILITY")
    print("=" * 60)
    
    print("""
Since you mentioned it worked with the original dscKeybusInterface library,
here are key differences to check:

1. TIMING DIFFERENCES:
   - ESPHome uses different timing than Arduino sketches
   - ESP-IDF framework has stricter timing requirements
   - Solution: Verify CONFIG_FREERTOS_HZ: "1000" for 1ms ticks

2. INTERRUPT HANDLING:
   - ESPHome handles interrupts differently
   - May need to adjust GPIO interrupt configuration
   - Solution: Check pin assignments don't conflict with ESPHome internals

3. LIBRARY VERSION:
   - Ensure using compatible dscKeybusInterface version
   - Some features may be disabled in ESPHome port
   - Solution: Verify all needed features are implemented

4. MEMORY MANAGEMENT:
   - ESP-IDF has different memory allocation patterns
   - May need larger heap allocation
   - Solution: Check CONFIG_ESP_MAIN_TASK_STACK_SIZE: "32768"

DEBUGGING STEPS:
1. Enable maximum debug logging:
   logger:
     level: DEBUG
     logs:
       dsc_keybus: DEBUG

2. Monitor serial output during startup for specific error messages

3. Compare working Arduino code pin assignments with ESPHome config

4. Test with minimal configuration (remove MQTT, WiFi temporarily)
""")

def generate_debug_config(config_file):
    """Generate a debug configuration for hardware troubleshooting"""
    print("\n🛠️  GENERATING DEBUG CONFIGURATION")
    print("=" * 60)
    
    debug_config_file = config_file.replace('.yaml', '_debug.yaml')
    
    try:
        with open(config_file, 'r') as f:
            content = f.read()
        
        # Enable debug logging
        content = re.sub(r'level:\s*\w+', 'level: DEBUG', content)
        
        # Add specific DSC debug logging
        if 'logs:' not in content:
            content = content.replace(
                'logger:\n  level: DEBUG',
                '''logger:
  level: DEBUG
  logs:
    dsc_keybus: DEBUG
    esp_timer: DEBUG
    system_api: DEBUG'''
            )
        
        # Increase debug verbosity in DSC component
        content = re.sub(r'debug:\s*\d+', 'debug: 3', content)
        
        # Add hardware troubleshooting comment
        debug_header = '''# DEBUG CONFIGURATION FOR DSC HARDWARE TROUBLESHOOTING
# This configuration enables maximum debugging for hardware connection issues
# Monitor serial output at 115200 baud for detailed initialization logs
#
# Look for these key messages:
# - "Setting up DSC Keybus Interface..."
# - "DSC Keybus hardware initialization complete" (SUCCESS)
# - "DSC hardware initialization permanently failed" (FAILURE)
#
'''
        
        content = debug_header + content
        
        with open(debug_config_file, 'w') as f:
            f.write(content)
        
        print(f"✅ Debug configuration created: {debug_config_file}")
        print(f"📝 Flash this configuration and monitor serial output at 115200 baud")
        print(f"🔍 Look for specific hardware initialization messages")
        
        return debug_config_file
        
    except Exception as e:
        print(f"❌ Error generating debug config: {e}")
        return None

def main():
    print("🚨 DSC HARDWARE CONNECTION TROUBLESHOOTING TOOL")
    print("=" * 60)
    print("User report: 'Panel is connected, worked with original dscKeybusInterface'")
    print("Issue: Hardware initialization failing with physical panel connected")
    print()
    
    config_file = 'user_dsc_config_fixed.yaml'
    
    if len(sys.argv) > 1:
        config_file = sys.argv[1]
    
    # Check if configuration is correct
    config_ok = check_configuration(config_file)
    
    # Provide hardware troubleshooting guide
    check_hardware_connections()
    
    # Check compatibility with original library
    check_original_dsckeybusinterface()
    
    # Generate debug configuration
    debug_config = generate_debug_config(config_file)
    
    print("\n" + "=" * 60)
    print("🎯 NEXT STEPS:")
    print("=" * 60)
    
    if not config_ok:
        print("1. ❌ Fix configuration issues listed above")
        print("2. 🔧 Verify hardware connections using checklist")
        print("3. 🛠️  Flash debug configuration and monitor serial output")
    else:
        print("1. ✅ Configuration looks correct")
        print("2. 🔧 Focus on hardware connections - check wiring and resistors")
        print("3. 📊 Use debug configuration to capture detailed logs")
        print("4. 🔄 Compare setup with original working dscKeybusInterface code")
    
    print(f"\nTo run debug configuration:")
    if debug_config:
        print(f"esphome run {debug_config}")
    print("\nMonitor output with:")
    print("esphome logs user_dsc_config_fixed.yaml")

if __name__ == "__main__":
    main()