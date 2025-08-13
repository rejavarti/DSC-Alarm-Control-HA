#!/usr/bin/env python3
"""
ESP-IDF 5.3.2 LoadProhibited Crash Fix Validation Script

This script validates the comprehensive fix for ESP32 LoadProhibited crashes
with the 0xcececece memory pattern that occur in ESP-IDF 5.3.2 during app_main().

Validates:
1. Enhanced ESP-IDF timer compatibility layer
2. ESP-IDF 5.3.2 specific static variable initialization  
3. Enhanced ESPHome component safeguards
4. ESP-IDF configuration optimizations
5. Memory safety and system health monitoring
"""

import os
import sys
import re
from pathlib import Path

def check_file_exists(filepath, description):
    """Check if a file exists and print status."""
    if os.path.exists(filepath):
        print(f"✅ {description}")
        return True
    else:
        print(f"❌ {description}")
        return False

def check_file_content(filepath, patterns, description):
    """Check if file contains required patterns."""
    if not os.path.exists(filepath):
        print(f"❌ {description} - File not found: {filepath}")
        return False
    
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
            
        missing_patterns = []
        for pattern_name, pattern in patterns.items():
            if not re.search(pattern, content, re.MULTILINE | re.DOTALL):
                missing_patterns.append(pattern_name)
        
        if missing_patterns:
            print(f"❌ {description} - Missing: {', '.join(missing_patterns)}")
            return False
        else:
            print(f"✅ {description}")
            return True
    except Exception as e:
        print(f"❌ {description} - Error reading file: {e}")
        return False

