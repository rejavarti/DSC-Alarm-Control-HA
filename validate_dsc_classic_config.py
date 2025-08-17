#!/usr/bin/env python3
"""
DSC Classic Panel Configuration Validator

This script validates that the user_dsc_config_classic_panel_fix.yaml
includes all necessary components for proper DSC Classic panel operation.

Usage: python3 validate_dsc_classic_config.py
"""

import sys
import os

def main():
    print("🔍 DSC Classic Panel Configuration Validator")
    print("=" * 50)
    
    config_file = "user_dsc_config_classic_panel_fix.yaml"
    
    if not os.path.exists(config_file):
        print(f"❌ Error: {config_file} not found in current directory")
        print("   Please run this script from the repository root directory.")
        return False
    
    try:
        with open(config_file, 'r') as f:
            content = f.read()
    except Exception as e:
        print(f"❌ Error reading {config_file}: {e}")
        return False
    
    print("📋 Checking Critical DSC Classic Panel Requirements...")
    
    checks = [
        # Critical PC16 Configuration
        {
            'name': 'PC16 Pin Configuration',
            'pattern': 'pc16_pin',
            'critical': True,
            'description': 'CRITICAL: PC16 pin required for Classic panels'
        },
        {
            'name': 'PC16 GPIO17 Assignment', 
            'pattern': 'GPIO17',
            'critical': True,
            'description': 'PC16 pin must be assigned to GPIO17'
        },
        
        # Series Configuration
        {
            'name': 'Classic Series Type',
            'pattern': 'series_type: "Classic"',
            'critical': True,
            'description': 'Series type must be explicitly set to Classic'
        },
        
        # Emergency Functions (missing from original)
        {
            'name': 'Fire Alarm Function',
            'pattern': 'fire_alarm',
            'critical': False,
            'description': 'Fire alarm functionality'
        },
        {
            'name': 'Aux Alarm Function',
            'pattern': 'aux_alarm', 
            'critical': False,
            'description': 'Auxiliary alarm functionality'
        },
        {
            'name': 'Panic Alarm Function',
            'pattern': 'panic_alarm',
            'critical': False,
            'description': 'Panic alarm functionality'
        },
        
        # Zone Alarm Monitoring (critical missing feature)
        {
            'name': 'Zone Alarm Monitoring',
            'pattern': 'zone_.*_alarm',
            'critical': True,
            'description': 'Zone alarm status monitoring (was missing from original)'
        },
        
        # Immediate Disarm
        {
            'name': 'Immediate Disarm',
            'pattern': 'accessCode',
            'critical': True,
            'description': 'Immediate disarm using access code'
        },
        
        # System Health
        {
            'name': 'System Health Monitoring',
            'pattern': 'uptime',
            'critical': False,
            'description': 'System health and uptime monitoring'
        },
        
        # Event Handlers
        {
            'name': 'Zone Alarm Event Handler',
            'pattern': 'on_zone_alarm_change',
            'critical': True,
            'description': 'Zone alarm change event handler (was missing)'
        }
    ]
    
    critical_passed = 0
    critical_total = 0
    optional_passed = 0
    optional_total = 0
    
    import re
    
    for check in checks:
        pattern = check['pattern']
        name = check['name']
        critical = check['critical']
        description = check['description']
        
        # Use regex search for patterns with regex syntax
        if '.*' in pattern:
            found = re.search(pattern, content, re.IGNORECASE)
        else:
            found = pattern in content
        
        if critical:
            critical_total += 1
            if found:
                critical_passed += 1
                print(f"✅ {name}: PASS")
            else:
                print(f"❌ {name}: FAIL - {description}")
        else:
            optional_total += 1
            if found:
                optional_passed += 1
                print(f"✅ {name}: PASS")
            else:
                print(f"⚠️  {name}: MISSING - {description}")
    
    print("\n📊 Validation Summary:")
    print(f"Critical Requirements: {critical_passed}/{critical_total}")
    print(f"Optional Features: {optional_passed}/{optional_total}")
    
    if critical_passed == critical_total:
        print("\n🎉 SUCCESS: All critical requirements met!")
        print("   Your DSC Classic panel configuration should work properly.")
        if optional_passed < optional_total:
            print(f"   Note: {optional_total - optional_passed} optional features missing but not required.")
        return True
    else:
        print(f"\n❌ FAILURE: {critical_total - critical_passed} critical requirements missing!")
        print("   Your configuration may not work properly with DSC Classic panels.")
        print("\n🔧 Required Actions:")
        print("   1. Ensure PC16 pin is connected to GPIO17 with 1kΩ resistor")
        print("   2. Set series_type to 'Classic' in configuration")
        print("   3. Add missing zone alarm monitoring")
        print("   4. Configure immediate disarm functionality")
        return False

if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)