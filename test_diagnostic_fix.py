#!/usr/bin/env python3
"""
Diagnostic Test for DSC ESPHome Configuration Fix
Tests that the user_dsc_config_fixed_debug.yaml configuration 
correctly enables standalone mode to resolve hardware initialization issues.
"""

import yaml
import sys
import os

def test_dsc_standalone_configuration():
    """Test that the DSC configuration has standalone mode enabled"""
    
    config_file = "user_dsc_config_fixed_debug.yaml"
    
    if not os.path.exists(config_file):
        print(f"❌ Configuration file {config_file} not found")
        return False
    
    try:
        # Read the file as text and check for standalone_mode setting
        with open(config_file, 'r') as f:
            content = f.read()
        
        # Check if dsc_keybus section exists
        if 'dsc_keybus:' not in content:
            print("❌ dsc_keybus configuration not found")
            return False
        
        # Look for standalone_mode setting
        lines = content.split('\n')
        standalone_found = False
        standalone_enabled = False
        
        for line in lines:
            line = line.strip()
            if line.startswith('standalone_mode:'):
                standalone_found = True
                if 'true' in line.lower():
                    standalone_enabled = True
                    print("✅ DSC standalone mode is correctly enabled")
                else:
                    print(f"❌ standalone_mode found but not enabled: {line}")
                break
        
        if not standalone_found:
            print("❌ standalone_mode not configured in dsc_keybus")
            return False
            
        if not standalone_enabled:
            print("❌ standalone_mode is not set to true")
            return False
        
        # Check for debug level
        debug_found = False
        for line in lines:
            line = line.strip()
            if line.startswith('debug:') and 'dsc_keybus' in content[content.find('dsc_keybus:'):content.find('dsc_keybus:') + content[content.find('dsc_keybus:'):].find('\n\n')]:
                debug_level = line.split(':')[1].strip()
                print(f"✅ Debug level: {debug_level} (diagnostic mode)")
                debug_found = True
                break
        
        # Check for series type
        for line in lines:
            line = line.strip()
            if line.startswith('series_type:'):
                series_type = line.split(':')[1].strip().replace('"', '')
                print(f"✅ Series type: {series_type}")
                break
        
        return True
        
    except Exception as e:
        print(f"❌ Error reading configuration: {e}")
        return False

def main():
    print("DSC ESPHome Configuration Diagnostic Test")
    print("=========================================")
    
    success = test_dsc_standalone_configuration()
    
    if success:
        print("\n🎉 Configuration test PASSED!")
        print("\nExpected behavior when flashing this configuration:")
        print("- ✅ DSC hardware initialization will simulate success")
        print("- ✅ No task watchdog timeouts from hardware initialization failures")
        print("- ✅ WiFi connection should proceed without blocking")
        print("- ✅ ESP32 should boot successfully without a physical DSC panel")
        print("- ✅ All sensors and entities will be available for testing")
        print("\nIn the logs, you should see messages like:")
        print("  [INFO][dsc_keybus]: Standalone mode enabled - simulating successful hardware initialization")
        print("  [INFO][dsc_keybus]: DSC Keybus hardware initialization complete (standalone mode)")
    else:
        print("\n❌ Configuration test FAILED!")
        return 1
        
    return 0

if __name__ == "__main__":
    sys.exit(main())