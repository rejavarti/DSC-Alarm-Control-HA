#!/usr/bin/env python3
"""
ESP32 LoadProhibited Crash Fix Validation Script

This script validates that the necessary fixes are in place to prevent
the 0xcececece LoadProhibited crash pattern in ESP32 builds.
"""

import os
import sys
import re
from pathlib import Path

def validate_file_exists(file_path, description):
    """Check if a file exists"""
    if os.path.exists(file_path):
        print(f"✅ {description}: {file_path}")
        return True
    else:
        print(f"❌ {description}: {file_path} - NOT FOUND")
        return False

def validate_pattern_in_file(file_path, pattern, description, should_exist=True):
    """Check if a pattern exists in a file"""
    if not os.path.exists(file_path):
        print(f"❌ {description}: File {file_path} does not exist")
        return False
        
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
            if should_exist:
                if re.search(pattern, content, re.MULTILINE | re.DOTALL):
                    print(f"✅ {description}: Found in {file_path}")
                    return True
                else:
                    print(f"❌ {description}: NOT FOUND in {file_path}")
                    return False
            else:
                if re.search(pattern, content, re.MULTILINE | re.DOTALL):
                    print(f"❌ {description}: STILL PRESENT in {file_path}")
                    return False
                else:
                    print(f"✅ {description}: Correctly removed from {file_path}")
                    return True
    except Exception as e:
        print(f"❌ Error reading {file_path}: {e}")
        return False

def main():
    """Main validation function"""
    print("🔍 ESP32 LoadProhibited Crash Fix Validation")
    print("=" * 50)
    
    validation_results = []
    
    # 1. Check critical files exist
    files_to_check = [
        ("src/HomeAssistant-MQTT.ino", "Main Arduino sketch"),
        ("src/dscClassic.cpp", "DSC Classic implementation"),
        ("src/dscKeybusInterface.cpp", "DSC Keybus interface"),
        ("src/dscKeypad.cpp", "DSC Keypad interface"),
        ("src/dscClassicKeypad.cpp", "DSC Classic Keypad interface")
    ]
    
    for file_path, description in files_to_check:
        validation_results.append(validate_file_exists(file_path, description))
    
    print("\n🔧 Static Variable Initialization Fixes:")
    print("-" * 40)
    
    # 2. Check for memory section attributes on timer variables
    timer_files = [
        "src/dscClassic.cpp",
        "src/dscKeybusInterface.cpp", 
        "src/dscKeypad.cpp",
        "src/dscClassicKeypad.cpp"
    ]
    
    for file_path in timer_files:
        validation_results.append(validate_pattern_in_file(
            file_path,
            r'__attribute__\(\(section\("\.data"\)\)\).*timer1',
            f"Timer variables memory section attribute in {file_path}"
        ))
    
    # 3. Check for constructor functions
    validation_results.append(validate_pattern_in_file(
        "src/dscClassic.cpp",
        r'__attribute__\(\(constructor\(\d+\)\)\).*dsc_classic_static_init',
        "Constructor function in dscClassic.cpp"
    ))
    
    # 4. Check that dsc_static_variables_initialized is properly set to false initially
    validation_results.append(validate_pattern_in_file(
        "src/dscClassic.cpp",
        r'dsc_static_variables_initialized\s*=\s*false',
        "Static variables initialized to false initially"
    ))
    
    # 5. Check for enhanced initialization in main sketch
    validation_results.append(validate_pattern_in_file(
        "src/HomeAssistant-MQTT.ino",
        r'dsc_static_variables_initialized',
        "Static variable check in main sketch"
    ))
    
    validation_results.append(validate_pattern_in_file(
        "src/HomeAssistant-MQTT.ino",
        r'delay\(2000\)',
        "Stabilization delay in main sketch"
    ))
    
    # 6. Check for LoadProhibited crash pattern checks
    validation_results.append(validate_pattern_in_file(
        "src/dscClassic.cpp",
        r'0xcececece.*0xa5a5a5a5',
        "LoadProhibited crash pattern detection"
    ))
    
    print("\n📊 Validation Summary:")
    print("-" * 20)
    
    passed = sum(validation_results)
    total = len(validation_results)
    
    if passed == total:
        print(f"✅ All {total} validations passed!")
        print("✅ ESP32 LoadProhibited crash fix is properly implemented.")
        return 0
    else:
        print(f"❌ {passed}/{total} validations passed.")
        print("❌ ESP32 LoadProhibited crash fix needs attention.")
        return 1

if __name__ == "__main__":
    sys.exit(main())