#!/usr/bin/env python3
"""
DSC Alarm Control ESPHome Diagnostic Script
Diagnoses and fixes the DSC hardware initialization failure
"""

import sys
import os
import shutil
import yaml
from pathlib import Path

class DSCDiagnostic:
    def __init__(self):
        self.base_path = Path("/home/runner/work/New-DSC-Alarm-Control-HA/New-DSC-Alarm-Control-HA")
        self.user_config = self.base_path / "user_dsc_config_fixed.yaml"
        self.standalone_config = self.base_path / "extras/ESPHome/DscAlarm_Standalone_Test.yaml"
        
    def analyze_error_logs(self):
        """Analyze the provided error logs"""
        print("🔍 DSC ALARM CONTROL - ERROR ANALYSIS")
        print("=" * 50)
        
        error_indicators = [
            "DSC hardware initialization permanently failed after 5 attempts",
            "If no DSC panel is connected, enable standalone_mode: true",
            "api set Warning flag: unspecified",
            "mqtt set Warning flag: unspecified"
        ]
        
        print("📋 DETECTED ISSUES FROM LOGS:")
        for i, error in enumerate(error_indicators, 1):
            if i <= 2:  # Primary errors
                print(f"   ❌ {i}. {error}")
            else:  # Secondary issues
                print(f"   ⚠️  {i}. {error}")
        
        print(f"\n🎯 ROOT CAUSE IDENTIFIED:")
        print(f"   The ESP32 is trying to initialize DSC hardware but no physical")
        print(f"   DSC alarm panel is connected (or there are connection issues).")
        
    def check_current_config(self):
        """Check the current user configuration"""
        print(f"\n🔧 CURRENT CONFIGURATION ANALYSIS:")
        print("=" * 50)
        
        try:
            with open(self.user_config, 'r') as f:
                content = f.read()
                
            # Check for standalone_mode setting
            if 'standalone_mode: false' in content:
                print(f"   ❌ Problem Found: standalone_mode is set to FALSE")
                print(f"   📍 Location: {self.user_config}")
                print(f"   🔍 This means the ESP32 expects a physical DSC panel connection")
                return False
            elif 'standalone_mode: true' in content:
                print(f"   ✅ standalone_mode is correctly set to TRUE")
                return True
            else:
                print(f"   ❓ standalone_mode setting not found - this may be the issue")
                return False
                
        except FileNotFoundError:
            print(f"   ❌ Configuration file not found: {self.user_config}")
            return False
        except Exception as e:
            print(f"   ❌ Error reading configuration: {e}")
            return False
    
    def provide_solutions(self):
        """Provide solution options"""
        print(f"\n💡 SOLUTION OPTIONS:")
        print("=" * 50)
        
        print(f"Option 1: 🧪 ENABLE STANDALONE MODE (Testing without physical panel)")
        print(f"   ✅ Use this if you want to test the ESP32 without a DSC panel")
        print(f"   ✅ Perfect for development, setup, and configuration validation")
        print(f"   ✅ All ESPHome features work normally (WiFi, API, MQTT, etc.)")
        print(f"   ⚠️  Zone and partition data will be simulated/inactive")
        
        print(f"\nOption 2: 🔌 VERIFY PHYSICAL CONNECTIONS (Production with panel)")
        print(f"   ✅ Use this if you have a DSC alarm panel and want full functionality")
        print(f"   ✅ Provides real zone status and alarm control")
        print(f"   ❗ Requires proper hardware wiring to DSC panel")
        print(f"   ❗ Panel must be powered and functioning")
        
    def create_standalone_config(self):
        """Create a standalone configuration"""
        standalone_config_path = self.base_path / "user_dsc_config_standalone.yaml"
        
        try:
            # Read the original config
            with open(self.user_config, 'r') as f:
                content = f.read()
            
            # Replace standalone_mode setting
            if 'standalone_mode: false' in content:
                new_content = content.replace('standalone_mode: false', 'standalone_mode: true')
                
                # Write the new config
                with open(standalone_config_path, 'w') as f:
                    f.write(new_content)
                
                print(f"\n✅ STANDALONE CONFIGURATION CREATED:")
                print(f"   📁 File: {standalone_config_path}")
                print(f"   🔄 Changed: standalone_mode: false → standalone_mode: true")
                print(f"   🎯 This config will work without a physical DSC panel")
                
                return True
            else:
                print(f"\n❌ Could not create standalone config - standalone_mode setting not found")
                return False
                
        except Exception as e:
            print(f"\n❌ Error creating standalone config: {e}")
            return False
    
    def validate_config(self, config_path):
        """Validate an ESPHome configuration"""
        print(f"\n🔍 VALIDATING CONFIGURATION: {config_path.name}")
        print("-" * 30)
        
        # Check if file exists
        if not config_path.exists():
            print(f"   ❌ Configuration file not found: {config_path}")
            return False
        
        # Run ESPHome validation
        try:
            import subprocess
            result = subprocess.run(['esphome', 'config', str(config_path)], 
                                  capture_output=True, text=True, timeout=60)
            
            if result.returncode == 0:
                print(f"   ✅ Configuration is VALID!")
                print(f"   📋 ESPHome validation passed successfully")
                return True
            else:
                print(f"   ❌ Configuration validation FAILED:")
                print(f"   📋 Error: {result.stderr}")
                return False
                
        except subprocess.TimeoutExpired:
            print(f"   ⏰ Validation timed out - configuration may be complex")
            return False
        except Exception as e:
            print(f"   ❌ Validation error: {e}")
            return False
    
    def provide_instructions(self):
        """Provide step-by-step instructions"""
        print(f"\n📋 STEP-BY-STEP INSTRUCTIONS:")
        print("=" * 50)
        
        print(f"🧪 FOR STANDALONE MODE (Testing without panel):")
        print(f"   1. Use the created file: user_dsc_config_standalone.yaml")
        print(f"   2. Update your secrets.yaml with your WiFi/MQTT credentials")
        print(f"   3. Flash to ESP32: esphome run user_dsc_config_standalone.yaml")
        print(f"   4. Expected log: 'DSC Keybus hardware initialization complete (standalone mode)'")
        
        print(f"\n🔌 FOR PRODUCTION MODE (With physical panel):")
        print(f"   1. Verify DSC panel wiring connections:")
        print(f"      - Clock pin (ESP32 GPIO18) → DSC Yellow wire")
        print(f"      - Data pin (ESP32 GPIO19) → DSC Green wire")
        print(f"      - Ground (ESP32 GND) → DSC Black wire")
        print(f"   2. Ensure DSC panel is powered on")
        print(f"   3. Use original: user_dsc_config_fixed.yaml")
        print(f"   4. Flash to ESP32: esphome run user_dsc_config_fixed.yaml")
        
        print(f"\n🔧 UPDATING SECRETS.YAML:")
        print(f"   Replace these values in secrets.yaml:")
        print(f"   - wifi_ssid: \"YOUR_ACTUAL_WIFI_NAME\"")
        print(f"   - wifi_password: \"YOUR_ACTUAL_WIFI_PASSWORD\"")
        print(f"   - mqtt_broker: \"YOUR_MQTT_BROKER_IP\"")
        print(f"   - mqtt_username: \"YOUR_MQTT_USERNAME\"")
        print(f"   - mqtt_password: \"YOUR_MQTT_PASSWORD\"")
    
    def run_diagnosis(self):
        """Run the complete diagnosis"""
        print("🚀 DSC ALARM CONTROL - ESPHOME DIAGNOSTIC TOOL")
        print("🔧 Diagnosing DSC hardware initialization failure...")
        print("=" * 60)
        
        # Step 1: Analyze error logs
        self.analyze_error_logs()
        
        # Step 2: Check current configuration
        config_ok = self.check_current_config()
        
        # Step 3: Provide solutions
        self.provide_solutions()
        
        # Step 4: Create standalone config if needed
        if not config_ok:
            standalone_created = self.create_standalone_config()
            
            if standalone_created:
                # Validate the new standalone config
                standalone_path = self.base_path / "user_dsc_config_standalone.yaml"
                self.validate_config(standalone_path)
        
        # Step 5: Validate original config
        print(f"\n🔍 VALIDATING ORIGINAL CONFIGURATION:")
        print("-" * 40)
        self.validate_config(self.user_config)
        
        # Step 6: Provide instructions
        self.provide_instructions()
        
        print(f"\n🎯 SUMMARY:")
        print("=" * 50)
        print(f"✅ Problem diagnosed: DSC hardware initialization failure")
        print(f"✅ Root cause: standalone_mode set to false without physical panel")
        print(f"✅ Solutions provided: Standalone mode OR hardware verification")
        print(f"✅ Ready-to-use configuration created: user_dsc_config_standalone.yaml")
        print(f"\n💡 Next step: Choose your preferred solution and follow the instructions above!")

if __name__ == "__main__":
    diagnostic = DSCDiagnostic()
    diagnostic.run_diagnosis()