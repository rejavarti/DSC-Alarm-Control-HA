#!/usr/bin/env python3
"""
ESP32 Boot Failure Helper

Quick diagnostic and recovery assistant for ESP32 "invalid header" boot failures.
"""

import sys
import subprocess
from pathlib import Path

def print_header():
    print("=" * 60)
    print("ESP32 BOOT FAILURE HELPER")
    print("=" * 60)
    print()

def check_prerequisites():
    """Check if required tools are available"""
    print("🔍 Checking prerequisites...")
    
    # Check ESPHome
    try:
        result = subprocess.run(['esphome', 'version'], 
                              capture_output=True, text=True, check=True)
        version = result.stdout.strip()
        print(f"✅ ESPHome: {version}")
    except:
        print("❌ ESPHome not found - install with: pip install esphome")
        return False
    
    # Check esptool
    try:
        result = subprocess.run(['esptool.py', '--help'], 
                              capture_output=True, text=True, check=True)
        print("✅ esptool.py: Available")
    except:
        print("❌ esptool.py not found - usually installed with ESPHome")
        return False
    
    return True

def show_main_menu():
    """Show the main help menu"""
    print("\n📋 SELECT YOUR SITUATION:")
    print()
    print("1. My ESP32 shows 'invalid header: 0x74696e69' errors")
    print("2. I want to run full diagnostic and recovery")
    print("3. I just need a recovery test configuration")
    print("4. I want to understand what's wrong")
    print("5. Show all available tools")
    print("6. Exit")
    print()

def handle_choice_1():
    """Handle invalid header situation"""
    print("\n🔧 INVALID HEADER BOOT FAILURE")
    print()
    print("This is a flash corruption issue. Here's what to do:")
    print()
    print("QUICK RECOVERY (Recommended):")
    print("  python3 esphome_flash_recovery.py full-recover")
    print()
    print("MANUAL RECOVERY:")
    print("  1. python3 diagnose_boot_failure.py")
    print("  2. Follow the generated recovery instructions")
    print()
    print("STEP-BY-STEP:")
    print("  1. Erase flash: esptool.py --chip esp32 --port /dev/ttyUSB0 erase_flash")
    print("  2. Create test config: python3 esphome_flash_recovery.py create-config")
    print("  3. Upload test: esphome upload esp32_recovery_test.yaml")
    print()

def handle_choice_2():
    """Handle full diagnostic"""
    print("\n🔍 FULL DIAGNOSTIC AND RECOVERY")
    print()
    print("Running comprehensive diagnostic...")
    try:
        subprocess.run(['python3', 'diagnose_boot_failure.py'], check=True)
        print("\n✅ Diagnostic complete! Follow the instructions above.")
    except subprocess.CalledProcessError:
        print("❌ Diagnostic failed. Run manually: python3 diagnose_boot_failure.py")
    except FileNotFoundError:
        print("❌ Diagnostic script not found. Make sure you're in the right directory.")

def handle_choice_3():
    """Handle recovery config creation"""
    print("\n⚙️  CREATING RECOVERY TEST CONFIGURATION")
    print()
    try:
        subprocess.run(['python3', 'esphome_flash_recovery.py', 'create-config'], check=True)
        print("\n✅ Recovery configuration created!")
        print("Next steps:")
        print("  1. esphome compile esp32_recovery_test.yaml")
        print("  2. esphome upload esp32_recovery_test.yaml")
        print("  3. esphome logs esp32_recovery_test.yaml")
    except subprocess.CalledProcessError:
        print("❌ Failed to create config")
    except FileNotFoundError:
        print("❌ Recovery tool not found")

def handle_choice_4():
    """Handle explanation request"""
    print("\n📚 UNDERSTANDING ESP32 BOOT FAILURES")
    print()
    print("INVALID HEADER ERRORS:")
    print("• 0x74696e69 ('tini') = Flash corruption")
    print("• ESP32 can't find valid firmware header")
    print("• Usually caused by interrupted upload or power issues")
    print()
    print("LOADPROHIBITED ERRORS:")
    print("• 0xcececece patterns = Memory access violations")
    print("• Fixed by configurations in this repository")
    print("• Different from flash corruption issues")
    print()
    print("FLASH vs MEMORY ISSUES:")
    print("• Flash corruption: Can't boot at all, invalid headers")
    print("• Memory issues: Boots partway then crashes with stack traces")
    print()
    print("For detailed documentation, see:")
    print("• ESP32_INVALID_HEADER_BOOT_FAILURE_FIX.md")
    print("• ESPHOME_ESP32_LOADPROHIBITED_COMPLETE_FIX.md")

def handle_choice_5():
    """Show all available tools"""
    print("\n🛠️  AVAILABLE TOOLS")
    print()
    
    tools = [
        ("diagnose_boot_failure.py", "Comprehensive boot failure analysis"),
        ("esphome_flash_recovery.py", "ESPHome-specific flash recovery"),
        ("flash_recovery.sh", "Basic bash recovery script (auto-generated)"),
        ("esp32_boot_diagnostic.yaml", "Minimal diagnostic configuration"),
        ("esp32_recovery_test.yaml", "Recovery test configuration"),
    ]
    
    for tool, description in tools:
        if Path(tool).exists():
            print(f"✅ {tool:<30} {description}")
        else:
            print(f"⚪ {tool:<30} {description} (not generated yet)")
    
    print()
    print("DOCUMENTATION:")
    print(f"✅ ESP32_INVALID_HEADER_BOOT_FAILURE_FIX.md")
    print()
    print("To generate missing tools, choose option 2 (full diagnostic)")

def main():
    """Main helper program"""
    
    print_header()
    
    if not check_prerequisites():
        print("\n❌ Prerequisites not met. Please install ESPHome first.")
        return 1
    
    while True:
        show_main_menu()
        
        try:
            choice = input("Enter your choice (1-6): ").strip()
        except KeyboardInterrupt:
            print("\n\n👋 Goodbye!")
            return 0
        
        if choice == "1":
            handle_choice_1()
        elif choice == "2":
            handle_choice_2()
        elif choice == "3":
            handle_choice_3()
        elif choice == "4":
            handle_choice_4()
        elif choice == "5":
            handle_choice_5()
        elif choice == "6":
            print("\n👋 Goodbye!")
            return 0
        else:
            print("\n❌ Invalid choice. Please enter 1-6.")
        
        input("\nPress Enter to continue...")

if __name__ == "__main__":
    sys.exit(main())