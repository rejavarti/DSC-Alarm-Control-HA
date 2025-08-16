#!/usr/bin/env python3
"""
ESP32 Boot Failure Diagnostic Tool

This script analyzes the ESP32 boot failure with "invalid header: 0x74696e69"
and provides diagnostic steps and potential solutions.
"""

import sys
import os
import subprocess
import re
from pathlib import Path

def analyze_boot_failure():
    """Analyze the boot failure pattern and provide diagnosis"""
    
    print("=== ESP32 Boot Failure Diagnostic Tool ===\n")
    
    # Parse the error pattern
    error_hex = "0x74696e69"
    
    # Convert hex to ASCII
    try:
        hex_bytes = bytes.fromhex(error_hex[2:])  # Remove 0x prefix
        ascii_chars = hex_bytes.decode('ascii')
        print(f"Error hex value: {error_hex}")
        print(f"ASCII interpretation: '{ascii_chars}'")
        print()
    except:
        ascii_chars = "unknown"
    
    # Analyze the failure pattern
    print("=== Boot Failure Analysis ===")
    print("Reset cause: RTCWDT_RTC_RESET (RTC Watchdog Timer Reset)")
    print("Boot mode: SPI_FAST_FLASH_BOOT (Attempting to boot from SPI flash)")
    print(f"Invalid header: {error_hex} ('{ascii_chars}')")
    print()
    
    # Identify the issue type
    if ascii_chars == "tini":
        print("=== DIAGNOSIS: Flash Corruption ===")
        print("The 'tini' pattern suggests:")
        print("1. Flash memory corruption")
        print("2. Incorrect bootloader or partition table")
        print("3. Firmware upload interrupted or incomplete")
        print("4. Flash memory hardware issue")
        print()
        
        print("=== RECOMMENDED SOLUTIONS ===")
        solutions = [
            "1. Complete flash erase and reflash firmware",
            "2. Flash bootloader and partition table separately",
            "3. Use esptool.py to manually erase and program flash",
            "4. Check physical connections and power supply",
            "5. Try different ESP32 board if hardware issue suspected"
        ]
        
        for solution in solutions:
            print(solution)
        print()
        
        return "flash_corruption"
    else:
        print("=== DIAGNOSIS: Unknown Pattern ===")
        print("This is an unusual boot failure pattern.")
        print("Manual investigation required.")
        return "unknown"

def create_flash_recovery_script():
    """Create a script to recover from flash corruption"""
    
    recovery_script = """#!/bin/bash
# ESP32 Flash Recovery Script
# Use this script to recover from flash corruption causing "invalid header" errors

set -e

echo "=== ESP32 Flash Recovery Tool ==="
echo "This will completely erase and reflash your ESP32"
echo "Make sure your ESP32 is connected via USB"
echo ""

# Check if esptool is available
if ! command -v esptool.py &> /dev/null; then
    echo "Installing esptool..."
    pip install esptool
fi

# Detect ESP32 port (adjust as needed)
ESP_PORT="/dev/ttyUSB0"
if [ ! -e "$ESP_PORT" ]; then
    ESP_PORT="/dev/ttyACM0"
fi

if [ ! -e "$ESP_PORT" ]; then
    echo "Error: ESP32 not found on common ports"
    echo "Please specify the correct port manually"
    exit 1
fi

echo "Using ESP32 on port: $ESP_PORT"
echo ""

# Step 1: Complete flash erase
echo "Step 1: Erasing entire flash..."
esptool.py --chip esp32 --port $ESP_PORT erase_flash

# Step 2: Flash bootloader (if available)
if [ -f "bootloader.bin" ]; then
    echo "Step 2: Flashing bootloader..."
    esptool.py --chip esp32 --port $ESP_PORT --baud 460800 write_flash 0x1000 bootloader.bin
fi

# Step 3: Flash partition table (if available)  
if [ -f "partitions.bin" ]; then
    echo "Step 3: Flashing partition table..."
    esptool.py --chip esp32 --port $ESP_PORT --baud 460800 write_flash 0x8000 partitions.bin
fi

# Step 4: Instructions for firmware
echo ""
echo "=== Next Steps ==="
echo "1. Compile your ESPHome firmware:"
echo "   esphome compile your_config.yaml"
echo ""
echo "2. Flash the firmware:"
echo "   esphome upload your_config.yaml"
echo ""
echo "3. Monitor serial output:"
echo "   esphome logs your_config.yaml"

echo ""
echo "Flash recovery preparation complete!"
"""
    
    script_path = Path("flash_recovery.sh")
    script_path.write_text(recovery_script)
    script_path.chmod(0o755)
    
    print(f"Created flash recovery script: {script_path}")
    return script_path

