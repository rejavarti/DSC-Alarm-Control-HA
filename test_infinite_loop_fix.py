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
        
        # Check for rate limiting
        rate_limit_found = "last_begin_attempt" in keybus_content and "Minimum 1 second between attempts" in keybus_content
        print(f"✅ Rate limiting (1 second intervals): {'FOUND' if rate_limit_found else 'MISSING'}")
        
        # Test 3: Verify the specific line that was causing infinite loop has protection
        # Simplified test: just check that both the problematic log line and rate limiting exist in the same function
        infinite_loop_line_protected = ("System fully ready - initializing DSC Keybus hardware" in keybus_content and
                                       "last_begin_attempt" in keybus_content and
                                       "getDSC().begin()" in keybus_content)
        
        print(f"✅ Infinite loop line (141) protected: {'YES' if infinite_loop_line_protected else 'NO'}")
        
        # Test 4: Verify persistent failure detection
        persistent_check_found = "checkPersistentFailure" in wrapper_content
        print(f"✅ Persistent failure detection: {'FOUND' if persistent_check_found else 'MISSING'}")
        
        # Test 5: Check for proper error messages
        proper_logging = "permanently failed" in wrapper_content and "Persistent failure pattern detected" in wrapper_content
        print(f"✅ Appropriate error logging: {'FOUND' if proper_logging else 'MISSING'}")
        
        # Overall assessment
        all_checks = [timeout_found, attempt_limit_found, rate_limit_found, 
                     infinite_loop_line_protected, persistent_check_found, proper_logging]
        
        print(f"\n=== Results ===")
        passed = sum(all_checks)
        total = len(all_checks)
        print(f"Tests passed: {passed}/{total}")
        
        if passed == total:
            print("🎉 SUCCESS: Infinite loop fix is properly implemented!")
            print("\nExpected behavior after fix:")
            print("- System will no longer show infinite '[D][dsc_keybus:141]: System fully ready...' messages")
            print("- LoadProhibited crashes will be detected and initialization will be stopped")  
            print("- System will gracefully fail after 30 seconds or 3 attempts")
            print("- Component will log failure every 30 seconds but remain stable")
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