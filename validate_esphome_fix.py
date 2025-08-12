#!/usr/bin/env python3
"""
ESPHome LoadProhibited Fix Validation Script

This script validates that the LoadProhibited crash fix has been properly applied 
to the ESPHome component's static variable definitions.
"""

import os
import re
import sys

def validate_esphome_fix():
    """Validate that ESPHome static variables are properly initialized"""
    
    print("=== ESPHome LoadProhibited Fix Validation ===\n")
    
    # Check if ESPHome component exists
    component_path = "extras/ESPHome/components/dsc_keybus"
    if not os.path.exists(component_path):
        print("❌ ESPHome component directory not found!")
        return False
    
    # Check static variables file
    static_vars_file = os.path.join(component_path, "dsc_static_variables.cpp")
    if not os.path.exists(static_vars_file):
        print("❌ ESPHome static variables file not found!")
        return False
    
    print(f"✅ Found ESPHome component at {component_path}")
    
    # Read and validate static variables file
    with open(static_vars_file, 'r') as f:
        content = f.read()
    
    validation_results = []
    
    # Check 1: Verify initialization comments are present
    if "Initialize static variables to prevent uninitialized access crashes" in content:
        validation_results.append("✅ LoadProhibited fix comments found")
    else:
        validation_results.append("❌ LoadProhibited fix comments missing")
    
    # Check 2: Verify Classic interface variables are initialized
    classic_patterns = [
        r"byte dscClassicInterface::dscClockPin\s*=\s*255",
        r"volatile bool dscClassicInterface::bufferOverflow\s*=\s*false",
        r"volatile byte dscClassicInterface::panelBuffer\[dscBufferSize\]\[dscReadSize\]\s*=\s*\{0\}"
    ]
    
    classic_matches = 0
    for pattern in classic_patterns:
        if re.search(pattern, content):
            classic_matches += 1
    
    if classic_matches == len(classic_patterns):
        validation_results.append("✅ Classic interface static variables properly initialized")
    else:
        validation_results.append(f"❌ Classic interface initialization incomplete ({classic_matches}/{len(classic_patterns)})")
    
    # Check 3: Verify PowerSeries interface variables are initialized  
    powerseries_patterns = [
        r"byte dscKeybusInterface::dscClockPin\s*=\s*255",
        r"volatile bool dscKeybusInterface::bufferOverflow\s*=\s*false",
        r"volatile byte dscKeybusInterface::panelBuffer\[dscBufferSize\]\[dscReadSize\]\s*=\s*\{0\}"
    ]
    
    powerseries_matches = 0
    for pattern in powerseries_patterns:
        if re.search(pattern, content):
            powerseries_matches += 1
    
    if powerseries_matches == len(powerseries_patterns):
        validation_results.append("✅ PowerSeries interface static variables properly initialized")
    else:
        validation_results.append(f"❌ PowerSeries interface initialization incomplete ({powerseries_matches}/{len(powerseries_patterns)})")
    
    # Check 4: Verify keypad interfaces are initialized
    keypad_patterns = [
        r"byte dscKeypadInterface::dscClockPin\s*=\s*255",
        r"volatile bool dscKeypadInterface::bufferOverflow\s*=\s*false"
    ]
    
    keypad_matches = 0
    for pattern in keypad_patterns:
        if re.search(pattern, content):
            keypad_matches += 1
    
    if keypad_matches == len(keypad_patterns):
        validation_results.append("✅ Keypad interface static variables properly initialized")
    else:
        validation_results.append(f"❌ Keypad interface initialization incomplete ({keypad_matches}/{len(keypad_patterns)})")
    
    # Check 5: Verify no uninitialized static declarations remain
    uninitialized_patterns = [
        r"byte\s+\w+::\w+\s*;",  # byte ClassName::varName;
        r"volatile\s+byte\s+\w+::\w+\s*;",  # volatile byte ClassName::varName;
        r"bool\s+\w+::\w+\s*;"  # bool ClassName::varName;
    ]
    
    uninitialized_found = []
    for pattern in uninitialized_patterns:
        matches = re.findall(pattern, content)
        uninitialized_found.extend(matches)
    
    if not uninitialized_found:
        validation_results.append("✅ No uninitialized static variables found")
    else:
        validation_results.append(f"❌ Found {len(uninitialized_found)} uninitialized static variables")
        for var in uninitialized_found[:3]:  # Show first 3
            print(f"      - {var.strip()}")
    
    # Print results
    print("\n=== Validation Results ===")
    for result in validation_results:
        print(result)
    
    # Check if ESPHome documentation exists
    esphome_doc = "ESPHOME_LOADPROHIBITED_FIX.md"
    if os.path.exists(esphome_doc):
        print("✅ ESPHome LoadProhibited fix documentation found")
    else:
        print("❌ ESPHome LoadProhibited fix documentation missing")
    
    # Overall result
    failed_checks = sum(1 for result in validation_results if result.startswith("❌"))
    total_checks = len(validation_results)
    
    print(f"\n=== Overall Result ===")
    if failed_checks == 0:
        print(f"🎉 All {total_checks} validation checks passed!")
        print("✅ ESPHome LoadProhibited crash fix successfully applied")
        return True
    else:
        print(f"⚠️  {failed_checks}/{total_checks} validation checks failed")
        print("❌ ESPHome LoadProhibited crash fix incomplete")
        return False

if __name__ == "__main__":
    success = validate_esphome_fix()
    sys.exit(0 if success else 1)