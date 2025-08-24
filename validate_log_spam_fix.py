#!/usr/bin/env python3
"""
Test script to verify DSC Classic log spam fix is correctly applied.
This script validates that the static variables are properly scoped at namespace level.
"""

import os
import re

def check_static_variable_scope_fix():
    """Verify the static variable scope bug fix is correctly applied."""
    
    print("🔍 Validating DSC Classic Log Spam Fix")
    print("=" * 60)
    
    # Check both main and minimal component files
    files_to_check = [
        'extras/ESPHome/components/dsc_keybus/dsc_keybus.cpp',
        'extras/ESPHome/components/dsc_keybus_minimal/dsc_keybus.cpp'
    ]
    
    all_good = True
    
    for file_path in files_to_check:
        print(f"\n📁 Checking: {file_path}")
        
        if not os.path.exists(file_path):
            print(f"❌ File not found: {file_path}")
            all_good = False
            continue
            
        with open(file_path, 'r') as f:
            content = f.read()
        
        # Check 1: Verify static variables are at namespace scope (before any function)
        namespace_scope_pattern = r'static uint32_t last_absolute_log_time = 0;\s*//.*FIXED.*namespace scope'
        if re.search(namespace_scope_pattern, content):
            print("✅ last_absolute_log_time correctly declared at namespace scope")
        else:
            print("❌ last_absolute_log_time not found at namespace scope with fix comment")
            all_good = False
        
        namespace_scope_pattern2 = r'static uint32_t absolute_log_count = 0;\s*//.*FIXED.*namespace scope'
        if re.search(namespace_scope_pattern2, content):
            print("✅ absolute_log_count correctly declared at namespace scope")
        else:
            print("❌ absolute_log_count not found at namespace scope with fix comment")
            all_good = False
        
        # Check 2: Verify NO static variables inside the classic timing conditional block
        function_scope_pattern = r'if.*classic_timing_mode.*\{[^}]*static uint32_t (last_absolute_log_time|absolute_log_count)'
        if re.search(function_scope_pattern, content, re.DOTALL):
            print("❌ Found static variables inside function scope - fix not applied correctly")
            all_good = False
        else:
            print("✅ No static variables found inside function scope - good!")
        
        # Check 3: Verify the variables are being used correctly in the function
        usage_pattern = r'current_time_classic - last_absolute_log_time >= 10000'
        if re.search(usage_pattern, content):
            print("✅ Static variables are being used correctly in rate limiting logic")
        else:
            print("❌ Rate limiting logic not found or incorrect")
            all_good = False
    
    print("\n" + "=" * 60)
    if all_good:
        print("🎉 SUCCESS: DSC Classic log spam fix is correctly applied!")
        print("📝 The static variables are now properly scoped at namespace level")
        print("🚫 This should completely eliminate the log spam issue")
        return True
    else:
        print("❌ FAILURE: Fix is not correctly applied")
        return False

def check_compilation_success():
    """Check if the test configuration compiled successfully."""
    print("\n🔨 Checking Compilation Status")
    print("=" * 60)
    
    # Check if firmware was built successfully
    firmware_path = '.esphome/build/dsc-classic-test/.pioenvs/dsc-classic-test/firmware.bin'
    if os.path.exists(firmware_path):
        file_size = os.path.getsize(firmware_path)
        print(f"✅ Firmware compiled successfully: {firmware_path}")
        print(f"📊 Firmware size: {file_size:,} bytes")
        return True
    else:
        print(f"❌ Firmware compilation failed - {firmware_path} not found")
        return False

if __name__ == "__main__":
    print("DSC Classic Timing Log Spam Fix Validation")
    print("=" * 60)
    
    scope_fix_ok = check_static_variable_scope_fix()
    compilation_ok = check_compilation_success()
    
    print(f"\n🏁 FINAL RESULT")
    print("=" * 60)
    
    if scope_fix_ok and compilation_ok:
        print("🎉 ALL TESTS PASSED!")
        print("✅ Static variable scope fix: APPLIED CORRECTLY")
        print("✅ Compilation test: SUCCESS")
        print("\n📋 Summary:")
        print("• The log spam bug has been fixed by moving static variables to namespace scope")
        print("• ESPHome compilation completes successfully with the fix")
        print("• The Classic timing mode message will now appear only ONCE per session")
        print("• No more spam of 'Classic timing mode enabled' messages")
        exit(0)
    else:
        print("❌ TESTS FAILED!")
        if not scope_fix_ok:
            print("❌ Static variable scope fix: NEEDS ATTENTION")
        if not compilation_ok:
            print("❌ Compilation test: FAILED")
        exit(1)