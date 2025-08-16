#!/usr/bin/env python3
"""
DSC Classic Series Hardware Connection Diagnostic Tool

This script helps diagnose why a DSC Classic panel is not connecting properly
to the ESP32 interface when the panel is physically connected.

Based on the user's feedback that the same hardware worked with the original
taligentx/dscKeybusInterface library, this focuses on configuration and
timing issues specific to the ESPHome implementation.
"""

import yaml
import sys
import os

def analyze_classic_configuration():
    """Analyze the DSC Classic configuration for common connectivity issues"""
    
    config_file = "user_dsc_config_fixed_debug.yaml"
    
    if not os.path.exists(config_file):
        print(f"❌ Configuration file {config_file} not found")
        return False
    
    print("🔍 DSC Classic Series Hardware Connection Analysis")
    print("=" * 60)
    
    try:
        with open(config_file, 'r') as f:
            content = f.read()
        
        # Check series type
        if 'series_type: "Classic"' in content:
            print("✅ Panel type: DSC Classic Series (correct)")
        else:
            print("❌ Panel type: Not configured for Classic series")
            print("   Fix: Ensure 'series_type: \"Classic\"' is set")
            return False
        
        # Check standalone mode
        if 'standalone_mode: false' in content:
            print("✅ Standalone mode: Disabled (correct for physical panel)")
        elif 'standalone_mode: true' in content:
            print("❌ Standalone mode: Enabled (wrong for physical panel)")
            print("   Fix: Set 'standalone_mode: false' for physical panel")
            return False
        else:
            print("✅ Standalone mode: Not specified (defaults to false)")
        
        # Check pin configuration
        pins = {}
        for line in content.split('\n'):
            line = line.strip()
            if 'clock_pin:' in line:
                pins['clock'] = line.split(':')[1].strip().replace('"', '')
            elif 'read_pin:' in line:
                pins['read'] = line.split(':')[1].strip().replace('"', '')
            elif 'write_pin:' in line:
                pins['write'] = line.split(':')[1].strip().replace('"', '')
            elif 'pc16_pin:' in line:
                pins['pc16'] = line.split(':')[1].strip().replace('"', '')
        
        print(f"✅ Pin Configuration:")
        print(f"   Clock (Yellow wire):     GPIO {pins.get('clock', 'NOT SET')}")
        print(f"   Read (Green wire):       GPIO {pins.get('read', 'NOT SET')}")
        print(f"   Write (Black wire):      GPIO {pins.get('write', 'NOT SET')}")
        print(f"   PC16 (Brown wire):       GPIO {pins.get('pc16', 'NOT SET')}")
        
        # Check debug level
        if 'debug: 3' in content:
            print("✅ Debug level: 3 (maximum debugging enabled)")
        else:
            print("⚠️  Debug level: Not set to maximum")
            print("   Recommendation: Set 'debug: 3' for detailed logs")
        
        return True
        
    except Exception as e:
        print(f"❌ Error analyzing configuration: {e}")
        return False

def print_hardware_checklist():
    """Print hardware troubleshooting checklist"""
    
    print("\n🔧 HARDWARE CONNECTION CHECKLIST")
    print("=" * 60)
    print("Since this setup worked with taligentx/dscKeybusInterface, check:")
    print()
    print("1. 🔌 POWER SUPPLY:")
    print("   □ DSC panel has stable 13.8V DC power")
    print("   □ ESP32 has stable 5V/3.3V power (separate from panel)")
    print("   □ No voltage drops during ESP32 boot")
    print()
    print("2. 🔗 WIRE CONNECTIONS:")
    print("   □ Clock (Yellow):    Panel AUX(-) → ESP32 GPIO 18")
    print("   □ Data (Green):      Panel DATA → ESP32 GPIO 19") 
    print("   □ Data Out (Black):  Panel DATA → ESP32 GPIO 21")
    print("   □ PC16 (Brown):      Panel PC16 → 1kΩ resistor → +5V → ESP32 GPIO 17")
    print("   □ Ground (Black):    Panel GND → ESP32 GND")
    print("   □ All connections are secure (no loose wires)")
    print()
    print("3. ⚡ CLASSIC SERIES SPECIFIC:")
    print("   □ PC16 pin MUST have 1kΩ pull-up resistor to +5V")
    print("   □ Panel is actively communicating (LED activity)")
    print("   □ No other devices interfering with keybus")
    print()
    print("4. 🔄 TIMING ISSUES:")
    print("   □ ESP32 boots completely before DSC communication starts")
    print("   □ No WiFi interference during hardware initialization")
    print("   □ Panel has been running for at least 30 seconds before ESP32 boot")
    print()

def print_esp_idf_compatibility():
    """Print ESP-IDF compatibility information"""
    
    print("\n⚙️  ESP-IDF COMPATIBILITY ANALYSIS")
    print("=" * 60)
    print("Since this worked with Arduino-based taligentx code:")
    print()
    print("🔍 Potential Issues:")
    print("   • ESP-IDF 5.3+ has stricter interrupt handling")
    print("   • Timer management changes in newer ESP-IDF versions")
    print("   • Different memory allocation patterns")
    print("   • Modified task scheduling behavior")
    print()
    print("🔧 Recommended Actions:")
    print("   1. Monitor serial output for 'esp32_hardware_initialized=false'")
    print("   2. Look for timer initialization failures")
    print("   3. Check for interrupt attachment errors")
    print("   4. Verify GPIO pin capabilities (all pins support interrupts)")
    print()

def print_debugging_guide():
    """Print detailed debugging guide"""
    
    print("\n📊 DEBUGGING GUIDE")
    print("=" * 60)
    print("Flash the configuration and monitor serial output at 115200 baud.")
    print("Look for these specific messages:")
    print()
    print("✅ SUCCESS indicators:")
    print("   • 'DSC Classic hardware initialization successful'")
    print("   • 'esp32_hardware_initialized=true'")
    print("   • 'attachInterrupt' completion without errors")
    print()
    print("❌ FAILURE indicators:")
    print("   • 'esp32_hardware_initialized=false'")
    print("   • 'DSC hardware initialization permanently failed'")
    print("   • Timer initialization failures")
    print("   • Interrupt attachment failures")
    print()
    print("🔍 Next Steps if Still Failing:")
    print("   1. Try different GPIO pins for clock/data")
    print("   2. Add oscilloscope to verify panel communication")
    print("   3. Test with minimal configuration (no WiFi/MQTT)")
    print("   4. Consider ESP-IDF version downgrade")
    print("   5. Compare with working taligentx Arduino version")
    print()

def main():
    print("DSC Classic Series Hardware Connection Diagnostic")
    print("=" * 60)
    print("Analyzing configuration for physical panel connectivity issues...")
    print()
    
    success = analyze_classic_configuration()
    
    print_hardware_checklist()
    print_esp_idf_compatibility() 
    print_debugging_guide()
    
    if success:
        print("\n🎯 SUMMARY")
        print("=" * 60)
        print("Configuration appears correct for DSC Classic panel.")
        print("Focus on:")
        print("• Hardware connections (especially PC16 with 1kΩ resistor)")
        print("• Power supply stability")
        print("• Panel communication timing")
        print("• ESP-IDF interrupt handling compatibility")
        print()
        print("Since this worked with taligentx code, the issue is likely:")
        print("• ESP-IDF timing/interrupt changes")
        print("• Loose hardware connection")
        print("• Power supply instability")
    else:
        print("\n❌ Configuration issues found - fix before hardware testing")
        
    return 0

if __name__ == "__main__":
    sys.exit(main())