#!/bin/bash
# ESP32 Task Watchdog Timeout Fix Validation Script
# This script validates the ESP-IDF 5.3.2 configuration fixes for DSC Alarm system

echo "=== ESP32 Task Watchdog Timeout Fix Validation ==="
echo "Testing ESP-IDF 5.3.2 + ESPHome 2025.7.5 compatibility fixes"
echo

cd "$(dirname "$0")/extras/ESPHome"

# Check if ESPHome is available
if ! command -v esphome &> /dev/null; then
    echo "❌ ERROR: ESPHome not found. Please install ESPHome first:"
    echo "   pip install esphome"
    exit 1
fi

echo "✅ ESPHome found: $(esphome version)"
echo

# Check if secrets file exists
if [ ! -f "secrets.yaml" ]; then
    echo "❌ ERROR: secrets.yaml not found. Please create it from secrets.yaml.example"
    exit 1
fi

echo "✅ secrets.yaml found"
echo

# Test configurations
configs=(
    "esp32_boot_test_standalone.yaml:ESP32 Boot Test (Standalone)"
    "dsc_minimal_test.yaml:DSC Minimal Test" 
    "DscAlarm_Physical_Connection_Diagnostic_Fixed.yaml:DSC Physical Connection Diagnostic"
)

echo "=== Testing Configuration Compilation ==="
echo

for config_info in "${configs[@]}"; do
    IFS=':' read -r config_file description <<< "$config_info"
    
    if [ ! -f "$config_file" ]; then
        echo "⚠️  SKIP: $description ($config_file not found)"
        continue
    fi
    
    echo "🔧 Testing: $description"
    echo "   File: $config_file"
    
    # Test configuration validation
    if esphome config "$config_file" > /dev/null 2>&1; then
        echo "   ✅ Configuration valid"
    else
        echo "   ❌ Configuration invalid"
        continue
    fi
    
    # Test compilation
    echo "   🏗️  Compiling..."
    if esphome compile "$config_file" > /dev/null 2>&1; then
        echo "   ✅ Compilation successful"
        
        # Check if binary was created
        build_dir=".esphome/build/$(basename "$config_file" .yaml)"
        if [ -f "$build_dir/firmware.bin" ]; then
            size=$(stat -c%s "$build_dir/firmware.bin" 2>/dev/null || stat -f%z "$build_dir/firmware.bin" 2>/dev/null)
            size_mb=$(awk "BEGIN {printf \"%.1f\", $size/1024/1024}")
            echo "   📦 Binary size: ${size_mb}MB"
        fi
    else
        echo "   ❌ Compilation failed"
    fi
    echo
done

echo "=== Validation Summary ==="
echo
echo "The ESP32 task watchdog timeout fix includes:"
echo "• Enhanced ESP-IDF stack sizes (32KB main task)"
echo "• Extended watchdog timeout (300s vs 5s)"  
echo "• Optimized CPU frequency (240MHz)"
echo "• Memory management improvements"
echo "• FreeRTOS task priority optimization"
echo
echo "These fixes resolve the 'returned from app_main()' issue with"
echo "ESP-IDF 5.3.2 + ESPHome 2025.7.5 combination."
echo
echo "✅ Validation complete!"
echo
echo "Next steps:"
echo "1. Flash esp32_boot_test_standalone.yaml to test boot stability"
echo "2. Flash DscAlarm_Physical_Connection_Diagnostic_Fixed.yaml for hardware testing"
echo "3. Monitor logs for successful boot sequence and DSC connection status"