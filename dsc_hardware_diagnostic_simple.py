#!/usr/bin/env python3
"""
DSC Hardware Communication Diagnostic Tool
Analyzes the DSC panel communication issues and provides specific fixes
"""

import os
import re

def analyze_dsc_config_simple(config_file):
    """Analyze DSC configuration for hardware communication issues"""
    print(f"🔍 Analyzing DSC configuration: {config_file}")
    print("=" * 60)
    
    if not os.path.exists(config_file):
        print(f"❌ ERROR: Configuration file {config_file} not found!")
        return False
    
    try:
        with open(config_file, 'r') as f:
            content = f.read()
            
        # Extract key configuration values using regex
        standalone_mode = re.search(r'standalone_mode:\s*(\w+)', content)
        series_type = re.search(r'series_type:\s*["\']?(\w+)["\']?', content)
        classic_timing = re.search(r'classic_timing_mode:\s*(\w+)', content)
        clock_pin = re.search(r'clock_pin:\s*["\']?(\d+)["\']?', content)
        read_pin = re.search(r'read_pin:\s*["\']?(\d+)["\']?', content)
        write_pin = re.search(r'write_pin:\s*["\']?(\d+)["\']?', content)
        pc16_pin = re.search(r'pc16_pin:\s*["\']?(\d+)["\']?', content)
        hardware_delay = re.search(r'hardware_detection_delay:\s*(\d+)', content)
        init_timeout = re.search(r'initialization_timeout:\s*(\d+)', content)
        
        print(f"📋 Configuration Analysis:")
        print(f"   Standalone Mode: {standalone_mode.group(1) if standalone_mode else 'Not found'}")
        print(f"   Series Type: {series_type.group(1) if series_type else 'Not found'}")
        print(f"   Classic Timing Mode: {classic_timing.group(1) if classic_timing else 'Not found'}")
        
        print(f"\n📌 Pin Configuration:")
        print(f"   Clock Pin (GPIO): {clock_pin.group(1) if clock_pin else 'Not found'}")
        print(f"   Read Pin (GPIO): {read_pin.group(1) if read_pin else 'Not found'}")
        print(f"   Write Pin (GPIO): {write_pin.group(1) if write_pin else 'Not found'}")
        print(f"   PC16 Pin (GPIO): {pc16_pin.group(1) if pc16_pin else 'Not found'}")
        
        print(f"\n⏱️  Timing Configuration:")
        print(f"   Hardware Detection Delay: {hardware_delay.group(1) if hardware_delay else 'Not found'} ms")
        print(f"   Initialization Timeout: {init_timeout.group(1) if init_timeout else 'Not found'} ms")
        
        # Analyze issues
        issues_found = []
        recommendations = []
        
        if standalone_mode and standalone_mode.group(1).lower() == 'false':
            print(f"\n⚠️  HARDWARE MODE DETECTED")
            print(f"   This requires a physical DSC panel connection!")
            
            if series_type and series_type.group(1) == "Classic":
                if not pc16_pin:
                    issues_found.append("PC16 pin not configured for Classic series")
                    recommendations.append("Add pc16_pin configuration for Classic series panels")
                
                if not classic_timing or classic_timing.group(1).lower() != 'true':
                    issues_found.append("Classic timing mode not enabled")
                    recommendations.append("Enable classic_timing_mode: true for Classic series")
                
                if hardware_delay and int(hardware_delay.group(1)) < 5000:
                    issues_found.append("Hardware detection delay may be too short for Classic series")
                    recommendations.append("Increase hardware_detection_delay to 5000ms or higher")
        
        return analyze_hardware_communication_issues(issues_found, recommendations, content)
        
    except Exception as e:
        print(f"❌ Error analyzing configuration: {e}")
        return False

