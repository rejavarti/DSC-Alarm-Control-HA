#!/usr/bin/env python3
"""
DSC Hardware Communication Diagnostic Tool
Analyzes the DSC panel communication issues and provides specific fixes

This script helps diagnose "no module communication to the panel" issues
by examining the hardware setup, timing, and communication protocol.
"""

import os
import sys
import re
import yaml

def analyze_dsc_config(config_file):
    """Analyze DSC configuration for hardware communication issues"""
    print(f"🔍 Analyzing DSC configuration: {config_file}")
    print("=" * 60)
    
    if not os.path.exists(config_file):
        print(f"❌ ERROR: Configuration file {config_file} not found!")
        return False
    
    try:
        with open(config_file, 'r') as f:
            content = f.read()
            
        # Parse YAML while preserving comments
        try:
            config = yaml.safe_load(content)
        except yaml.YAMLError as e:
            print(f"❌ YAML parsing error: {e}")
            return False
            
        # Check key configuration elements
        issues_found = []
        recommendations = []
        
        # 1. Check standalone mode
        dsc_config = config.get('dsc_keybus', {})
        standalone_mode = dsc_config.get('standalone_mode', False)
        
        print(f"📋 Configuration Analysis:")
        print(f"   Standalone Mode: {standalone_mode}")
        
        if not standalone_mode:
            print("   ⚠️  Hardware mode detected - requires physical DSC panel connection")
            
            # 2. Check series type
            series_type = dsc_config.get('series_type', 'PowerSeries')
            print(f"   Series Type: {series_type}")
            
            # 3. Check pin configuration
            pins = {
                'clock_pin': dsc_config.get('clock_pin'),
                'read_pin': dsc_config.get('read_pin'), 
                'write_pin': dsc_config.get('write_pin'),
                'pc16_pin': dsc_config.get('pc16_pin')
            }
            
            print(f"   Pin Configuration:")
            for pin_name, pin_value in pins.items():
                if pin_value:
                    print(f"     {pin_name}: GPIO {pin_value}")
                else:
                    print(f"     {pin_name}: Not configured")
            
            # 4. Check timing configuration
            timing_config = {
                'classic_timing_mode': dsc_config.get('classic_timing_mode', False),
                'hardware_detection_delay': dsc_config.get('hardware_detection_delay', 2000),
                'initialization_timeout': dsc_config.get('initialization_timeout', 30000),
                'retry_delay': dsc_config.get('retry_delay', 2000)
            }
            
            print(f"   Timing Configuration:")
            for timing_name, timing_value in timing_config.items():
                print(f"     {timing_name}: {timing_value}")
            
            # 5. Analyze issues
            if series_type == "Classic" and not pins.get('pc16_pin'):
                issues_found.append("PC16 pin not configured for Classic series")
                recommendations.append("Add pc16_pin configuration for Classic series panels")
            
            if not timing_config['classic_timing_mode'] and series_type == "Classic":
                issues_found.append("Classic timing mode disabled")
                recommendations.append("Enable classic_timing_mode: true for Classic series")
            
            if timing_config['hardware_detection_delay'] < 5000:
                issues_found.append("Hardware detection delay may be too short")
                recommendations.append("Increase hardware_detection_delay to 5000ms or higher")
                
        else:
            print("   ✅ Standalone mode - no hardware communication required")
            
        return analyze_hardware_communication_issues(issues_found, recommendations)
        
    except Exception as e:
        print(f"❌ Error analyzing configuration: {e}")
        return False

def analyze_hardware_communication_issues(issues_found, recommendations):
    """Analyze specific hardware communication issues"""
    
    print("\n🔧 Hardware Communication Analysis:")
    print("=" * 60)
    
    # Common DSC Classic communication issues
    classic_issues = [
        {
            "issue": "Interrupt attachment failure",
            "cause": "GPIO pins not properly configured for interrupts",
            "solution": "Verify GPIO 18 (clock) supports interrupts and is properly wired"
        },
        {
            "issue": "PC16 signal missing", 
            "cause": "Classic series requires PC16 pin with pullup resistor",
            "solution": "Connect PC16 (GPIO 17) through 1kΩ resistor to +5V rail"
        },
        {
            "issue": "Timing synchronization failure",
            "cause": "Classic series has different timing than PowerSeries",
            "solution": "Enable classic_timing_mode and increase delays"
        },
        {
            "issue": "Power supply instability",
            "cause": "Insufficient or noisy 5V power to interface board",
            "solution": "Use stable 5V supply, add decoupling capacitors"
        },
        {
            "issue": "Ground loop or electrical noise",
            "cause": "Poor grounding or electrical interference",
            "solution": "Ensure solid ground connection, use shielded cables"
        }
    ]
    
    print("Common Classic Series Communication Issues:")
    for i, issue in enumerate(classic_issues, 1):
        print(f"\n{i}. {issue['issue']}")
        print(f"   Cause: {issue['cause']}")
        print(f"   Solution: {issue['solution']}")
    
    # Configuration-specific issues
    if issues_found:
        print(f"\n⚠️  Configuration Issues Found ({len(issues_found)}):")
        for i, issue in enumerate(issues_found, 1):
            print(f"{i}. {issue}")
            
        print(f"\n💡 Recommended Fixes:")
        for i, rec in enumerate(recommendations, 1):
            print(f"{i}. {rec}")
    else:
        print("\n✅ No obvious configuration issues detected")
    
    return True

