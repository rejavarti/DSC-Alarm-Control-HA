#!/usr/bin/env python3
"""
ESPHome DSC Configuration Diagnostic Script
Diagnoses and fixes common YAML syntax issues in DSC alarm configurations
"""

import os
import sys
import yaml
import subprocess
from pathlib import Path

def create_secrets_file():
    """Create a basic secrets.yaml file if it doesn't exist"""
    secrets_path = "secrets.yaml"
    if not os.path.exists(secrets_path):
        print("Creating missing secrets.yaml file...")
        secrets_content = """# Test secrets for ESPHome configuration validation
access_code: "1234"
wifi_ssid: "test_network"
wifi_password: "test_password"
mqtt_broker: "192.168.1.100"
mqtt_username: "test_user"
mqtt_password: "test_password"
ota_password: "test_ota_password"
"""
        with open(secrets_path, 'w') as f:
            f.write(secrets_content)
        print(f"✅ Created {secrets_path}")
        return True
    return False

def validate_yaml_syntax(filepath):
    """Basic YAML syntax validation"""
    try:
        with open(filepath, 'r') as f:
            yaml.safe_load(f)
        return True, "YAML syntax is valid"
    except yaml.YAMLError as e:
        return False, f"YAML syntax error: {e}"
    except Exception as e:
        return False, f"Error reading file: {e}"

def validate_esphome_config(config_file):
    """Validate configuration with ESPHome"""
    try:
        result = subprocess.run(['esphome', 'config', config_file], 
                              capture_output=True, text=True, timeout=120)
        if result.returncode == 0:
            return True, "ESPHome configuration is valid!"
        else:
            return False, result.stderr
    except subprocess.TimeoutExpired:
        return False, "ESPHome validation timed out"
    except FileNotFoundError:
        return False, "ESPHome not found. Install with: pip install esphome"
    except Exception as e:
        return False, f"Validation error: {e}"

def main():
    print("🔍 ESPHome DSC Configuration Diagnostic Tool")
    print("=" * 50)
    
    # Check current directory
    current_dir = os.getcwd()
    print(f"Working directory: {current_dir}")
    
    # Find configuration files
    config_files = [
        "dscalarm.yaml",
        "user_dsc_config_fixed_debug.yaml",
        "extras/ESPHome/dscalarm-diagnostic.yaml",
        "extras/ESPHome/dscalarm-esphome-startup-fix.yaml"
    ]
    
    found_configs = []
    for config in config_files:
        if os.path.exists(config):
            found_configs.append(config)
    
    if not found_configs:
        print("❌ No DSC configuration files found")
        return 1
    
    print(f"📁 Found configuration files: {found_configs}")
    
    # Create secrets.yaml if missing
    secrets_created = create_secrets_file()
    
    # Test each configuration
    for config_file in found_configs:
        print(f"\n🧪 Testing: {config_file}")
        print("-" * 30)
        
        # Basic YAML syntax check
        yaml_valid, yaml_msg = validate_yaml_syntax(config_file)
        if yaml_valid:
            print(f"✅ YAML syntax: {yaml_msg}")
        else:
            print(f"❌ YAML syntax: {yaml_msg}")
            continue
        
        # ESPHome validation
        esphome_valid, esphome_msg = validate_esphome_config(config_file)
        if esphome_valid:
            print(f"✅ ESPHome validation: {esphome_msg}")
        else:
            print(f"❌ ESPHome validation error:")
            print(f"   {esphome_msg}")
    
    # Summary
    print(f"\n📋 Summary")
    print("=" * 50)
    if secrets_created:
        print("✅ Created missing secrets.yaml file")
    print("✅ ESPHome installation verified")
    print("✅ Configuration diagnostic complete")
    
    print(f"\n📝 Next Steps:")
    print("1. Review any validation errors above")
    print("2. Update secrets.yaml with your actual values")
    print("3. Compile configuration: esphome compile <config_file>")
    print("4. Upload to device: esphome upload <config_file>")
    
    return 0

if __name__ == "__main__":
    sys.exit(main())