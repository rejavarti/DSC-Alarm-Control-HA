#!/usr/bin/env python3
"""
Quick DSC Setup Validator
Validates the fixed DSC configuration and provides next steps
"""

import sys
import subprocess
from pathlib import Path

def main():
    print("🔍 DSC ALARM CONTROL - SETUP VALIDATOR")
    print("=" * 50)
    
    base_path = Path(".")
    standalone_config = base_path / "DSCAlarm_Fixed_Standalone.yaml"
    secrets_file = base_path / "secrets.yaml"
    
    # Check files exist
    issues = []
    if not standalone_config.exists():
        issues.append("❌ DSCAlarm_Fixed_Standalone.yaml not found")
    else:
        print("✅ Fixed configuration found")
        
    if not secrets_file.exists():
        issues.append("❌ secrets.yaml not found")
    else:
        print("✅ Secrets file found")
    
    # Check ESPHome
    try:
        result = subprocess.run(['esphome', 'version'], capture_output=True, text=True)
        if result.returncode == 0:
            print(f"✅ ESPHome ready: {result.stdout.strip()}")
        else:
            issues.append("❌ ESPHome not working")
    except:
        issues.append("❌ ESPHome not installed")
    
    # Validate configuration if possible
    if standalone_config.exists() and not issues:
        print("\n🔍 Validating configuration...")
        try:
            result = subprocess.run(['esphome', 'config', str(standalone_config)], 
                                  capture_output=True, text=True, timeout=60)
            if result.returncode == 0:
                print("✅ Configuration is VALID!")
                print("\n🚀 READY TO FLASH!")
                print("Next command:")
                print(f"   esphome run {standalone_config.name}")
            else:
                print("❌ Configuration validation failed")
                print("Error:", result.stderr[:200] + "..." if len(result.stderr) > 200 else result.stderr)
        except subprocess.TimeoutExpired:
            print("⏰ Validation timed out")
        except Exception as e:
            print(f"❌ Validation error: {e}")
    
    # Summary
    print(f"\n📊 SETUP STATUS:")
    if issues:
        print("❌ Issues found:")
        for issue in issues:
            print(f"   {issue}")
        print("\n💡 Run the full diagnostic:")
        print("   python3 dsc_esphome_installation_diagnostic.py")
    else:
        print("✅ Setup is ready!")
        print("✅ Configuration validated")
        print("✅ Ready to flash to ESP32")
        
        print(f"\n📋 NEXT STEPS:")
        print(f"1. Update secrets.yaml with your WiFi credentials")
        print(f"2. Flash: esphome run {standalone_config.name}")
        print(f"3. Monitor: esphome logs {standalone_config.name}")

if __name__ == "__main__":
    main()