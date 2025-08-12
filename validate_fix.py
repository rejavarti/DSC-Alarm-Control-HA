#!/usr/bin/env python3
"""
DSC ESP32 Fix Validation Script

This script helps validate that the ESP32 LoadProhibited crash fix is working correctly
by checking the code structure and providing testing guidance.
"""

import os
import re
import sys

def check_file_exists(filepath, description):
    """Check if a file exists and print status"""
    if os.path.exists(filepath):
        print(f"✅ {description}: {filepath}")
        return True
    else:
        print(f"❌ {description}: {filepath} - NOT FOUND")
        return False

def check_static_initialization(filepath, class_name):
    """Check if static variables are properly initialized"""
    if not os.path.exists(filepath):
        print(f"❌ Cannot check {filepath} - file not found")
        return False
    
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()
    
    # Look for static variable initialization patterns
    patterns = [
        rf'{class_name}::dscClockPin\s*=',
        rf'{class_name}::dscReadPin\s*=', 
        rf'{class_name}::writeKey\s*=',
        rf'{class_name}::bufferOverflow\s*=',
        rf'{class_name}::panelData\[.*\]\s*=',
        rf'{class_name}::moduleData\[.*\]\s*='
    ]
    
    found_initializations = 0
    for pattern in patterns:
        if re.search(pattern, content):
            found_initializations += 1
    
    if found_initializations >= 4:  # At least 4 key initializations
        print(f"✅ Static variable initialization found in {filepath}")
        return True
    else:
        print(f"⚠️  Static variable initialization may be incomplete in {filepath}")
        return False

def check_constructor_safety(filepath, class_name):
    """Check if constructor has safety validation"""
    if not os.path.exists(filepath):
        return False
        
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()
    
    # Look for constructor safety patterns
    safety_patterns = [
        r'if.*setClockPin.*==.*255',
        r'if.*setReadPin.*==.*255',
        r'for.*byte.*i.*<.*dsc.*',
        r'panelBuffer.*\[.*\]\[.*\]\s*=\s*0'
    ]
    
    found_safety = 0
    for pattern in safety_patterns:
        if re.search(pattern, content):
            found_safety += 1
    
    if found_safety >= 2:
        print(f"✅ Constructor safety checks found in {filepath}")
        return True
    else:
        print(f"⚠️  Constructor safety checks may be incomplete in {filepath}")
        return False

def check_begin_method_safety(filepath):
    """Check if begin() method has proper safety checks"""
    if not os.path.exists(filepath):
        return False
        
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()
    
    # Look for begin method safety patterns
    safety_patterns = [
        r'if.*dscClockPin.*==.*255',
        r'CRITICAL.*attach.*interrupt.*AFTER',
        r'attachInterrupt.*dscClockPin.*dscClockInterrupt',
        r'if.*_stream.*println.*ERROR'
    ]
    
    found_safety = 0
    for pattern in safety_patterns:
        if re.search(pattern, content, re.IGNORECASE):
            found_safety += 1
    
    if found_safety >= 2:
        print(f"✅ begin() method safety checks found in {filepath}")
        return True
    else:
        print(f"⚠️  begin() method safety checks may be incomplete in {filepath}")
        return False

def check_arduino_sketch_safety(filepath):
    """Check Arduino sketch for safety improvements"""
    if not os.path.exists(filepath):
        return False
        
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()
    
    # Look for sketch safety patterns
    safety_patterns = [
        r'if.*dscReadSize.*==.*0',
        r'ESP\.getFreeHeap\(\)',
        r'if.*partition.*>=.*dscPartitions',
        r'bounds.*check.*partition.*index',
        r'Invalid.*partition.*index'
    ]
    
    found_safety = 0
    for pattern in safety_patterns:
        if re.search(pattern, content, re.IGNORECASE):
            found_safety += 1
    
    if found_safety >= 3:
        print(f"✅ Arduino sketch safety checks found in {filepath}")
        return True
    else:
        print(f"⚠️  Arduino sketch safety checks may be incomplete in {filepath}")
        return False