def create_hardware_diagnostic_config():
    """Create a diagnostic configuration for hardware testing"""
    
    diagnostic_config = """# DSC Classic Hardware Diagnostic Configuration
# This config enables maximum debugging to diagnose panel communication issues

substitutions:
  accessCode: !secret access_code
  # Classic Series pin assignments - VERIFY THESE MATCH YOUR WIRING
  clock_pin: "18"    # Yellow wire - DSC Clock signal
  read_pin: "19"     # Green wire - DSC Data signal  
  write_pin: "21"    # Black wire - DSC Data Out signal
  pc16_pin: "17"     # Brown/Purple wire - PC16 signal (CRITICAL for Classic series)

esphome:
  name: dsc-hardware-diagnostic
  comment: "DSC Classic hardware communication diagnostic"

esp32:
  board: esp32dev
  framework:
    type: esp-idf

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password

logger:
  level: DEBUG
  baud_rate: 115200

api:

ota:
  - platform: esphome
    password: !secret ota_password

external_components:
  - source:
      type: local
      path: extras/ESPHome/components
    components: [dsc_keybus]

# DSC Keybus Interface with maximum debugging for hardware diagnosis
dsc_keybus:
  id: dsc_interface
  access_code: ${accessCode}
  
  # CRITICAL: Classic series configuration
  series_type: "Classic"
  
  # Pin configuration
  clock_pin: ${clock_pin}
  read_pin: ${read_pin}
  write_pin: ${write_pin}
  pc16_pin: ${pc16_pin}
  
  # MAXIMUM debugging for hardware diagnosis
  debug: 3  # Enable all packet debugging
  
  # Enhanced timing for Classic series hardware issues
  classic_timing_mode: true              # CRITICAL for Classic series
  hardware_detection_delay: 10000        # 10 second delay for hardware detection
  initialization_timeout: 60000          # 60 second timeout
  retry_delay: 5000                      # 5 second delay between retries
  
  # Hardware mode - this will show communication errors
  standalone_mode: false
  
  # Event handlers to monitor communication
  on_system_status_change:
    then:
      - logger.log:
          format: "✅ DSC System Status: %s"
          args: ['status.c_str()']
          level: INFO

# Status indicators
binary_sensor:
  - platform: template
    id: panel_communication
    name: "DSC Panel Communication"
    icon: "mdi:ethernet"
    
# Diagnostic sensors
text_sensor:
  - platform: template
    id: diagnostic_status
    name: "DSC Diagnostic Status"
    icon: "mdi:medical-bag"

# Hardware diagnostic script
script:
  - id: hardware_diagnostic
    mode: restart
    then:
      - delay: 10s  # Wait for initialization
      - lambda: |-
          ESP_LOGI("diagnostic", "=== DSC Hardware Diagnostic Started ===");
          ESP_LOGI("diagnostic", "Clock Pin: GPIO %s", "${clock_pin}");
          ESP_LOGI("diagnostic", "Read Pin: GPIO %s", "${read_pin}");
          ESP_LOGI("diagnostic", "Write Pin: GPIO %s", "${write_pin}");
          ESP_LOGI("diagnostic", "PC16 Pin: GPIO %s", "${pc16_pin}");
          ESP_LOGI("diagnostic", "Series Type: Classic");
          ESP_LOGI("diagnostic", "Classic Timing Mode: Enabled");
          ESP_LOGI("diagnostic", "=== Monitor log for communication status ===");
          
interval:
  - interval: 30s
    then:
      - script.execute: hardware_diagnostic
"""
    
    with open("dsc_hardware_diagnostic.yaml", "w") as f:
        f.write(diagnostic_config)
    
    print("\n📝 Created hardware diagnostic configuration:")
    print("   File: dsc_hardware_diagnostic.yaml")
    print("\n🔍 To use this diagnostic:")
    print("1. esphome config dsc_hardware_diagnostic.yaml")
    print("2. esphome compile dsc_hardware_diagnostic.yaml") 
    print("3. Monitor serial output for detailed communication logs")
    print("4. Look for 'esp32_hardware_initialized=false' errors")

