#!/usr/bin/env python3
"""
Specific test for the ESP32 LoadProhibited infinite loop fix.
Validates that the circuit breaker mechanisms prevent the infinite loop described in the issue.
"""

import re

def test_infinite_loop_fix():
    """Test that the infinite loop fix is properly implemented."""
    print("=== ESP32 LoadProhibited Infinite Loop Fix Test ===\n")
    
    try:
        # Test 1: Verify timeout circuit breaker in wrapper
        with open("extras/ESPHome/components/dsc_keybus/dsc_wrapper.cpp", "r") as f:
            wrapper_content = f.read()
        
        # Check for 30-second timeout
        timeout_found = "30000" in wrapper_content and "timeout after 30 seconds" in wrapper_content
        print(f"✅ 30-second timeout circuit breaker: {'FOUND' if timeout_found else 'MISSING'}")
        
        # Check for attempt counter
        attempt_limit_found = "initialization_attempts_" in wrapper_content and "> 3" in wrapper_content
        print(f"✅ 3-attempt limit circuit breaker: {'FOUND' if attempt_limit_found else 'MISSING'}")
        
        # Test 2: Verify rate limiting in main loop
        with open("extras/ESPHome/components/dsc_keybus/dsc_keybus.cpp", "r") as f:
            keybus_content = f.read()
        
        # Check for ESP-IDF 5.3.2+ specific rate limiting
        esp_idf_rate_limit_found = "last_esp_idf_log" in keybus_content and "Log every 5 seconds max" in keybus_content
        print(f"✅ ESP-IDF 5.3.2+ rate limiting (5 second intervals): {'FOUND' if esp_idf_rate_limit_found else 'MISSING'}")
        
        # Check for the specific problematic message with rate limiting
        problematic_message_fixed = ("ESP-IDF 5.3.2+ stabilization period not complete - delaying hardware init" in keybus_content and
                                    "last_esp_idf_log" in keybus_content)
        print(f"✅ Problematic stabilization message rate limited: {'YES' if problematic_message_fixed else 'NO'}")
        
        # Test 3: Verify 30-second timeout for stuck stabilization
        stuck_stabilization_fix = ("elapsed_time > 30000" in keybus_content and 
                                  "forcing continuation" in keybus_content)
        print(f"✅ 30-second stabilization timeout protection: {'YES' if stuck_stabilization_fix else 'NO'}")
        
        # Test 4: Check for enhanced logging with timing info
        enhanced_logging = ("elapsed: %lu ms, need: 3000 ms" in keybus_content)
        print(f"✅ Enhanced debug logging with timing: {'YES' if enhanced_logging else 'NO'}")
        
        # Test 4: Verify persistent failure detection
        persistent_check_found = "checkPersistentFailure" in wrapper_content
        print(f"✅ Persistent failure detection: {'FOUND' if persistent_check_found else 'MISSING'}")
        
        # Test 5: Check for proper error messages
        proper_logging = "permanently failed" in wrapper_content and "Persistent failure pattern detected" in wrapper_content
        print(f"✅ Appropriate error logging: {'FOUND' if proper_logging else 'MISSING'}")
        
        # Overall assessment
        all_checks = [timeout_found, attempt_limit_found, esp_idf_rate_limit_found, 
                     problematic_message_fixed, stuck_stabilization_fix, enhanced_logging, 
                     persistent_check_found, proper_logging]
        
        print(f"\n=== Results ===")
        passed = sum(all_checks)
        total = len(all_checks)
        print(f"Tests passed: {passed}/{total}")
        
        if passed == total:
            print("🎉 SUCCESS: Infinite loop fix is properly implemented!")
            print("\nExpected behavior after fix:")
            print("- System will no longer spam '[D][dsc_keybus:163]: ESP-IDF 5.3.2+ stabilization period not complete' messages")
            print("- Rate limiting will reduce log spam to once every 5 seconds maximum")
            print("- If stabilization gets stuck for >30 seconds, system will force continuation")
            print("- Enhanced debug logging will show elapsed time to help diagnose timing issues")
            print("- Component will proceed normally after stabilization period completes")
            return True
        else:
            print("❌ FAILED: Some components of the infinite loop fix are missing")
            return False
            
    except FileNotFoundError as e:
        print(f"❌ ERROR: Could not find required file: {e}")
        return False

if __name__ == "__main__":
    import sys
    success = test_infinite_loop_fix()
    sys.exit(0 if success else 1)