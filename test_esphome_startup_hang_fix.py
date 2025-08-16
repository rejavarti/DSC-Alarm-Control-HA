#!/usr/bin/env python3
"""
Test script to validate the ESPHome app_main() return hang fix
"""

import os
import sys
import re
import yaml

def test_esphome_startup_hang_fix():
    """Test that the ESP32 startup hang fix is properly implemented"""
    print("Testing ESPHome 'Returned from app_main()' Hang Fix...")
    print("=" * 60)
    
    # Test the fixed configuration file
    config_path = "extras/ESPHome/dscalarm-esphome-startup-fix.yaml"
    
    if not os.path.exists(config_path):
        print(f"❌ Fixed configuration file not found: {config_path}")
        return False
    
    # Load and validate the configuration
    try:
        with open(config_path, 'r') as f:
            config_content = f.read()
            
        # Parse YAML to validate structure
        config = yaml.safe_load(config_content)
        
    except Exception as e:
        print(f"❌ Failed to load configuration: {e}")
        return False
    
    print(f"✅ Configuration file loaded: {config_path}")
    
    # Test ESP-IDF configuration
    print("\n=== ESP-IDF Configuration Validation ===")
    
    esp32_config = config.get('esp32', {})
    framework = esp32_config.get('framework', {})
    sdkconfig = framework.get('sdkconfig_options', {})
    
    # Critical configuration checks
    critical_configs = {
        'CONFIG_ESP_MAIN_TASK_STACK_SIZE': '32768',
        'CONFIG_ESP_MAIN_TASK_AFFINITY_CPU0': 'y',
        'CONFIG_ESP_TASK_WDT_TIMEOUT_S': '300',
        'CONFIG_ESP_TASK_WDT_PANIC': 'n',
        'CONFIG_FREERTOS_IDLE_TASK_STACKSIZE': '4096',
        'CONFIG_ESP32_DEFAULT_CPU_FREQ_240': 'y',
        'CONFIG_PM_ENABLE': 'n'
    }
    
    all_configs_present = True
    for config_key, expected_value in critical_configs.items():
        if config_key in sdkconfig:
            actual_value = str(sdkconfig[config_key]).strip('"')
            if actual_value == expected_value:
                print(f"✅ {config_key}: {actual_value}")
            else:
                print(f"❌ {config_key}: expected {expected_value}, got {actual_value}")
                all_configs_present = False
        else:
            print(f"❌ Missing critical configuration: {config_key}")
            all_configs_present = False
    
    # Test component configuration
    print("\n=== Component Configuration Validation ===")
    
    dsc_config = config.get('dsc_keybus', {})
    
    # Check standalone mode is enabled for testing
    standalone_mode = dsc_config.get('standalone_mode', False)
    if standalone_mode:
        print("✅ Standalone mode enabled for safe testing")
    else:
        print("⚠️  Standalone mode disabled - may require actual DSC panel")
    
    # Check debug output is enabled
    debug_level = dsc_config.get('debug', 0)
    if debug_level > 0:
        print(f"✅ Debug logging enabled: level {debug_level}")
    else:
        print("⚠️  Debug logging disabled")
    
    # Test monitoring components
    print("\n=== Monitoring Components Validation ===")
    
    # Check for startup monitoring
    has_uptime_sensor = False
    has_health_sensor = False
    has_startup_interval = False
    
    sensors = config.get('sensor', [])
    for sensor in sensors:
        if sensor.get('platform') == 'uptime':
            has_uptime_sensor = True
            print("✅ Uptime sensor present")
            break
    
    binary_sensors = config.get('binary_sensor', [])
    for sensor in binary_sensors:
        if 'ESPHome Started' in sensor.get('name', ''):
            has_health_sensor = True
            print("✅ ESPHome startup detection sensor present")
            break
    
    intervals = config.get('interval', [])
    for interval in intervals:
        if 'startup_logged' in str(interval.get('then', [])):
            has_startup_interval = True
            print("✅ Startup monitoring interval present")
            break
    
    # Test logging configuration
    print("\n=== Logging Configuration Validation ===")
    
    logger_config = config.get('logger', {})
    log_level = logger_config.get('level', 'INFO')
    baud_rate = logger_config.get('baud_rate', 115200)
    
    print(f"✅ Log level: {log_level}")
    print(f"✅ Baud rate: {baud_rate}")
    
    # Check for essential log categories
    logs_config = logger_config.get('logs', {})
    essential_logs = ['app', 'component', 'dsc_keybus']
    for log_category in essential_logs:
        if log_category in logs_config:
            print(f"✅ {log_category} logging configured")
        else:
            print(f"⚠️  {log_category} logging not explicitly configured")
    
    # Overall assessment
    print("\n=== Overall Assessment ===")
    
    if all_configs_present and has_uptime_sensor and has_health_sensor:
        print("✅ ESPHome startup hang fix is properly implemented")
        print("✅ Configuration should resolve 'Returned from app_main()' hang")
        return True
    else:
        print("❌ Fix implementation is incomplete")
        if not all_configs_present:
            print("   - Missing critical ESP-IDF configurations")
        if not has_uptime_sensor:
            print("   - Missing uptime monitoring")
        if not has_health_sensor:
            print("   - Missing startup detection")
        return False

