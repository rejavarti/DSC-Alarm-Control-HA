#!/bin/bash

# DSC Classic Timing Log Spam Fix Validation Script
# This script checks if the fix has been properly applied

echo "=== DSC Classic Timing Log Spam Fix Validation ==="
echo ""

# Function to check if a static variable is declared at namespace scope
check_namespace_scope() {
    local file=$1
    local variable=$2
    local component_name=$3
    
    echo "Checking $component_name component for $variable..."
    
    # Look for the variable declaration in the first 50 lines (namespace scope)
    if head -50 "$file" | grep -E "^static bool $variable = false;" > /dev/null; then
        echo "  ✅ $variable is declared at namespace scope"
        return 0
    else
        echo "  ❌ $variable is NOT properly declared at namespace scope"
        return 1
    fi
}

# Function to check that function-scope declarations are removed
check_no_function_scope() {
    local file=$1
    local variable=$2
    local component_name=$3
    
    echo "Checking $component_name component for absence of function-scope $variable..."
    
    # Look for function-scope declarations (these should be removed)
    local function_scope_count=$(grep -n "static.*$variable" "$file" | grep -v "^[0-9]\+:static bool $variable = false;" | wc -l)
    
    if [ "$function_scope_count" -eq 0 ]; then
        echo "  ✅ No problematic function-scope declarations found"
        return 0
    else
        echo "  ❌ Found $function_scope_count problematic function-scope declarations"
        grep -n "static.*$variable" "$file" | grep -v "^[0-9]\+:static bool $variable = false;"
        return 1
    fi
}

# Check main dsc_keybus component
MAIN_FILE="extras/ESPHome/components/dsc_keybus/dsc_keybus.cpp"
MINIMAL_FILE="extras/ESPHome/components/dsc_keybus_minimal/dsc_keybus.cpp"

if [ ! -f "$MAIN_FILE" ]; then
    echo "❌ Main component file not found: $MAIN_FILE"
    exit 1
fi

if [ ! -f "$MINIMAL_FILE" ]; then
    echo "❌ Minimal component file not found: $MINIMAL_FILE"
    exit 1
fi

echo "Validating main dsc_keybus component..."
echo "-------------------------------------------"

all_good=true

# Check main component
check_namespace_scope "$MAIN_FILE" "classic_timing_logged" "main" || all_good=false
check_namespace_scope "$MAIN_FILE" "classic_retry_logged" "main" || all_good=false
check_namespace_scope "$MAIN_FILE" "classic_rate_limit_logged" "main" || all_good=false

echo ""
echo "Validating minimal dsc_keybus component..."
echo "-------------------------------------------"

# Check minimal component
check_namespace_scope "$MINIMAL_FILE" "classic_timing_logged" "minimal" || all_good=false
check_namespace_scope "$MINIMAL_FILE" "classic_rate_limit_logged" "minimal" || all_good=false

echo ""
echo "Checking for proper usage in code..."
echo "-----------------------------------"

# Check that the variables are being used correctly
if grep -n "!classic_timing_logged" "$MAIN_FILE" > /dev/null; then
    echo "  ✅ classic_timing_logged is being used correctly in main component"
else
    echo "  ❌ classic_timing_logged usage not found in main component"
    all_good=false
fi

if grep -n "!classic_retry_logged" "$MAIN_FILE" > /dev/null; then
    echo "  ✅ classic_retry_logged is being used correctly in main component"
else
    echo "  ❌ classic_retry_logged usage not found in main component"
    all_good=false
fi

if grep -n "!classic_timing_logged" "$MINIMAL_FILE" > /dev/null; then
    echo "  ✅ classic_timing_logged is being used correctly in minimal component"
else
    echo "  ❌ classic_timing_logged usage not found in minimal component"
    all_good=false
fi

echo ""
echo "=== VALIDATION RESULT ==="
if [ "$all_good" = true ]; then
    echo "✅ SUCCESS: DSC Classic timing log spam fix has been properly applied!"
    echo ""
    echo "The fix should prevent the following log spam:"
    echo "  [D][dsc_keybus:290]: Classic timing mode enabled - applying extended delays for DSC Classic panels"
    echo "  [D][dsc_keybus:489]: Classic timing mode: Using extended retry delay of XXXX ms"
    echo ""
    echo "These messages will now appear only ONCE per session instead of continuously."
    exit 0
else
    echo "❌ FAILED: There are issues with the fix implementation!"
    echo ""
    echo "Please review the output above and ensure all static variables are properly"
    echo "declared at namespace scope in both component files."
    exit 1
fi