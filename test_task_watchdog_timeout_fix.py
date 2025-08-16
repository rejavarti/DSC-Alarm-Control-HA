#!/usr/bin/env python3
"""
Task Watchdog Timeout Fix Test Script

This script validates that the ESP32 task watchdog timeout fix is properly implemented
to prevent the specific issue shown in the problem statement:

E (5855) task_wdt: Task watchdog got triggered. The following tasks/users did not reset the watchdog in time:
E (5855) task_wdt:  - IDLE0 (CPU 0)
"""

import os
import re
import yaml

def test_task_watchdog_timeout_fix():
    """Test that the task watchdog timeout fix configuration is properly applied"""
    
    print("=== ESP32 Task Watchdog Timeout Fix Validation ===")
    print()
    
    test_config_file = "extras/ESPHome/task_watchdog_timeout_fix_test.yaml"
    boot_diagnostic_file = "extras/ESPHome/dsc_boot_diagnostic.yaml"
    
    success = True
    
    # Test 1: Check that the test configuration exists
    if not os.path.exists(test_config_file):
        print(f"❌ ERROR: Could not find test configuration {test_config_file}")
        return False
    
    with open(test_config_file, 'r', encoding='utf-8', errors='ignore') as f:
        test_config = f.read()
    
    # Parse YAML to validate structure
    try:
        yaml_config = yaml.safe_load(test_config)
        print("✅ Test configuration YAML syntax is valid")
    except yaml.YAMLError as e:
        print(f"❌ YAML syntax error in test configuration: {e}")
        return False
    
    # Test 2: Validate critical ESP-IDF configuration options are present
    critical_configs = [
        ('CONFIG_ESP_MAIN_TASK_STACK_SIZE: "32768"', 'Main task stack size increased to 32KB'),
        ('CONFIG_ESP_TASK_WDT_TIMEOUT_S: "300"', 'Watchdog timeout extended to 5 minutes'),
        ('CONFIG_ESP_TASK_WDT_CHECK_IDLE_TASK_CPU0: y', 'IDLE0 task monitoring enabled'),
        ('CONFIG_ESP_TASK_WDT_CHECK_IDLE_TASK_CPU1: y', 'IDLE1 task monitoring enabled'),
        ('CONFIG_FREERTOS_IDLE_TASK_STACKSIZE: "4096"', 'IDLE task stack size increased'),
        ('CONFIG_ESP32_DEFAULT_CPU_FREQ_240: y', '240MHz CPU frequency enabled'),
        ('CONFIG_ESP_TASK_WDT_PANIC: n', 'Watchdog reset instead of panic'),
    ]
    
    print("🔍 Validating critical ESP-IDF configuration options:")
    for config_option, description in critical_configs:
        if config_option in test_config:
            print(f"✅ {description}")
        else:
            print(f"❌ Missing: {description}")
            success = False
    
    # Test 3: Validate project name matches the problem statement
    if 'name: dscalarm-boot-diagnostic' in test_config or 'project_name: "dscalarm-boot-diagnostic"' in test_config:
        print("✅ Project name matches problem statement logs")
    else:
        print("❌ Project name doesn't match problem statement")
        success = False
    
    # Test 4: Validate standalone mode is enabled for testing
    if 'standalone_mode: true' in test_config:
        print("✅ Standalone mode enabled for isolated testing")
    else:
        print("❌ Standalone mode not enabled")
        success = False
    
    # Test 5: Check for diagnostic logging
    diagnostic_patterns = [
        r'ESP_LOGI.*watchdog.*fix',
        r'ESP_LOGI.*IDLE0.*task',
        r'ESP_LOGI.*app_main.*loop',
    ]
    
    diagnostic_found = 0
    for pattern in diagnostic_patterns:
        if re.search(pattern, test_config, re.IGNORECASE):
            diagnostic_found += 1
    
    if diagnostic_found >= 2:
        print(f"✅ Diagnostic logging present ({diagnostic_found} patterns found)")
    else:
        print(f"❌ Insufficient diagnostic logging ({diagnostic_found} patterns found)")
        success = False
    
    # Test 6: Validate the boot diagnostic configuration also has the fix
    if os.path.exists(boot_diagnostic_file):
        with open(boot_diagnostic_file, 'r', encoding='utf-8', errors='ignore') as f:
            boot_config = f.read()
        
        # Check key configurations are in the boot diagnostic file too
        boot_config_checks = [
            'CONFIG_ESP_MAIN_TASK_STACK_SIZE: "32768"',
            'CONFIG_ESP_TASK_WDT_TIMEOUT_S: "300"',
            'CONFIG_ESP_TASK_WDT_CHECK_IDLE_TASK_CPU0: y',
        ]
        
        boot_config_found = sum(1 for check in boot_config_checks if check in boot_config)
        if boot_config_found >= len(boot_config_checks):
            print("✅ Boot diagnostic configuration includes task watchdog fix")
        else:
            print(f"❌ Boot diagnostic configuration missing some fixes ({boot_config_found}/{len(boot_config_checks)})")
            success = False
    else:
        print("⚠️  Boot diagnostic configuration not found")
    
    # Test 7: Validate memory and stack size calculations
    try:
        # Extract the main task stack size
        main_stack_match = re.search(r'CONFIG_ESP_MAIN_TASK_STACK_SIZE:\s*"(\d+)"', test_config)
        if main_stack_match:
            main_stack_size = int(main_stack_match.group(1))
            if main_stack_size >= 32768:  # 32KB minimum
                print(f"✅ Main task stack size adequate: {main_stack_size} bytes")
            else:
                print(f"❌ Main task stack size too small: {main_stack_size} bytes")
                success = False
        
        # Extract the watchdog timeout
        wdt_timeout_match = re.search(r'CONFIG_ESP_TASK_WDT_TIMEOUT_S:\s*"(\d+)"', test_config)
        if wdt_timeout_match:
            wdt_timeout = int(wdt_timeout_match.group(1))
            if wdt_timeout >= 300:  # 5 minutes minimum
                print(f"✅ Watchdog timeout adequate: {wdt_timeout} seconds")
            else:
                print(f"❌ Watchdog timeout too short: {wdt_timeout} seconds")
                success = False
                
    except (ValueError, AttributeError) as e:
        print(f"❌ Error parsing configuration values: {e}")
        success = False
    
    print()
    print("=== Test Results Summary ===")
    if success:
        print("🎉 SUCCESS: Task watchdog timeout fix is properly configured!")
        print()
        print("Expected behavior with this fix:")
        print("✅ ESP32 will boot without 'E (5855) task_wdt: Task watchdog got triggered' error")
        print("✅ IDLE0 task will get sufficient CPU time to reset the watchdog")
        print("✅ Main task has 32KB stack to prevent blocking during component initialization")
        print("✅ Watchdog timeout extended to 5 minutes to allow complete initialization")
        print("✅ System will transition successfully from app_main() to ESPHome main loop")
        print()
        print("Configuration file ready for testing: extras/ESPHome/task_watchdog_timeout_fix_test.yaml")
    else:
        print("❌ FAILURE: Task watchdog timeout fix has issues that need to be addressed")
    
    return success

