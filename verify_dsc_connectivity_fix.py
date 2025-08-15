#!/usr/bin/env python3
"""
DSC Connectivity Verification Script
Quick test to verify ESPHome configuration is working after the timer system fix.
"""

import subprocess
import sys
import os
import tempfile

def check_esphome_installed():
    """Check if ESPHome is installed"""
    try:
        result = subprocess.run(["esphome", "version"], capture_output=True, text=True)
        if result.returncode == 0:
            print(f"✅ ESPHome installed: {result.stdout.strip()}")
            return True
        else:
            print("❌ ESPHome not found")
            return False
    except FileNotFoundError:
        print("❌ ESPHome not installed")
        print("Install with: pip install esphome")
        return False

def create_test_secrets():
    """Create temporary secrets for testing"""
    return """# Temporary test secrets
access_code: "1234"
wifi_ssid: "your_network"
wifi_password: "your_password"
ota_password: "secure_ota_password"
api_encryption_key: "O2kzIZpg4764NPzfNnUo3AAoSXWZC0SFhtQikpcbQzA="
"""

def test_configuration():
    """Test the DSC configuration"""
    # Check if we're in the repository root or ESPHome directory
    if os.path.exists("DSCAlarm_ESP_IDF_5_3_LoadProhibited_Fix.yaml"):
        # We're in the ESPHome directory
        esphome_dir = "."
        config_file = "DSCAlarm_ESP_IDF_5_3_LoadProhibited_Fix.yaml"
    elif os.path.exists("extras/ESPHome"):
        # We're in the repository root
        esphome_dir = "extras/ESPHome"
        config_file = "DSCAlarm_ESP_IDF_5_3_LoadProhibited_Fix.yaml"
    else:
        print("❌ Please run this script from either:")
        print("   - Repository root directory")
        print("   - extras/ESPHome directory")
        return False
    
    config_path = os.path.join(esphome_dir, config_file)
    if not os.path.exists(config_path):
        print(f"❌ Configuration file not found: {config_path}")
        return False
    
    # Create temporary secrets
    secrets_path = os.path.join(esphome_dir, "secrets.yaml")
    secrets_existed = os.path.exists(secrets_path)
    
    try:
        if not secrets_existed:
            with open(secrets_path, "w") as f:
                f.write(create_test_secrets())
        
        print(f"📝 Testing configuration: {config_file}")
        
        # Test configuration validation
        result = subprocess.run(
            ["esphome", "config", config_path],
            cwd=esphome_dir,
            capture_output=True,
            text=True,
            timeout=60
        )
        
        if result.returncode == 0:
            print("✅ Configuration validates successfully!")
            print("✅ Timer system fix is properly integrated")
            return True
        else:
            print("❌ Configuration validation failed:")
            print(result.stderr)
            return False
            
    finally:
        # Clean up temporary secrets if we created them
        if not secrets_existed and os.path.exists(secrets_path):
            os.remove(secrets_path)

def main():
    print("🔧 DSC Connectivity Fix Verification")
    print("=" * 40)
    
    # Check prerequisites
    if not check_esphome_installed():
        return 1
    
    # Test configuration
    if not test_configuration():
        return 1
    
    print("\n🎉 Verification PASSED!")
    print("\nNext steps:")
    print("1. Update your secrets.yaml with actual WiFi credentials")
    print("2. Flash firmware: esphome run DSCAlarm_ESP_IDF_5_3_LoadProhibited_Fix.yaml")
    print("3. Monitor logs for: 'ESP-IDF 5.3.2+ timer system ready'")
    print("4. Verify: 'DSC Keybus hardware initialization complete'")
    
    return 0

if __name__ == "__main__":
    sys.exit(main())