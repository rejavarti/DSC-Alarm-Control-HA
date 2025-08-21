#!/usr/bin/env python3
"""
DSC Classic Rate Limiting Fix Validation Script
Validates that DSC Classic panel configurations have the correct settings
to benefit from the rate limiting fix (300 attempts instead of 100).
"""

import yaml
import sys
import os

def validate_dsc_classic_config(config_file):
    """Validate a DSC Classic configuration file"""
    if not os.path.exists(config_file):
        print(f"❌ Configuration file not found: {config_file}")
        return False
    
    try:
        with open(config_file, 'r') as f:
            config = yaml.safe_load(f)
    except Exception as e:
        print(f"❌ Error parsing YAML file: {e}")
        return False
    
    print(f"🔍 Validating DSC Classic rate limiting fix in: {config_file}")
    print("=" * 70)
    
    issues = []
    recommendations = []
    
    # Check if dsc_keybus component exists
    if 'dsc_keybus' not in config:
        issues.append("No 'dsc_keybus' section found in configuration")
        return False
    
    dsc_config = config['dsc_keybus']
    
    # Check critical settings for Classic panels
    print("📋 Checking Critical Settings for Classic Panel Rate Limiting Fix:")
    print("-" * 50)
    
    # 1. Classic timing mode
    classic_timing = dsc_config.get('classic_timing_mode', False)
    if classic_timing:
        print("✅ classic_timing_mode: ENABLED (will get 300 attempts)")
    else:
        print("❌ classic_timing_mode: DISABLED (will only get 100 attempts)")
        issues.append("Classic timing mode must be enabled for Classic panels")
        recommendations.append("Add 'classic_timing_mode: true' to your dsc_keybus section")
    
    # 2. Standalone mode
    standalone = dsc_config.get('standalone_mode', False)
    if not standalone:
        print("✅ standalone_mode: FALSE (correct for real panel connection)")
    else:
        print("⚠️  standalone_mode: TRUE (panel connection testing will be skipped)")
        recommendations.append("Set 'standalone_mode: false' for real panel connection")
    
    # 3. Series type
    series_type = dsc_config.get('series_type', 'Classic')
    if series_type == 'Classic':
        print("✅ series_type: Classic (correct for Classic panels)")
    else:
        print(f"⚠️  series_type: {series_type} (make sure this matches your panel)")
    
    # 4. Timing settings
    print("\n📊 Timing Configuration:")
    print("-" * 30)
    
    hardware_delay = dsc_config.get('hardware_detection_delay', 2000)
    init_timeout = dsc_config.get('initialization_timeout', 30000)
    retry_delay = dsc_config.get('retry_delay', 2000)
    
    print(f"  Hardware detection delay: {hardware_delay}ms", end="")
    if hardware_delay >= 8000:
        print(" ✅ (recommended for Classic)")
    elif hardware_delay >= 5000:
        print(" ⚠️  (consider increasing to 8000ms)")
    else:
        print(" ❌ (too low for Classic panels)")
        recommendations.append("Increase hardware_detection_delay to 8000ms for Classic panels")
    
    print(f"  Initialization timeout: {init_timeout}ms", end="")
    if init_timeout >= 45000:
        print(" ✅ (recommended for Classic)")
    elif init_timeout >= 30000:
        print(" ⚠️  (consider increasing to 45000ms)")
    else:
        print(" ❌ (may be too low for Classic panels)")
        recommendations.append("Increase initialization_timeout to 45000ms for Classic panels")
    
    print(f"  Retry delay: {retry_delay}ms", end="")
    if retry_delay >= 3000:
        print(" ✅ (recommended for Classic)")
    else:
        print(" ⚠️  (consider increasing to 3000ms)")
        recommendations.append("Increase retry_delay to 3000ms for Classic panels")
    
    # 5. External components
    print("\n🔧 Component Configuration:")
    print("-" * 30)
    
    external_components = config.get('external_components', [])
    if external_components:
        for comp in external_components:
            components = comp.get('components', [])
            if 'dsc_keybus' in components:
                print("✅ Using main 'dsc_keybus' component (supports Classic timing mode)")
            elif 'dsc_keybus_minimal' in components:
                print("✅ Using 'dsc_keybus_minimal' component (now supports Classic timing mode)")
            else:
                print(f"⚠️  Unknown components: {components}")
    else:
        print("⚠️  No external_components section found")
    
    # Summary
    print("\n" + "=" * 70)
    if not issues:
        print("🎉 Configuration looks good for DSC Classic rate limiting fix!")
        print("   Your Classic panel should now get 300 initialization attempts.")
    else:
        print("⚠️  Configuration issues found:")
        for issue in issues:
            print(f"   • {issue}")
    
    if recommendations:
        print("\n💡 Recommendations for optimal Classic panel support:")
        for rec in recommendations:
            print(f"   • {rec}")
    
    print("\n📖 Expected log messages after applying this fix:")
    print("   ✅ '[D][dsc_keybus]: Classic timing mode enabled - applying extended delays'")
    print("   ✅ '[D][dsc_keybus]: Classic timing mode rate limiting: allowing 300 attempts'")
    print("   ✅ '[I][dsc_keybus]: DSC Keybus hardware initialization complete'")
    
    return len(issues) == 0

def main():
    if len(sys.argv) != 2:
        print("Usage: python validate_classic_rate_limiting_fix.py <config_file.yaml>")
        print("\nExamples:")
        print("  python validate_classic_rate_limiting_fix.py dscalarm.yaml")
        print("  python validate_classic_rate_limiting_fix.py dsc_classic_enhanced_connection_fix.yaml")
        sys.exit(1)
    
    config_file = sys.argv[1]
    success = validate_dsc_classic_config(config_file)
    
    if success:
        print("\n✅ Validation passed! Configuration should work with the rate limiting fix.")
        sys.exit(0)
    else:
        print("\n❌ Validation failed! Please fix the issues above.")
        sys.exit(1)

if __name__ == "__main__":
    main()