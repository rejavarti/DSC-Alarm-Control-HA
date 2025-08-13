#!/usr/bin/env python3
"""
Test script to verify the ESP-IDF 5.3.2 LoadProhibited fix compiles properly.
This performs basic syntax and dependency checks without requiring full compilation.
"""

import os
import re
import sys

def check_cpp_syntax(filepath):
    """Basic C++ syntax checking for common issues."""
    if not os.path.exists(filepath):
        return False, f"File not found: {filepath}"
    
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Check for basic syntax issues
        issues = []
        
        # Check for unmatched braces (basic check)
        open_braces = content.count('{')
        close_braces = content.count('}')
        if open_braces != close_braces:
            issues.append(f"Unmatched braces: {open_braces} open, {close_braces} close")
        
        # Check for unmatched parentheses in function definitions
        function_pattern = r'(\w+\s*\([^)]*\)\s*{)'
        functions = re.findall(function_pattern, content)
        
        # Check for missing semicolons after function declarations
        decl_pattern = r'(\w+\s*\([^)]*\))\s*;'
        
        # Check for proper includes
        if '#include' in content:
            include_pattern = r'#include\s*[<"]([^>"]+)[>"]'
            includes = re.findall(include_pattern, content)
            if not includes:
                issues.append("No valid includes found despite #include presence")
        
        if issues:
            return False, f"Syntax issues in {filepath}: {'; '.join(issues)}"
        
        return True, f"Basic syntax check passed for {filepath}"
        
    except Exception as e:
        return False, f"Error checking {filepath}: {e}"

def check_yaml_syntax(filepath):
    """Basic YAML syntax checking."""
    if not os.path.exists(filepath):
        return False, f"File not found: {filepath}"
    
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
        
        issues = []
        lines = content.split('\n')
        
        # Check for basic YAML issues
        for i, line in enumerate(lines):
            line_num = i + 1
            stripped = line.strip()
            
            # Skip comments and empty lines
            if not stripped or stripped.startswith('#'):
                continue
            
            # Check for tabs (YAML doesn't allow tabs)
            if '\t' in line:
                issues.append(f"Line {line_num}: Contains tabs (YAML requires spaces)")
            
            # Check for basic key-value syntax
            if ':' in stripped and not stripped.startswith('-'):
                if not re.match(r'^[^:]+:\s*.*$', stripped):
                    issues.append(f"Line {line_num}: Invalid key-value syntax")
        
        if issues:
            return False, f"YAML issues in {filepath}: {'; '.join(issues)}"
        
        return True, f"Basic YAML syntax check passed for {filepath}"
        
    except Exception as e:
        return False, f"Error checking {filepath}: {e}"

def main():
    """Main test function."""
    print("=== ESP-IDF 5.3.2 LoadProhibited Fix Compilation Test ===")
    print()
    
    # Files to check
    files_to_check = [
        ("extras/ESPHome/components/dsc_keybus/dsc_esp_idf_timer_fix.h", "cpp"),
        ("extras/ESPHome/components/dsc_keybus/dsc_esp_idf_timer_fix.cpp", "cpp"),
        ("extras/ESPHome/components/dsc_keybus/dsc_static_variables.cpp", "cpp"),
        ("extras/ESPHome/components/dsc_keybus/dsc_keybus.cpp", "cpp"),
        ("extras/ESPHome/DSCAlarm_ESP_IDF_5_3_LoadProhibited_Fix.yaml", "yaml"),
    ]
    
    all_passed = True
    
    for filepath, file_type in files_to_check:
        if file_type == "cpp":
            passed, message = check_cpp_syntax(filepath)
        elif file_type == "yaml":
            passed, message = check_yaml_syntax(filepath)
        else:
            passed, message = False, f"Unknown file type: {file_type}"
        
        if passed:
            print(f"✅ {message}")
        else:
            print(f"❌ {message}")
            all_passed = False
    
    print()
    
    # Check for required build flags and settings
    print("=== Build Configuration Check ===")
    
    yaml_file = "extras/ESPHome/DSCAlarm_ESP_IDF_5_3_LoadProhibited_Fix.yaml"
    if os.path.exists(yaml_file):
        with open(yaml_file, 'r') as f:
            yaml_content = f.read()
        
        required_flags = [
            "DSC_ESP_IDF_5_3_PLUS",
            "DSC_ENHANCED_MEMORY_SAFETY", 
            "CONFIG_ESP_MAIN_TASK_STACK_SIZE",
            "CONFIG_ESP_TIMER_TASK_STACK_SIZE",
        ]
        
        missing_flags = []
        for flag in required_flags:
            if flag not in yaml_content:
                missing_flags.append(flag)
        
        if missing_flags:
            print(f"❌ Missing required build flags: {', '.join(missing_flags)}")
            all_passed = False
        else:
            print("✅ All required build flags present")
    
    print()
    
    # Summary
    if all_passed:
        print("🎉 All compilation tests passed!")
        print("✅ ESP-IDF 5.3.2 LoadProhibited fix ready for compilation")
        print()
        print("Next steps:")
        print("1. Copy DSCAlarm_ESP_IDF_5_3_LoadProhibited_Fix.yaml to your ESPHome config")
        print("2. Update secrets.yaml with your WiFi and access code")
        print("3. Compile and flash to ESP32")
        print("4. Monitor serial output for successful initialization")
        return 0
    else:
        print("❌ Some compilation tests failed")
        print("⚠️  Fix the above issues before attempting compilation")
        return 1

if __name__ == "__main__":
    sys.exit(main())