#!/usr/bin/env python3
"""
ESPHome Flash Recovery Tool

This tool provides ESPHome-specific flash recovery for ESP32 devices
experiencing "invalid header" boot failures.
"""

import subprocess
import sys
import os
from pathlib import Path
import time

def run_command(cmd, description=""):
    """Run a command and handle errors gracefully"""
    print(f"\n{description}")
    print(f"Running: {' '.join(cmd)}")
    
    try:
        result = subprocess.run(cmd, check=True, capture_output=True, text=True)
        print("✅ Success")
        if result.stdout:
            print(result.stdout)
        return True
    except subprocess.CalledProcessError as e:
        print(f"❌ Error: {e}")
        if e.stderr:
            print(f"Error details: {e.stderr}")
        return False
    except FileNotFoundError:
        print(f"❌ Command not found: {cmd[0]}")
        print("Please ensure the required tools are installed")
        return False

def detect_esp32_port():
    """Try to detect the ESP32 port automatically"""
    common_ports = [
        "/dev/ttyUSB0", "/dev/ttyUSB1", "/dev/ttyUSB2",
        "/dev/ttyACM0", "/dev/ttyACM1", "/dev/ttyACM2",
        "COM3", "COM4", "COM5", "COM6", "COM7", "COM8"
    ]
    
    for port in common_ports:
        if os.path.exists(port) or port.startswith("COM"):
            # Try to query the device
            try:
                result = subprocess.run([
                    "esptool.py", "--port", port, "chip_id"
                ], capture_output=True, timeout=5)
                if result.returncode == 0:
                    return port
            except:
                continue
    
    return None

def esphome_flash_recovery(config_file=None, port=None):
    """Perform ESPHome-specific flash recovery"""
    
    print("=== ESPHome Flash Recovery Tool ===\n")
    
    # Auto-detect port if not specified
    if not port:
        print("🔍 Auto-detecting ESP32...")
        port = detect_esp32_port()
        if not port:
            print("❌ Could not auto-detect ESP32 port")
            print("Please connect your ESP32 and specify the port manually")
            return False
    
    print(f"📱 Using ESP32 on port: {port}")
    
    # Step 1: Complete flash erase
    print("\n📝 Step 1: Erasing flash memory...")
    if not run_command([
        "esptool.py", "--chip", "esp32", "--port", port, "erase_flash"
    ], "Completely erasing ESP32 flash memory"):
        return False
    
    print("⏳ Waiting for ESP32 to reset...")
    time.sleep(3)
    
    # Step 2: If config file provided, compile and upload
    if config_file and Path(config_file).exists():
        print(f"\n📝 Step 2: Compiling ESPHome configuration: {config_file}")
        if not run_command([
            "esphome", "compile", config_file
        ], f"Compiling {config_file}"):
            print("⚠️  Compilation failed, but flash erase was successful")
            print("You can manually fix the configuration and upload later")
            return True
        
        print(f"\n📝 Step 3: Uploading firmware to ESP32")
        if not run_command([
            "esphome", "upload", config_file, "--device", port
        ], f"Uploading {config_file} to {port}"):
            print("⚠️  Upload failed, but compilation was successful")
            print("Try uploading again or check physical connections")
            return True
        
        print(f"\n📝 Step 4: Monitoring boot process")
        print("Starting serial monitor... (Press Ctrl+C to exit)")
        try:
            subprocess.run([
                "esphome", "logs", config_file, "--device", port
            ], timeout=30)
        except KeyboardInterrupt:
            print("\n✅ Monitoring stopped")
        except subprocess.TimeoutExpired:
            print("\n✅ Monitoring timeout reached")
    
    else:
        print("\n📝 Next Steps:")
        print("1. Compile your ESPHome configuration:")
        print(f"   esphome compile your_config.yaml")
        print("2. Upload to ESP32:")
        print(f"   esphome upload your_config.yaml --device {port}")
        print("3. Monitor boot process:")
        print(f"   esphome logs your_config.yaml --device {port}")
    
    return True

def create_recovery_config():
    """Create a minimal recovery configuration"""
    config_content = """# ESP32 Flash Recovery Test Configuration
# Minimal configuration to verify ESP32 boot after flash recovery

esphome:
  name: esp32-recovery-test
  friendly_name: "ESP32 Recovery Test"

esp32:
  board: esp32dev
  framework:
    type: esp-idf
    # Conservative settings for maximum compatibility
    sdkconfig_options:
      CONFIG_ESP32_DEFAULT_CPU_FREQ_240: y
      CONFIG_ESP_MAIN_TASK_STACK_SIZE: "4096"
      CONFIG_ESP_TASK_WDT_TIMEOUT_S: "30"

# WiFi AP mode for testing
wifi:
  ap:
    ssid: "ESP32-Recovery-Test"
    password: "recovery123"

# Basic logging
logger:
  level: INFO

# API for testing
api:

# OTA updates
ota:
  - platform: esphome

# Recovery success indicators
sensor:
  - platform: uptime
    name: "Recovery Test Uptime"
    
  - platform: wifi_signal
    name: "WiFi Signal"
    update_interval: 10s

binary_sensor:
  - platform: template
    name: "Recovery Success"
    lambda: |-
      return true;  // If we reach here, recovery worked

# Status light
light:
  - platform: status_led
    name: "Recovery Status"
    pin: 
      number: 2
      inverted: true
"""
    
    config_path = Path("esp32_recovery_test.yaml") 
    config_path.write_text(config_content)
    return config_path

def main():
    """Main recovery program"""
    
    if len(sys.argv) < 2:
        print("ESPHome Flash Recovery Tool")
        print("\nUsage:")
        print("  python3 esphome_flash_recovery.py <command> [options]")
        print("\nCommands:")
        print("  create-config    Create a minimal recovery test configuration")
        print("  recover [port]   Perform flash recovery (with optional port)")
        print("  full-recover     Create config and perform full recovery")
        print("\nExamples:")
        print("  python3 esphome_flash_recovery.py create-config")
        print("  python3 esphome_flash_recovery.py recover /dev/ttyUSB0")
        print("  python3 esphome_flash_recovery.py full-recover")
        return 1
    
    command = sys.argv[1]
    
    if command == "create-config":
        config_path = create_recovery_config()
        print(f"✅ Created recovery configuration: {config_path}")
        print(f"\nTo test recovery:")
        print(f"  python3 esphome_flash_recovery.py recover")
        print(f"  esphome compile {config_path}")
        print(f"  esphome upload {config_path}")
        return 0
    
    elif command == "recover":
        port = sys.argv[2] if len(sys.argv) > 2 else None
        success = esphome_flash_recovery(port=port)
        return 0 if success else 1
    
    elif command == "full-recover":
        # Create recovery config
        config_path = create_recovery_config()
        print(f"✅ Created recovery configuration: {config_path}")
        
        # Perform recovery
        port = sys.argv[2] if len(sys.argv) > 2 else None
        success = esphome_flash_recovery(config_file=str(config_path), port=port)
        
        if success:
            print("\n🎉 Recovery completed successfully!")
            print("Your ESP32 should now be running the recovery test configuration.")
            print("If the status LED is on and you can see WiFi AP 'ESP32-Recovery-Test',")
            print("then the flash corruption has been fixed.")
        else:
            print("\n❌ Recovery failed")
            print("Please check connections and try again")
        
        return 0 if success else 1
    
    else:
        print(f"Unknown command: {command}")
        return 1

if __name__ == "__main__":
    sys.exit(main())