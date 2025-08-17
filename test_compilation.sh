#!/bin/bash
# ESPHome DSC Alarm Compilation Test Script
# This script verifies that the dscalarm.yaml compiles successfully

echo "=== ESPHome DSC Alarm Compilation Test ==="
echo "Testing dscalarm.yaml compilation..."
echo

# Check if ESPHome is installed
if ! command -v esphome &> /dev/null; then
    echo "❌ ESPHome is not installed"
    exit 1
fi

# Validate configuration
echo "🔍 Validating configuration..."
if esphome config dscalarm.yaml > /dev/null 2>&1; then
    echo "✅ Configuration validation: PASSED"
else
    echo "❌ Configuration validation: FAILED"
    exit 1
fi

# Compile the project
echo "🔨 Compiling project..."
if timeout 300 esphome compile dscalarm.yaml > compile.log 2>&1; then
    echo "✅ Compilation: SUCCESS"
    
    # Check for warnings
    if grep -i "warning.*redefined" compile.log > /dev/null; then
        echo "⚠️  Macro redefinition warnings found"
    else
        echo "✅ No macro redefinition warnings"
    fi
    
    # Show memory usage
    echo "📊 Memory usage:"
    grep -E "RAM:|Flash:" compile.log | tail -2
    
else
    echo "❌ Compilation: FAILED"
    echo "Last 20 lines of compilation log:"
    tail -20 compile.log
    exit 1
fi

echo
echo "🎉 All tests passed! The dscalarm.yaml compiles successfully."
echo "The DSC Alarm ESPHome configuration is ready for use."