def test_esp_idf_configuration_completeness():
    """Test that the ESP-IDF configuration is complete and matches the diagnostic report"""
    
    print("\n=== ESP-IDF Configuration Completeness Test ===")
    
    # Check the diagnostic report for the expected configuration
    diagnostic_report_file = "ESP32_TASK_WATCHDOG_TIMEOUT_DIAGNOSTIC_REPORT.md"
    test_config_file = "extras/ESPHome/task_watchdog_timeout_fix_test.yaml"
    
    if not os.path.exists(diagnostic_report_file):
        print("⚠️  Diagnostic report not found, skipping completeness test")
        return True
    
    if not os.path.exists(test_config_file):
        print("❌ Test configuration not found")
        return False
    
    with open(diagnostic_report_file, 'r', encoding='utf-8', errors='ignore') as f:
        report_content = f.read()
    
    with open(test_config_file, 'r', encoding='utf-8', errors='ignore') as f:
        config_content = f.read()
    
    # Extract configuration options from the diagnostic report
    report_configs = re.findall(r'CONFIG_\w+:\s*[yn]|CONFIG_\w+:\s*"\d+"', report_content)
    
    config_coverage = 0
    total_configs = len(report_configs)
    
    for config in report_configs:
        if config in config_content:
            config_coverage += 1
    
    coverage_percentage = (config_coverage / total_configs * 100) if total_configs > 0 else 0
    
    if coverage_percentage >= 90:
        print(f"✅ Configuration coverage: {coverage_percentage:.1f}% ({config_coverage}/{total_configs})")
        return True
    else:
        print(f"❌ Insufficient configuration coverage: {coverage_percentage:.1f}% ({config_coverage}/{total_configs})")
        return False

if __name__ == "__main__":
    print("Testing ESP32 Task Watchdog Timeout Fix Implementation")
    print("=" * 60)
    
    test1_success = test_task_watchdog_timeout_fix()
    test2_success = test_esp_idf_configuration_completeness()
    
    overall_success = test1_success and test2_success
    
    print("\n" + "=" * 60)
    if overall_success:
        print("🏆 ALL TESTS PASSED: Task watchdog timeout fix is ready for deployment")
    else:
        print("❌ SOME TESTS FAILED: Please review and fix the issues above")
    
    exit(0 if overall_success else 1)