def create_fixed_config():
    """Create a fixed configuration with enhanced hardware communication"""
    
    print("\n🛠️  Creating enhanced DSC Classic configuration...")
    
    # Read the original config
    with open("dscalarm.yaml", "r") as f:
        content = f.read()
    
    # Apply specific fixes for Classic series hardware communication
    fixes = [
        {
            "search": "hardware_detection_delay: 5000",
            "replace": "hardware_detection_delay: 10000        # Increased for Classic series",
            "description": "Increase hardware detection delay"
        },
        {
            "search": "initialization_timeout: 30000",
            "replace": "initialization_timeout: 60000          # Increased timeout for Classic series",
            "description": "Increase initialization timeout"
        },
        {
            "search": "retry_delay: 2000",
            "replace": "retry_delay: 5000                      # Increased retry delay",
            "description": "Increase retry delay"
        },
        {
            "search": "debug: 3",
            "replace": "debug: 3  # Maximum debugging - monitor for 'esp32_hardware_initialized=false'",
            "description": "Add debugging note"
        }
    ]
    
    fixed_content = content
    applied_fixes = []
    
    for fix in fixes:
        if fix["search"] in fixed_content:
            fixed_content = fixed_content.replace(fix["search"], fix["replace"])
            applied_fixes.append(fix["description"])
    
    # Write the fixed configuration
    with open("dscalarm_hardware_communication_fix.yaml", "w") as f:
        f.write(fixed_content)
    
    print(f"✅ Created enhanced configuration: dscalarm_hardware_communication_fix.yaml")
    print(f"📋 Applied fixes:")
    for fix in applied_fixes:
        print(f"   • {fix}")
    
    return True

def main():
    """Main diagnostic function"""
    print("🔧 DSC Hardware Communication Diagnostic Tool")
    print("=" * 60)
    print("This tool helps diagnose 'no module communication to the panel' issues")
    print()
    
    # Check if we're in the right directory
    if not os.path.exists("dscalarm.yaml"):
        print("❌ ERROR: dscalarm.yaml not found in current directory")
        print("Please run this script from the DSC repository root directory")
        return False
    
    # Analyze the current configuration
    success = analyze_dsc_config("dscalarm.yaml")
    
    if success:
        # Create diagnostic tools
        create_hardware_diagnostic_config()
        create_fixed_config()
        
        print("\n🎯 Next Steps for Hardware Communication Fix:")
        print("=" * 60)
        print("1. PHYSICAL VERIFICATION:")
        print("   • Verify DSC panel is powered and operational")
        print("   • Check all wire connections are secure:")
        print("     - Clock (Yellow) -> GPIO 18")
        print("     - Data (Green) -> GPIO 19")
        print("     - Data Out (Black) -> GPIO 21") 
        print("     - PC16 (Brown/Purple) -> GPIO 17 through 1kΩ resistor to +5V")
        print("     - Ground (Black) -> ESP32 GND")
        print("   • Ensure 5V power supply is stable")
        
        print("\n2. TEST WITH DIAGNOSTIC CONFIG:")
        print("   esphome compile dsc_hardware_diagnostic.yaml")
        print("   (Monitor serial output for detailed logs)")
        
        print("\n3. USE ENHANCED CONFIG:")
        print("   esphome compile dscalarm_hardware_communication_fix.yaml")
        
        print("\n4. MONITOR FOR THESE KEY MESSAGES:")
        print("   ✅ 'DSC Classic hardware initialization successful'")
        print("   ❌ 'esp32_hardware_initialized=false'")
        print("   ❌ 'DSC hardware initialization permanently failed'")
        
        print("\n5. IF STILL FAILING:")
        print("   • Try different GPIO pins (avoid strapping pins)")
        print("   • Check for electrical interference") 
        print("   • Verify panel model is truly Classic series")
        print("   • Test with original taligentx Arduino library first")
        
        return True
    
    return False

if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)