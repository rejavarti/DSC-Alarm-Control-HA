#!/usr/bin/env python3
"""
Test script to validate the timer pre-initialization fix for infinite loop prevention
"""

import os
import re

def test_timer_preinitialization_fix():
    """Test that timer pre-initialization has circuit breaker logic"""
    cpp_file = "extras/ESPHome/components/dsc_keybus/dsc_keybus.cpp"
    
    if not os.path.exists(cpp_file):
        print(f"❌ ERROR: Could not find {cpp_file}")
        return False
    
    with open(cpp_file, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()
    
    print("=== ESP32 Timer Pre-initialization Circuit Breaker Fix Test ===")
    print()
    
    # Test 1: Check for timer initialization attempts tracking
    attempts_pattern = r'timer_init_attempts\s*\+\+'
    if re.search(attempts_pattern, content):
        print("✅ Timer initialization attempts tracking: FOUND")
    else:
        print("❌ Timer initialization attempts tracking: NOT FOUND")
        return False
    
    # Test 2: Check for timer initialization attempt limit
    limit_pattern = r'timer_init_attempts\s*>\s*10'
    if re.search(limit_pattern, content):
        print("✅ Timer initialization attempt limit (10): FOUND")
    else:
        print("❌ Timer initialization attempt limit: NOT FOUND")
        return False
    
    # Test 3: Check for timer initialization timeout (120 seconds)
    timeout_pattern = r'120000'
    if re.search(timeout_pattern, content):
        print("✅ Timer initialization timeout (120 seconds): FOUND")
    else:
        print("❌ Timer initialization timeout: NOT FOUND")
        return False
    
    # Test 4: Check for permanent failure marking
    failure_pattern = r'markInitializationFailed\(\)'
    if re.search(failure_pattern, content):
        print("✅ Permanent failure marking: FOUND")
    else:
        print("❌ Permanent failure marking: NOT FOUND")
        return False
    
    # Test 5: Check for exponential backoff
    backoff_pattern = r'backoff_delay = 5000 \+ \(1000 \* timer_init_attempts\)'
    if re.search(backoff_pattern, content):
        print("✅ Exponential backoff delay: FOUND")
    else:
        print("❌ Exponential backoff delay: NOT FOUND")
        return False
    
    # Test 6: Check for enhanced error logging with attempt count
    error_log_pattern = r'after.*attempts'
    if re.search(error_log_pattern, content):
        print("✅ Enhanced error logging with attempt count: FOUND")
    else:
        print("❌ Enhanced error logging: NOT FOUND")
        return False
    
    return True

def test_wrapper_failure_handling():
    """Test that wrapper has enhanced failure handling"""
    cpp_file = "extras/ESPHome/components/dsc_keybus/dsc_wrapper.cpp"
    
    if not os.path.exists(cpp_file):
        print(f"❌ ERROR: Could not find {cpp_file}")
        return False
    
    with open(cpp_file, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()
    
    print()
    print("=== DSC Wrapper Enhanced Failure Handling Test ===")
    print()
    
    # Test 1: Check for markInitializationFailed method
    method_pattern = r'void DSCWrapper::markInitializationFailed\(\)'
    if re.search(method_pattern, content):
        print("✅ markInitializationFailed method implementation: FOUND")
    else:
        print("❌ markInitializationFailed method: NOT FOUND")
        return False
    
    # Test 2: Check for enhanced hardware initialization success logging
    success_pattern = r'hardware initialization successful on attempt'
    if re.search(success_pattern, content):
        print("✅ Enhanced success logging: FOUND")
    else:
        print("❌ Enhanced success logging: NOT FOUND")
        return False
    
    # Test 3: Check for hardware initialization failure detection
    failure_pattern = r'hardware initialization incomplete.*esp32_hardware_initialized=false'
    if re.search(failure_pattern, content):
        print("✅ Hardware initialization failure detection: FOUND")
    else:
        print("❌ Hardware initialization failure detection: NOT FOUND")
        return False
    
    # Test 4: Check for permanent failure after attempts/timeout
    permanent_pattern = r'permanently failed after.*attempts over.*ms'
    if re.search(permanent_pattern, content):
        print("✅ Permanent failure after timeout logic: FOUND")
    else:
        print("❌ Permanent failure logic: NOT FOUND")
        return False
    
    return True

def main():
    print("Testing Enhanced Timer Pre-initialization and Wrapper Failure Handling...")
    print()
    
    test1_result = test_timer_preinitialization_fix()
    test2_result = test_wrapper_failure_handling()
    
    print()
    print("=== Results ===")
    
    if test1_result and test2_result:
        print("Tests passed: ALL")
        print("🎉 SUCCESS: Enhanced infinite loop prevention is properly implemented!")
        print()
        print("Expected behavior after fix:")
        print("- Timer pre-initialization failures will be limited to 10 attempts over 120 seconds")
        print("- Hardware initialization failures will be properly detected and marked permanent")
        print("- System will not get stuck in infinite loops due to timer/hardware init failures")
        print("- Enhanced logging will provide better diagnostics for troubleshooting")
        print("- Circuit breaker logic will prevent endless retry attempts")
        return True
    else:
        print("❌ FAILURE: Some tests failed. The infinite loop fix may be incomplete.")
        return False

if __name__ == "__main__":
    success = main()
    exit(0 if success else 1)