def analyze_hardware_communication_issues(issues_found, recommendations, original_content):
    """Analyze specific hardware communication issues"""
    
    print("\n🔧 Hardware Communication Analysis:")
    print("=" * 60)
    
    # Common DSC Classic communication issues
    classic_issues = [
        {
            "issue": "Interrupt attachment failure",
            "cause": "GPIO pins not properly configured for interrupts or wiring issue",
            "solution": "Verify GPIO 18 (clock) supports interrupts and Yellow wire is securely connected"
        },
        {
            "issue": "PC16 signal missing", 
            "cause": "Classic series requires PC16 pin with pullup resistor",
            "solution": "Connect PC16 (GPIO 17) through 1kΩ resistor to +5V rail (CRITICAL for Classic)"
        },
        {
            "issue": "Timing synchronization failure",
            "cause": "Classic series has different timing requirements than PowerSeries",
            "solution": "Enable classic_timing_mode and increase hardware_detection_delay to 10000ms"
        },
        {
            "issue": "Power supply instability",
            "cause": "Insufficient or noisy 5V power to ESP32 interface",
            "solution": "Use stable 5V supply with at least 500mA capacity, add decoupling capacitors"
        },
        {
            "issue": "Panel not responding",
            "cause": "DSC panel may be in installer mode or not properly powered",
            "solution": "Exit installer mode on panel, verify 13.8V DC supply to panel is stable"
        },
        {
            "issue": "Ground loop or electrical noise",
            "cause": "Poor grounding or electrical interference",
            "solution": "Ensure solid ground connection between ESP32 and panel, use shielded cables"
        }
    ]
    
    print("🚨 Common Classic Series Communication Issues:")
    for i, issue in enumerate(classic_issues, 1):
        print(f"\n{i}. {issue['issue']}")
        print(f"   Cause: {issue['cause']}")
        print(f"   Solution: {issue['solution']}")
    
    # Configuration-specific issues
    if issues_found:
        print(f"\n⚠️  Configuration Issues Found ({len(issues_found)}):")
        for i, issue in enumerate(issues_found, 1):
            print(f"{i}. {issue}")
            
        print(f"\n💡 Recommended Configuration Fixes:")
        for i, rec in enumerate(recommendations, 1):
            print(f"{i}. {rec}")
    else:
        print(f"\n✅ No obvious configuration issues detected")
    
    # Create enhanced configuration
    create_enhanced_config(original_content)
    create_diagnostic_config()
    
    return True

def create_enhanced_config(original_content):
    """Create enhanced configuration for better hardware communication"""
    
    print(f"\n🛠️  Creating enhanced hardware communication configuration...")
    
    # Apply specific fixes for Classic series hardware communication
    enhanced_content = original_content
    
    # Increase timing values for better Classic series support
    enhanced_content = re.sub(
        r'hardware_detection_delay:\s*\d+',
        'hardware_detection_delay: 10000         # Increased for Classic series reliability',
        enhanced_content
    )
    
    enhanced_content = re.sub(
        r'initialization_timeout:\s*\d+',
        'initialization_timeout: 60000          # Increased timeout for Classic series',
        enhanced_content
    )
    
    enhanced_content = re.sub(
        r'retry_delay:\s*\d+',
        'retry_delay: 5000                      # Longer delay between retry attempts',
        enhanced_content
    )
    
    # Ensure debug level is maximum
    enhanced_content = re.sub(
        r'debug:\s*\d+',
        'debug: 3  # Maximum debugging - monitor for esp32_hardware_initialized status',
        enhanced_content
    )
    
    # Add comment about monitoring
    if 'standalone_mode: false' in enhanced_content:
        enhanced_content = enhanced_content.replace(
            'standalone_mode: false',
            'standalone_mode: false  # Hardware mode - monitor logs for communication status'
        )
    
    # Write the enhanced configuration
    with open("dscalarm_enhanced_hardware_fix.yaml", "w") as f:
        f.write(enhanced_content)
    
    print(f"✅ Created enhanced configuration: dscalarm_enhanced_hardware_fix.yaml")

def create_diagnostic_config():
    """Create a minimal diagnostic configuration"""
    
    diagnostic_config = '''# DSC Classic Hardware Communication Diagnostic
# Minimal config for testing panel communication

substitutions:
  accessCode: !secret access_code
  clock_pin: "18"    # Yellow wire - DSC Clock
  read_pin: "19"     # Green wire - DSC Data  
  write_pin: "21"    # Black wire - DSC Data Out
  pc16_pin: "17"     # Brown/Purple - PC16 (CRITICAL for Classic)

esphome:
  name: dsc-diagnostic
  comment: "DSC Classic communication test"

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
  logs:
    dsc_keybus: DEBUG

api:

external_components:
  - source:
      type: local
      path: extras/ESPHome/components
    components: [dsc_keybus]

# Diagnostic DSC configuration
dsc_keybus:
  id: dsc_interface
  access_code: ${accessCode}
  series_type: "Classic"
  
  clock_pin: ${clock_pin}
  read_pin: ${read_pin}
  write_pin: ${write_pin}
  pc16_pin: ${pc16_pin}
  
  debug: 3  # Maximum debugging
  
  # Enhanced timing for hardware diagnosis
  classic_timing_mode: true
  hardware_detection_delay: 15000   # 15 second delay
  initialization_timeout: 90000     # 90 second timeout
  retry_delay: 10000                # 10 second retry delay
  
  standalone_mode: false  # Test actual hardware

# Monitor panel connection status
text_sensor:
  - platform: template
    id: connection_status
    name: "Panel Connection Status"

script:
  - id: connection_monitor
    mode: restart
    then:
      - delay: 5s
      - lambda: |-
          ESP_LOGI("diagnostic", "=== DSC Hardware Diagnostic Monitor ===");
          ESP_LOGI("diagnostic", "Monitor for these messages:");
          ESP_LOGI("diagnostic", "✅ SUCCESS: 'DSC Classic hardware initialization successful'");
          ESP_LOGI("diagnostic", "❌ FAILURE: 'esp32_hardware_initialized=false'");
          ESP_LOGI("diagnostic", "❌ TIMEOUT: 'permanently failed after X attempts'");

interval:
  - interval: 60s
    then:
      - script.execute: connection_monitor
'''
    
    with open("dsc_minimal_diagnostic.yaml", "w") as f:
        f.write(diagnostic_config)
    
    print(f"✅ Created minimal diagnostic config: dsc_minimal_diagnostic.yaml")