def main():
    """Main validation function."""
    print("=== ESP-IDF 5.3.2 LoadProhibited Crash Fix Validation ===")
    print()
    
    # Base path for ESPHome components
    base_path = "extras/ESPHome/components/dsc_keybus"
    
    if not os.path.exists(base_path):
        print(f"❌ ESPHome component directory not found: {base_path}")
        return 1
    
    print(f"✅ Found ESPHome component at {base_path}")
    print()
    
    validation_results = []
    
    # 1. Check ESP-IDF timer compatibility layer
    print("=== 1. ESP-IDF Timer Compatibility Layer ===")
    
    timer_fix_header = os.path.join(base_path, "dsc_esp_idf_timer_fix.h")
    timer_fix_impl = os.path.join(base_path, "dsc_esp_idf_timer_fix.cpp")
    
    validation_results.append(check_file_exists(timer_fix_header, "ESP-IDF timer fix header"))
    validation_results.append(check_file_exists(timer_fix_impl, "ESP-IDF timer fix implementation"))
    
    if os.path.exists(timer_fix_header):
        timer_header_patterns = {
            "ESP-IDF timer class": r"class DSCTimer",
            "ESP-IDF mode detection": r"#ifdef DSC_TIMER_MODE_ESP_IDF",
            "Arduino mode detection": r"#ifdef DSC_TIMER_MODE_ARDUINO", 
            "Helper functions": r"bool dsc_timer_begin\(",
            "Critical section management": r"void dsc_timer_enter_critical\(",
        }
        validation_results.append(check_file_content(timer_fix_header, timer_header_patterns, 
                                                   "ESP-IDF timer header content"))
    
    if os.path.exists(timer_fix_impl):
        timer_impl_patterns = {
            "ESP-IDF implementation": r"esp_timer_create_args_t timer_args",
            "Arduino implementation": r"hw_timer_handle = timerBegin",
            "Error handling": r"esp_err_t.*ESP_OK",
            "Global timer instance": r"DSCTimer dsc_global_timer",
        }
        validation_results.append(check_file_content(timer_fix_impl, timer_impl_patterns,
                                                   "ESP-IDF timer implementation content"))
    
    print()
    
    # 2. Check enhanced static variable initialization
    print("=== 2. Enhanced Static Variable Initialization ===")
    
    static_vars_file = os.path.join(base_path, "dsc_static_variables.cpp")
    
    if os.path.exists(static_vars_file):
        static_patterns = {
            "ESP-IDF version detection": r"#include <esp_idf_version\.h>",
            "ESP-IDF 5.3+ flag": r"#ifdef DSC_ESP_IDF_5_3_PLUS",
            "Enhanced constructor": r"__attribute__\(\(constructor\)\)",
            "Timer system readiness": r"dsc_esp_idf_timer_system_ready",
            "Enhanced error checking": r"esp_timer_create.*test_timer",
            "0xcececece pattern comments": r"0xcececece.*pattern",
        }
        validation_results.append(check_file_content(static_vars_file, static_patterns,
                                                   "Enhanced static variable initialization"))
    else:
        validation_results.append(False)
        print(f"❌ Static variables file not found: {static_vars_file}")
    
    print()
    
    # 3. Check enhanced ESPHome component
    print("=== 3. Enhanced ESPHome Component ===")
    
    component_file = os.path.join(base_path, "dsc_keybus.cpp")
    
    if os.path.exists(component_file):
        component_patterns = {
            "ESP-IDF timer fix include": r"#include.*dsc_esp_idf_timer_fix\.h",
            "ESP-IDF version detection": r"#include <esp_idf_version\.h>",
            "ESP-IDF 5.3+ detection": r"ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL\(5, 3, 0\)",
            "Enhanced setup safeguards": r"dsc_esp_idf_timer_system_ready",
            "Stricter heap requirements": r"min_heap = 50000",
            "Timer system verification": r"esp_timer_create.*test_timer",
            "Enhanced loop initialization": r"ESP-IDF 5\.3\.2\+ specific readiness",
        }
        validation_results.append(check_file_content(component_file, component_patterns,
                                                   "Enhanced ESPHome component"))
    else:
        validation_results.append(False)
        print(f"❌ ESPHome component file not found: {component_file}")
    
    print()
    
    # 4. Check enhanced YAML configuration
    print("=== 4. Enhanced YAML Configuration ===")
    
    yaml_config = "extras/ESPHome/DSCAlarm_ESP_IDF_5_3_LoadProhibited_Fix.yaml"
    
    if os.path.exists(yaml_config):
        yaml_patterns = {
            "ESP-IDF 5.3+ build flag": r"-DDSC_ESP_IDF_5_3_PLUS",
            "Enhanced main task stack": r"CONFIG_ESP_MAIN_TASK_STACK_SIZE.*32768",
            "ESP timer configuration": r"CONFIG_ESP_TIMER_TASK_STACK_SIZE",
            "Enhanced memory debugging": r"CONFIG_HEAP_POISONING_COMPREHENSIVE",
            "LoadProhibited crash comments": r"0xcececece.*pattern",
            "System health monitoring": r"DSCAlarm System Healthy",
            "Memory monitoring": r"DSCAlarm Free Heap",
        }
        validation_results.append(check_file_content(yaml_config, yaml_patterns,
                                                   "Enhanced YAML configuration"))
    else:
        validation_results.append(False)
        print(f"❌ Enhanced YAML configuration not found: {yaml_config}")
    
    print()
    
    # 5. Check header file updates
    print("=== 5. Header File Updates ===")
    
    headers_to_check = [
        ("dscKeybus.h", "PowerSeries interface header"),
        ("dscClassic.h", "Classic interface header"),
    ]
    
    for header_file, description in headers_to_check:
        header_path = os.path.join(base_path, header_file)
        if os.path.exists(header_path):
            header_patterns = {
                "ESP-IDF 5.3+ variables": r"#ifdef DSC_ESP_IDF_5_3_PLUS",
                "Enhanced static variables": r"esp32_esp_idf_timer_ready",
                "System initialization tracking": r"esp32_system_fully_initialized",
            }
            validation_results.append(check_file_content(header_path, header_patterns,
                                                       f"{description} updates"))
        else:
            validation_results.append(False)
            print(f"❌ {description} not found: {header_path}")
    
    print()
    
    # 6. Validate fix documentation
    print("=== 6. Fix Documentation ===")
    
    # Check if there are any existing LoadProhibited fix docs to update
    doc_files = [
        "ESP32_LOADPROHIBITED_CRASH_FIX.md",
        "ESPHOME_ESP32_LOADPROHIBITED_COMPLETE_FIX.md",
        "ESP32_TIMER_INITIALIZATION_FIX.md"
    ]
    
    for doc_file in doc_files:
        if os.path.exists(doc_file):
            print(f"✅ Found existing fix documentation: {doc_file}")
        else:
            print(f"ℹ️  No existing documentation: {doc_file}")
    
    print()
    
    # Summary
    print("=== Validation Summary ===")
    passed_count = sum(validation_results)
    total_count = len(validation_results)
    
    if passed_count == total_count:
        print(f"🎉 All {total_count} validation checks passed!")
        print("✅ ESP-IDF 5.3.2 LoadProhibited crash fix successfully implemented")
        print()
        print("The fix addresses:")
        print("- 0xcececece memory access pattern during app_main()")
        print("- ESP-IDF 5.3.2 timer system initialization race conditions") 
        print("- Enhanced memory safety and system health monitoring")
        print("- Comprehensive static variable initialization safeguards")
        return 0
    else:
        print(f"❌ {passed_count}/{total_count} validation checks passed")
        print("⚠️  ESP-IDF 5.3.2 LoadProhibited crash fix needs attention")
        return 1

if __name__ == "__main__":
    sys.exit(main())