def main():
    """Main validation routine"""
    print("🔧 DSC ESP32 LoadProhibited Crash Fix Validation")
    print("=" * 50)
    
    base_dir = os.getcwd()
    src_dir = os.path.join(base_dir, 'src')
    
    # Check required files exist
    files_ok = True
    files_ok &= check_file_exists(os.path.join(src_dir, 'dscKeybusInterface.cpp'), "DSC Keybus Interface")
    files_ok &= check_file_exists(os.path.join(src_dir, 'dscClassic.cpp'), "DSC Classic Interface")  
    files_ok &= check_file_exists(os.path.join(src_dir, 'HomeAssistant-MQTT.ino'), "Arduino Sketch")
    files_ok &= check_file_exists(os.path.join(base_dir, 'ESP32_LOADPROHIBITED_CRASH_FIX.md'), "Fix Documentation")
    
    if not files_ok:
        print("\n❌ Not all required files found. Please ensure you're running from the project root directory.")
        return False
    
    print("\n🔍 Checking Static Variable Initialization...")
    print("-" * 40)
    
    # Check static variable initialization
    static_ok = True
    static_ok &= check_static_initialization(os.path.join(src_dir, 'dscKeybusInterface.cpp'), 'dscKeybusInterface')
    static_ok &= check_static_initialization(os.path.join(src_dir, 'dscClassic.cpp'), 'dscClassicInterface')
    
    print("\n🔧 Checking Constructor Safety...")
    print("-" * 40)
    
    # Check constructor safety
    constructor_ok = True
    constructor_ok &= check_constructor_safety(os.path.join(src_dir, 'dscKeybusInterface.cpp'), 'dscKeybusInterface')
    constructor_ok &= check_constructor_safety(os.path.join(src_dir, 'dscClassic.cpp'), 'dscClassicInterface')
    
    print("\n🚀 Checking begin() Method Safety...")
    print("-" * 40)
    
    # Check begin method safety
    begin_ok = True
    begin_ok &= check_begin_method_safety(os.path.join(src_dir, 'dscKeybusInterface.cpp'))
    begin_ok &= check_begin_method_safety(os.path.join(src_dir, 'dscClassic.cpp'))
    
    print("\n📱 Checking Arduino Sketch Safety...")
    print("-" * 40)
    
    # Check Arduino sketch safety
    sketch_ok = check_arduino_sketch_safety(os.path.join(src_dir, 'HomeAssistant-MQTT.ino'))
    
    print("\n" + "=" * 50)
    print("📋 VALIDATION SUMMARY")
    print("=" * 50)
    
    overall_status = files_ok and static_ok and constructor_ok and begin_ok and sketch_ok
    
    if overall_status:
        print("✅ ALL CHECKS PASSED - Fix appears to be properly implemented")
        print("\n📋 NEXT STEPS:")
        print("1. Compile the project using PlatformIO or Arduino IDE")
        print("2. Flash to ESP32 and monitor serial output for successful initialization")
        print("3. Test normal operation for 24+ hours")
        print("4. Monitor heap usage with ESP.getFreeHeap()")
        print("5. Test rapid arming/disarming cycles")
    else:
        print("⚠️  SOME CHECKS FAILED - Please review the implementation")
        print("\n📋 RECOMMENDED ACTIONS:")
        print("1. Review the failed checks above")
        print("2. Compare with ESP32_LOADPROHIBITED_CRASH_FIX.md documentation")
        print("3. Ensure all static variables are properly initialized")
        print("4. Verify safety checks are in place")
    
    print(f"\n📊 VALIDATION SCORE: {sum([files_ok, static_ok, constructor_ok, begin_ok, sketch_ok])}/5")
    
    return overall_status

if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)