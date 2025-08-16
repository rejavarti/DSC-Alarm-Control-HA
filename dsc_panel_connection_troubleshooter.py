#!/usr/bin/env python3
"""
DSC Panel Connection Troubleshooting Tool
Specifically designed to help diagnose timing and connection issues with DSC Classic panels
"""

import os
import sys
import subprocess
import time
from datetime import datetime

def print_header(title):
    """Print a formatted header"""
    print("\n" + "="*60)
    print(f" {title}")
    print("="*60)

def print_step(step_num, description):
    """Print a formatted step"""
    print(f"\n🔧 Step {step_num}: {description}")
    print("-" * 50)

def run_command(cmd, description, capture_output=True):
    """Run a command and return the result"""
    print(f"Running: {description}")
    print(f"Command: {cmd}")
    
    try:
        if capture_output:
            result = subprocess.run(cmd, shell=True, capture_output=True, text=True, timeout=120)
            if result.returncode == 0:
                print(f"✅ Success: {description}")
                return result.stdout
            else:
                print(f"❌ Failed: {description}")
                print(f"Error: {result.stderr}")
                return None
        else:
            # For interactive commands
            result = subprocess.run(cmd, shell=True, timeout=120)
            if result.returncode == 0:
                print(f"✅ Success: {description}")
                return True
            else:
                print(f"❌ Failed: {description}")
                return False
    except subprocess.TimeoutExpired:
        print(f"⏰ Timeout: {description} took too long")
        return None
    except Exception as e:
        print(f"❌ Error running {description}: {e}")
        return None

def check_prerequisites():
    """Check if all prerequisites are met"""
    print_step(1, "Checking Prerequisites")
    
    # Check if ESPHome is installed
    esphome_check = run_command("esphome version", "Checking ESPHome installation")
    if esphome_check is None:
        print("❌ ESPHome is not installed or not in PATH")
        print("Please run: pip install esphome")
        return False
    else:
        print(f"✅ ESPHome is installed: {esphome_check.strip()}")
    
    # Check if configuration files exist
    configs = [
        "user_dsc_config_fixed_debug.yaml", 
        "user_dsc_config_enhanced_timing.yaml",
        "secrets.yaml"
    ]
    
    for config in configs:
        if os.path.exists(config):
            print(f"✅ Found: {config}")
        else:
            print(f"❌ Missing: {config}")
            if config == "secrets.yaml":
                print("   Copy secrets.yaml.example to secrets.yaml and configure it")
            return False
    
    return True

def validate_configurations():
    """Validate all configuration files"""
    print_step(2, "Validating Configuration Files")
    
    configs = [
        ("user_dsc_config_fixed_debug.yaml", "Original debug configuration"),
        ("user_dsc_config_enhanced_timing.yaml", "Enhanced timing configuration")
    ]
    
    validation_results = {}
    
    for config_file, description in configs:
        print(f"\n📋 Validating {description}...")
        result = run_command(f"esphome config {config_file}", f"Validating {config_file}")
        validation_results[config_file] = result is not None
        
        if result is not None:
            print(f"✅ {config_file} is valid")
        else:
            print(f"❌ {config_file} has configuration errors")
    
    return validation_results

def analyze_timing_differences():
    """Analyze the differences between configurations"""
    print_step(3, "Analyzing Timing Configuration Differences")
    
    print("📊 Key timing differences between configurations:")
    print("""
Original Debug Config → Enhanced Timing Config:
┌─────────────────────────────────────┬─────────────┬─────────────────┐
│ Parameter                           │ Original    │ Enhanced        │
├─────────────────────────────────────┼─────────────┼─────────────────┤
│ Hardware Detection Delay            │ 5000ms      │ 8000ms          │
│ Initialization Timeout              │ 30000ms     │ 45000ms         │
│ Retry Delay                         │ 2000ms      │ 3000ms          │
│ Task Watchdog Timeout               │ 300s        │ 600s            │
│ Main Task Stack Size                │ 32KB        │ 40KB            │
│ WiFi Task Priority                  │ 18          │ 15 (lower)      │
│ Classic Series Support              │ Optional    │ Enabled         │
│ ESP-IDF Timer Task Stack            │ 8KB         │ 12KB            │
└─────────────────────────────────────┴─────────────┴─────────────────┘

🎯 Key improvements in Enhanced Timing Config:
• Longer detection delays for slow DSC Classic initialization
• Extended timeouts to prevent premature failure
• Lower WiFi priority to reduce interference with DSC timing
• Larger stacks to handle complex initialization sequences
• Explicit Classic series support with timing adjustments
""")