def test_main_config_updated():
    """Test that the main ESPHome configuration has been updated"""
    print("\n" + "=" * 60)
    print("Testing Main Configuration Update...")
    
    main_config_path = "extras/ESPHome/DscAlarm_ESP32.yaml"
    
    if not os.path.exists(main_config_path):
        print(f"❌ Main configuration file not found: {main_config_path}")
        return False
    
    with open(main_config_path, 'r') as f:
        content = f.read()
    
    # Check for updated ESP-IDF configurations
    critical_updates = [
        'CONFIG_ESP_MAIN_TASK_STACK_SIZE: "32768"',
        'CONFIG_ESP_MAIN_TASK_AFFINITY_CPU0: y',
        'CONFIG_ESP_TASK_WDT_TIMEOUT_S: "300"',
        'CONFIG_FREERTOS_IDLE_TASK_STACKSIZE: "4096"'
    ]
    
    all_present = True
    for update in critical_updates:
        if update in content:
            print(f"✅ Found: {update}")
        else:
            print(f"❌ Missing: {update}")
            all_present = False
    
    if all_present:
        print("✅ Main configuration has been updated with startup hang fix")
        return True
    else:
        print("❌ Main configuration needs updates")
        return False

def test_documentation_exists():
    """Test that proper documentation exists for the fix"""
    print("\n" + "=" * 60)
    print("Testing Documentation...")
    
    doc_path = "ESPHOME_APP_MAIN_RETURN_HANG_FIX.md"
    
    if not os.path.exists(doc_path):
        print(f"❌ Documentation file not found: {doc_path}")
        return False
    
    with open(doc_path, 'r') as f:
        content = f.read()
    
    # Check for key sections
    required_sections = [
        "Root Cause Analysis",
        "Solution Implementation", 
        "Expected Behavior After Fix",
        "Technical Details",
        "Validation"
    ]
    
    all_sections_present = True
    for section in required_sections:
        if section in content:
            print(f"✅ Documentation section present: {section}")
        else:
            print(f"❌ Missing documentation section: {section}")
            all_sections_present = False
    
    if all_sections_present:
        print("✅ Complete documentation provided")
        return True
    else:
        print("❌ Documentation is incomplete")
        return False

def main():
    """Run all tests"""
    print("ESPHome App_Main Return Hang Fix Validation")
    print("=" * 60)
    
    results = []
    
    # Run individual tests
    results.append(test_esphome_startup_hang_fix())
    results.append(test_main_config_updated())
    results.append(test_documentation_exists())
    
    # Summary
    print("\n" + "=" * 60)
    print("FINAL RESULTS")
    print("=" * 60)
    
    passed = sum(results)
    total = len(results)
    
    if passed == total:
        print(f"✅ ALL TESTS PASSED ({passed}/{total})")
        print("✅ ESPHome startup hang fix is properly implemented")
        print("✅ Configuration should resolve 'Returned from app_main()' hang issue")
        return 0
    else:
        print(f"❌ SOME TESTS FAILED ({passed}/{total})")
        print("❌ Fix implementation needs attention")
        return 1

if __name__ == "__main__":
    sys.exit(main())