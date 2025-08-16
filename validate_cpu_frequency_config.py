#!/usr/bin/env python3
"""
Validate ESP32 CPU Frequency Configuration
Checks that all ESP32 configuration files have proper 240MHz settings
"""

import os
import sys
import yaml
import glob

def check_yaml_file(filepath):
    """Check a single YAML file for proper CPU frequency configuration."""
    try:
        with open(filepath, 'r') as f:
            content = f.read()
            
        # Check if it's an ESP32 ESP-IDF configuration
        if 'esp-idf' not in content or 'CONFIG_ESP32_DEFAULT_CPU_FREQ' not in content:
            return True, "Not an ESP32 ESP-IDF configuration file"
            
        # Check for required settings
        required_configs = [
            'CONFIG_ESP32_DEFAULT_CPU_FREQ_240: y',
            'CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ: "240"',
            'CONFIG_ESP32_DEFAULT_CPU_FREQ_80: n',
            'CONFIG_ESP32_DEFAULT_CPU_FREQ_160: n',
            'CONFIG_ESP32_XTAL_FREQ_40: y'
        ]
        
        missing_configs = []
        for config in required_configs:
            if config not in content:
                missing_configs.append(config)
                
        if missing_configs:
            return False, f"Missing configurations: {missing_configs}"
        else:
            return True, "All CPU frequency configurations present"
            
    except Exception as e:
        return False, f"Error reading file: {e}"

def main():
    """Main validation function."""
    script_dir = os.path.dirname(os.path.abspath(__file__))
    yaml_files = glob.glob(os.path.join(script_dir, "extras/ESPHome/*.yaml"))
    
    print("🔍 Validating ESP32 CPU Frequency Configurations")
    print("=" * 60)
    
    total_files = 0
    passed_files = 0
    failed_files = []
    
    for yaml_file in sorted(yaml_files):
        filename = os.path.basename(yaml_file)
        success, message = check_yaml_file(yaml_file)
        
        total_files += 1
        
        if success:
            print(f"✅ {filename}: {message}")
            passed_files += 1
        else:
            print(f"❌ {filename}: {message}")
            failed_files.append(filename)
    
    print("\n" + "=" * 60)
    print(f"📊 Results: {passed_files}/{total_files} files passed validation")
    
    if failed_files:
        print(f"\n❌ Failed files: {', '.join(failed_files)}")
        print("\n🔧 Required fixes:")
        print("- Add CONFIG_ESP32_DEFAULT_CPU_FREQ_240: y")
        print("- Add CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ: \"240\"")
        print("- Add CONFIG_ESP32_DEFAULT_CPU_FREQ_80: n")
        print("- Add CONFIG_ESP32_DEFAULT_CPU_FREQ_160: n")
        print("- Add CONFIG_ESP32_XTAL_FREQ_40: y")
        return 1
    else:
        print("\n🎉 All ESP32 configurations have correct 240MHz CPU frequency settings!")
        return 0

if __name__ == "__main__":
    sys.exit(main())