def main():
    """Main diagnostic function"""
    print("🔧 DSC Hardware Communication Diagnostic Tool")
    print("=" * 60)
    print("Diagnoses 'no module communication to the panel' issues")
    print()
    
    # Check if we're in the right directory
    if not os.path.exists("dscalarm.yaml"):
        print("❌ ERROR: dscalarm.yaml not found in current directory")
        print("Please run this script from the DSC repository root directory")
        return False
    
    # Analyze the current configuration
    success = analyze_dsc_config_simple("dscalarm.yaml")
    
    if success:
        print("\n🎯 STEP-BY-STEP HARDWARE COMMUNICATION FIX:")
        print("=" * 60)
        
        print("1️⃣  PHYSICAL WIRING VERIFICATION (MOST CRITICAL):")
        print("   ⚡ DSC Panel must be powered (13.8V DC) and operational")
        print("   📍 Verify exact wire connections:")
        print("      • Yellow wire (Clock) -> ESP32 GPIO 18")
        print("      • Green wire (Data) -> ESP32 GPIO 19") 
        print("      • Black wire (Data Out) -> ESP32 GPIO 21")
        print("      • Brown/Purple wire (PC16) -> ESP32 GPIO 17")
        print("   🔧 PC16 CRITICAL: Must connect through 1kΩ resistor to +5V")
        print("   ⚫ Black wire (Ground) -> ESP32 GND")
        print("   🔋 ESP32 powered with stable 5V supply (min 500mA)")
        
        print("\n2️⃣  TEST WITH ENHANCED CONFIGURATION:")
        print("   esphome compile dscalarm_enhanced_hardware_fix.yaml")
        print("   (Monitor serial output at 115200 baud)")
        
        print("\n3️⃣  USE MINIMAL DIAGNOSTIC FOR TROUBLESHOOTING:")
        print("   esphome compile dsc_minimal_diagnostic.yaml")
        print("   (Simplified config to isolate communication issues)")
        
        print("\n4️⃣  MONITOR SERIAL OUTPUT FOR THESE KEY MESSAGES:")
        print("   ✅ SUCCESS: 'DSC Classic hardware initialization successful'")
        print("   ❌ FAILURE: 'esp32_hardware_initialized=false'") 
        print("   ❌ TIMEOUT: 'DSC hardware initialization permanently failed'")
        print("   🔍 DEBUG: Look for interrupt attachment and timing messages")
        
        print("\n5️⃣  IF STILL FAILING - ADVANCED TROUBLESHOOTING:")
        print("   • Exit installer mode on DSC panel (if active)")
        print("   • Try different ESP32 GPIO pins (avoid GPIO 0, 2, 15)")
        print("   • Check for 13.8V at panel auxiliary power terminals")
        print("   • Verify panel model is truly Classic series (not PowerSeries)")
        print("   • Test with original taligentx Arduino library first")
        print("   • Use oscilloscope to verify clock signal on Yellow wire")
        print("   • Check for electrical noise/interference")
        
        print("\n6️⃣  REFERENCE - ORIGINAL TALIGENTX LIBRARY:")
        print("   📚 https://github.com/taligentx/dscKeybusInterface")
        print("   📖 Compare timing and initialization with original Arduino code")
        print("   🔄 Consider testing Arduino version first to verify hardware")
        
        return True
    
    return False

if __name__ == "__main__":
    success = main()
    print(f"\n{'='*60}")
    if success:
        print("🎉 Diagnostic complete! Use the enhanced configurations to test hardware communication.")
    else:
        print("❌ Diagnostic failed. Check configuration file and try again.")
    print(f"{'='*60}")