def provide_troubleshooting_guidance():
    """Provide specific troubleshooting guidance"""
    print_step(4, "Troubleshooting Guidance for DSC Panel Connection")
    
    print("""
🔍 PROBLEM ANALYSIS from your logs:
The ESP32 is experiencing:
1. "DSC hardware initialization permanently failed after 5 attempts"
2. Task watchdog timeout (IDLE0 CPU 0) during WiFi connection
3. Need for better timing coordination

🛠️  RECOMMENDED SOLUTIONS (in order of priority):

1️⃣  TRY ENHANCED TIMING CONFIGURATION FIRST:
   esphome run user_dsc_config_enhanced_timing.yaml
   
   This configuration includes:
   • Extended hardware detection delays (8 seconds)
   • Longer initialization timeouts (45 seconds)
   • Better WiFi/DSC timing coordination
   • Classic series specific optimizations

2️⃣  VERIFY PHYSICAL CONNECTIONS:
   • Clock (Yellow) → GPIO 18
   • Data (Green) → GPIO 19
   • Data Out (Black) → GPIO 21
   • PC16 (Brown/Purple) → GPIO 17 through 1kΩ resistor to +5V
   • Ground → ESP32 GND
   • Verify DSC panel is powered and operational

3️⃣  IF STILL HAVING ISSUES, TRY INCREMENTAL TIMING INCREASES:
   Edit user_dsc_config_enhanced_timing.yaml and increase:
   • hardware_detection_delay: 8000 → 10000 (10 seconds)
   • initialization_timeout: 45000 → 60000 (60 seconds)
   • retry_delay: 3000 → 5000 (5 seconds)

4️⃣  MONITOR SERIAL OUTPUT FOR THESE SUCCESS INDICATORS:
   [INFO][dsc_keybus]: Setting up DSC Keybus Interface...
   [INFO][dsc_keybus]: DSC Classic hardware initialization successful
   [INFO][dsc_keybus]: DSC Keybus connected

5️⃣  IF CONNECTION STILL FAILS:
   • Try different GPIO pins (interference)
   • Check panel compatibility (ensure Classic series)
   • Verify 5V power supply stability
   • Check for loose connections or oxidation
""")

def create_monitoring_script():
    """Create a script to monitor the ESP32 during flashing and boot"""
    print_step(5, "Creating Monitoring Script")
    
    monitoring_script = '''#!/bin/bash
# DSC Panel Connection Monitoring Script
echo "🔍 DSC Panel Connection Monitor - $(date)"
echo "This script will flash the enhanced timing configuration and monitor the logs"
echo "Press Ctrl+C to stop monitoring"
echo ""

echo "📡 Flashing enhanced timing configuration..."
esphome run user_dsc_config_enhanced_timing.yaml --device /dev/ttyUSB0

echo ""
echo "🔍 Monitoring logs for connection status..."
echo "Watch for these key messages:"
echo "  ✅ SUCCESS: 'DSC Classic hardware initialization successful'"
echo "  ❌ FAILURE: 'DSC hardware initialization permanently failed'"
echo "  ⚠️  WARNING: 'Task watchdog got triggered'"
echo ""

# Monitor logs
esphome logs user_dsc_config_enhanced_timing.yaml --device /dev/ttyUSB0
'''
    
    with open("monitor_dsc_connection.sh", "w") as f:
        f.write(monitoring_script)
    
    os.chmod("monitor_dsc_connection.sh", 0o755)
    print("✅ Created monitoring script: monitor_dsc_connection.sh")
    print("   Run with: ./monitor_dsc_connection.sh")

def main():
    """Main troubleshooting routine"""
    print_header("DSC Panel Connection Troubleshooting Tool")
    print(f"🕒 Started at: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    print("""
This tool will help diagnose and resolve DSC panel connection issues.
Based on your logs, we'll focus on timing adjustments and configuration optimization.
""")
    
    # Step 1: Check prerequisites
    if not check_prerequisites():
        print("\n❌ Prerequisites not met. Please resolve the issues above and try again.")
        return 1
    
    # Step 2: Validate configurations
    validation_results = validate_configurations()
    if not all(validation_results.values()):
        print("\n❌ Configuration validation failed. Please fix the errors above.")
        return 1
    
    # Step 3: Analyze timing differences
    analyze_timing_differences()
    
    # Step 4: Provide troubleshooting guidance
    provide_troubleshooting_guidance()
    
    # Step 5: Create monitoring script
    create_monitoring_script()
    
    print_header("NEXT STEPS")
    print("""
🚀 RECOMMENDED NEXT ACTIONS:

1. Use the enhanced timing configuration:
   esphome run user_dsc_config_enhanced_timing.yaml

2. Monitor the connection attempt:
   ./monitor_dsc_connection.sh

3. Watch serial output for success indicators:
   • "DSC Classic hardware initialization successful"
   • "DSC Keybus connected"  
   • No watchdog timeout errors

4. If issues persist, edit timing values in user_dsc_config_enhanced_timing.yaml:
   • Increase hardware_detection_delay to 10000ms
   • Increase initialization_timeout to 60000ms

💡 The enhanced timing configuration addresses:
   ✅ Extended hardware detection delays
   ✅ Better watchdog timeout handling  
   ✅ Optimized WiFi/DSC coordination
   ✅ Classic series specific timing adjustments
   ✅ Larger stack sizes for complex initialization

📞 If you continue to experience issues, the problem may be:
   • Physical wiring (especially PC16 resistor)
   • Panel compatibility
   • Power supply stability
   • Electrical interference
""")
    
    return 0

if __name__ == "__main__":
    exit(main())