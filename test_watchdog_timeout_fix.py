#!/usr/bin/env python3
"""
Test script to validate the watchdog timeout fix
"""

import os
import re

def test_watchdog_timeout_fix():
    """Test that watchdog timeout prevention measures are in place"""
    keybus_file = "extras/ESPHome/components/dsc_keybus/dsc_keybus.cpp"
    wrapper_file = "extras/ESPHome/components/dsc_keybus/dsc_wrapper.cpp"
    
    print("=== Watchdog Timeout Prevention Fix Test ===")
    print()
    
    success = True
    
    # Test the main keybus component
    if not os.path.exists(keybus_file):
        print(f"❌ ERROR: Could not find {keybus_file}")
        return False
    
    with open(keybus_file, 'r', encoding='utf-8', errors='ignore') as f:
        keybus_content = f.read()
    
    # Test 1: Check for yield() calls
    yield_pattern = r'yield\(\)'
    yield_matches = re.findall(yield_pattern, keybus_content)
    if len(yield_matches) >= 8:  # Should have multiple yield calls
        print(f"✅ yield() calls in keybus component: FOUND ({len(yield_matches)} instances)")
    else:
        print(f"❌ Insufficient yield() calls in keybus component: {len(yield_matches)} found, expected at least 8")
        success = False
    
    # Test 2: Check for watchdog resets
    watchdog_pattern = r'esp_task_wdt_reset\(\)'
    watchdog_matches = re.findall(watchdog_pattern, keybus_content)
    if len(watchdog_matches) >= 10:  # Should have multiple watchdog resets
        print(f"✅ Watchdog resets in keybus component: FOUND ({len(watchdog_matches)} instances)")
    else:
        print(f"❌ Insufficient watchdog resets in keybus component: {len(watchdog_matches)} found, expected at least 10")
        success = False
    
    # Test 3: Check for reduced maximum loop attempts
    max_attempts_pattern = r'total_loop_attempts\s*>\s*500'
    if re.search(max_attempts_pattern, keybus_content):
        print("✅ Reduced maximum loop attempts (500): FOUND")
    else:
        print("❌ Reduced maximum loop attempts: NOT FOUND")
        success = False
    
    # Test 4: Check for increased rate limiting delay
    rate_limit_pattern = r'now\s*-\s*last_begin_attempt\s*<\s*2000'
    if re.search(rate_limit_pattern, keybus_content):
        print("✅ Increased rate limiting delay (2s): FOUND")
    else:
        print("❌ Increased rate limiting delay: NOT FOUND")
        success = False
    
    # Test 5: Check for increased stabilization delays
    stabilization_pattern = r'required_delay\s*=\s*2000'
    if re.search(stabilization_pattern, keybus_content):
        print("✅ Increased stabilization delay (2s): FOUND")
    else:
        print("❌ Increased stabilization delay: NOT FOUND")
        success = False
    
    # Test the wrapper component
    if not os.path.exists(wrapper_file):
        print(f"❌ ERROR: Could not find {wrapper_file}")
        return False
    
    with open(wrapper_file, 'r', encoding='utf-8', errors='ignore') as f:
        wrapper_content = f.read()
    
    # Test 6: Check for yield() calls in wrapper
    wrapper_yield_matches = re.findall(yield_pattern, wrapper_content)
    if len(wrapper_yield_matches) >= 4:  # Should have multiple yield calls
        print(f"✅ yield() calls in wrapper: FOUND ({len(wrapper_yield_matches)} instances)")
    else:
        print(f"❌ Insufficient yield() calls in wrapper: {len(wrapper_yield_matches)} found, expected at least 4")
        success = False
    
    # Test 7: Check for watchdog resets in wrapper
    wrapper_watchdog_matches = re.findall(watchdog_pattern, wrapper_content)
    if len(wrapper_watchdog_matches) >= 4:  # Should have multiple watchdog resets
        print(f"✅ Watchdog resets in wrapper: FOUND ({len(wrapper_watchdog_matches)} instances)")
    else:
        print(f"❌ Insufficient watchdog resets in wrapper: {len(wrapper_watchdog_matches)} found, expected at least 4")
        success = False
    
    # Test 8: Check for increased timeout in wrapper (60 seconds)
    wrapper_timeout_pattern = r'60000'
    if re.search(wrapper_timeout_pattern, wrapper_content):
        print("✅ Increased wrapper timeout (60s): FOUND")
    else:
        print("❌ Increased wrapper timeout: NOT FOUND")
        success = False
    
    print()
    print("=== Summary ===")
    if success:
        print("🎉 SUCCESS: All watchdog timeout prevention measures are in place!")
        print()
        print("Expected behavior after fix:")
        print("- IDLE task will get regular CPU time to reset task watchdog")
        print("- Hardware initialization attempts are properly rate limited")
        print("- System will not overwhelm the task scheduler")
        print("- Watchdog timeout crashes should be prevented")
    else:
        print("❌ FAILURE: Some watchdog timeout prevention measures are missing")
    
    return success

if __name__ == "__main__":
    success = test_watchdog_timeout_fix()
    exit(0 if success else 1)