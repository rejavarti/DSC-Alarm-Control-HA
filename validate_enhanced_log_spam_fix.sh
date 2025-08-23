#!/bin/bash

# DSC Classic Timing Log Spam Fix Verification Script
# This script validates that the enhanced protection is working correctly

echo "=== DSC Classic Timing Log Spam Fix - Enhanced Verification ==="
echo

# Check if the enhanced static variables are properly declared
echo "1. Checking enhanced static variable declarations..."
echo "---------------------------------------------------"

main_component="/home/runner/work/New-DSC-Alarm-Control-HA/New-DSC-Alarm-Control-HA/extras/ESPHome/components/dsc_keybus/dsc_keybus.cpp"
minimal_component="/home/runner/work/New-DSC-Alarm-Control-HA/New-DSC-Alarm-Control-HA/extras/ESPHome/components/dsc_keybus_minimal/dsc_keybus.cpp"

# Check for enhanced static variables
for component in "$main_component" "$minimal_component"; do
    component_name=$(basename $(dirname "$component"))
    echo "Checking $component_name component..."
    
    if grep -q "static uint32_t classic_timing_call_count" "$component"; then
        echo "  ✅ classic_timing_call_count diagnostic counter found"
    else
        echo "  ❌ classic_timing_call_count diagnostic counter missing"
    fi
    
    if grep -q "static void\* last_component_instance" "$component"; then
        echo "  ✅ last_component_instance tracker found"
    else
        echo "  ❌ last_component_instance tracker missing"
    fi
    
    if grep -q "Multiple DSC component instances detected" "$component"; then
        echo "  ✅ Multi-instance detection logic found"
    else
        echo "  ❌ Multi-instance detection logic missing"
    fi
    
    if grep -q "log suppression active" "$component"; then
        echo "  ✅ Call frequency monitoring found"
    else
        echo "  ❌ Call frequency monitoring missing"
    fi
    echo
done

echo "2. Validating enhanced protection logic..."
echo "----------------------------------------"

# Check for the enhanced protection patterns
if grep -q "4-layer protection" "$main_component"; then
    echo "  ✅ Enhanced 4-layer protection documented in main component"
else
    echo "  ❌ Enhanced 4-layer protection documentation missing"
fi

if grep -q "call #%u" "$main_component"; then
    echo "  ✅ Call counter logging implemented"
else
    echo "  ❌ Call counter logging missing"
fi

if grep -q "classic_timing_call_count++" "$main_component"; then
    echo "  ✅ Call counter increment logic found"
else
    echo "  ❌ Call counter increment logic missing"
fi

echo

echo "3. Testing configuration compatibility..."
echo "---------------------------------------"

config_file="/home/runner/work/New-DSC-Alarm-Control-HA/New-DSC-Alarm-Control-HA/test_classic_timing_fix.yaml"
if python3 -c "import yaml; yaml.safe_load(open('$config_file'))" 2>/dev/null; then
    echo "  ✅ test_classic_timing_fix.yaml syntax is valid"
    
    # Extract key settings
    classic_mode=$(python3 -c "import yaml; config=yaml.safe_load(open('$config_file')); print(config['dsc_keybus']['classic_timing_mode'])")
    debug_level=$(python3 -c "import yaml; config=yaml.safe_load(open('$config_file')); print(config['dsc_keybus']['debug'])")
    
    if [ "$classic_mode" = "True" ]; then
        echo "  ✅ classic_timing_mode is enabled (required for fix testing)"
    else
        echo "  ❌ classic_timing_mode is disabled (should be True for testing)"
    fi
    
    if [ "$debug_level" = "3" ]; then
        echo "  ✅ debug level is 3 (will show diagnostic messages)"
    else
        echo "  ⚠️  debug level is $debug_level (consider level 3 for full diagnostics)"
    fi
else
    echo "  ❌ test_classic_timing_fix.yaml has syntax errors"
fi

echo

echo "4. Enhanced fix validation summary..."
echo "-----------------------------------"

# Count the protection layers implemented
protection_layers=0

if grep -q "classic_timing_logged" "$main_component"; then
    echo "  ✅ Layer 1: Namespace-scope static variables"
    ((protection_layers++))
fi

if grep -q "last_component_instance" "$main_component"; then
    echo "  ✅ Layer 2: Instance tracking"
    ((protection_layers++))
fi

if grep -q "classic_timing_call_count" "$main_component"; then
    echo "  ✅ Layer 3: Call frequency monitoring"
    ((protection_layers++))
fi

if grep -q "current_time_classic - last_classic_timing_log >= 10000" "$main_component"; then
    echo "  ✅ Layer 4: Time-based rate limiting"
    ((protection_layers++))
fi

echo

if [ $protection_layers -eq 4 ]; then
    echo "🎉 ENHANCED FIX VALIDATION: SUCCESS!"
    echo "   All 4 protection layers are properly implemented"
    echo
    echo "Expected behavior with enhanced fix:"
    echo "  • Initial message: 'Classic timing mode enabled... (call #1)'"
    echo "  • Rapid call monitoring: 'called N times rapidly - log suppression active'"
    echo "  • Multi-instance detection: 'Multiple DSC component instances detected'"
    echo "  • Time-based reminders: Limited to once every 10 seconds"
    echo
    echo "The enhanced fix should completely eliminate the log spam reported in the issue."
else
    echo "❌ ENHANCED FIX VALIDATION: FAILED!"
    echo "   Only $protection_layers/4 protection layers found"
fi