def create_diagnostic_esphome_config():
    """Create a minimal ESPHome config for boot diagnosis"""
    
    config_content = """# ESP32 Boot Diagnostic Configuration
# Minimal configuration to test basic ESP32 boot functionality
# Use this to verify if ESP32 can boot after flash recovery

esphome:
  name: esp32-boot-diagnostic
  # Minimal build to reduce complexity
  platformio_options:
    build_flags:
      - -DDIAGNOSTIC_MODE
    lib_deps: []

esp32:
  board: esp32dev
  framework:
    type: esp-idf
    # Conservative ESP-IDF settings for maximum compatibility
    sdkconfig_options:
      # Minimal memory requirements
      CONFIG_ESP_MAIN_TASK_STACK_SIZE: "3584"
      CONFIG_FREERTOS_TIMER_TASK_STACK_DEPTH: "2048"
      
      # Stable CPU frequency
      CONFIG_ESP32_DEFAULT_CPU_FREQ_240: y
      CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ: "240"
      
      # Disable features that might cause issues
      CONFIG_PM_ENABLE: n
      CONFIG_ESP32_WIFI_DYNAMIC_TX_BUFFER: n
      
      # Conservative watchdog settings
      CONFIG_ESP_TASK_WDT_TIMEOUT_S: "30"
      CONFIG_ESP_INT_WDT_TIMEOUT_MS: "3000"

# Minimal WiFi for testing
wifi:
  ap:
    ssid: "ESP32-Diagnostic"
    password: "diagnostic123"

# Basic logging to verify boot
logger:
  level: INFO
  baud_rate: 115200

# Simple API without encryption
api:

# OTA for easy updates
ota:
  - platform: esphome

# Boot diagnostic sensors
sensor:
  - platform: uptime
    name: "Boot Diagnostic Uptime"
    id: diagnostic_uptime
    
  - platform: template
    name: "Free Heap"
    unit_of_measurement: "bytes"
    lambda: |-
      return esp_get_free_heap_size();

# Boot success indicator
binary_sensor:
  - platform: template
    name: "Boot Success"
    lambda: |-
      // If we reach here, boot was successful
      return true;

# Status LED to show boot success
light:
  - platform: monochromatic
    output: status_led_output
    name: "Boot Status LED"
    id: boot_status

output:
  - platform: ledc
    pin: 2  # Built-in LED on most ESP32 boards
    id: status_led_output

# Turn on LED when boot completes
interval:
  - interval: 1s
    then:
      - light.turn_on: boot_status
      - logger.log: "ESP32 boot diagnostic: System running normally"
"""
    
    config_path = Path("esp32_boot_diagnostic.yaml")
    config_path.write_text(config_content)
    
    print(f"Created diagnostic ESPHome config: {config_path}")
    return config_path

def check_esphome_installation():
    """Check if ESPHome is properly installed and working"""
    
    print("=== Checking ESPHome Installation ===")
    
    try:
        result = subprocess.run(['esphome', 'version'], 
                              capture_output=True, text=True, check=True)
        version = result.stdout.strip()
        print(f"ESPHome version: {version}")
        return True
    except subprocess.CalledProcessError:
        print("Error: ESPHome not found or not working properly")
        return False
    except FileNotFoundError:
        print("Error: ESPHome not installed")
        return False

def main():
    """Main diagnostic routine"""
    
    # Check ESPHome installation
    if not check_esphome_installation():
        print("Please install ESPHome first: pip install esphome")
        return 1
    
    # Analyze the boot failure
    failure_type = analyze_boot_failure()
    
    if failure_type == "flash_corruption":
        print("=== Creating Recovery Tools ===")
        
        # Create recovery script
        recovery_script = create_flash_recovery_script()
        
        # Create diagnostic config
        diagnostic_config = create_diagnostic_esphome_config()
        
        print(f"")
        print("=== RECOVERY PROCEDURE ===")
        print(f"1. Run the recovery script: ./{recovery_script}")
        print(f"2. Compile diagnostic config: esphome compile {diagnostic_config}")
        print(f"3. Upload diagnostic config: esphome upload {diagnostic_config}")
        print(f"4. Monitor boot process: esphome logs {diagnostic_config}")
        print("")
        print("If the diagnostic config boots successfully, the flash corruption is fixed.")
        print("Then you can proceed to upload your actual DSC alarm configuration.")
    
    return 0

if __name__ == "__main__":
    sys.exit(main())