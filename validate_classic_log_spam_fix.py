#!/usr/bin/env python3
"""
Python wrapper for validate_classic_log_spam_fix.sh

This script provides helpful guidance when users try to run the bash script
with Python instead of bash, with enhanced WSL/environment compatibility.
"""

import os
import subprocess
import sys
import shutil
import platform

def find_bash_executable():
    """Find the bash executable in various common locations"""
    # Try to find bash using shutil.which first (respects PATH)
    found_bash = shutil.which("bash")
    if found_bash:
        return found_bash
    
    # If not found in PATH, try common locations
    bash_locations = [
        "/bin/bash",
        "/usr/bin/bash", 
        "/usr/local/bin/bash"
    ]
    
    for bash_path in bash_locations:
        if os.path.isfile(bash_path) and os.access(bash_path, os.X_OK):
            return bash_path
    
    return None

def detect_environment():
    """Detect the current environment and potential issues"""
    env_info = {
        "platform": platform.system(),
        "is_wsl": False,
        "bash_available": False,
        "bash_path": None
    }
    
    # Detect WSL
    try:
        with open("/proc/version", "r") as f:
            version_info = f.read().lower()
            if "microsoft" in version_info or "wsl" in version_info:
                env_info["is_wsl"] = True
    except (FileNotFoundError, PermissionError):
        pass
    
    # Find bash executable
    bash_path = find_bash_executable()
    if bash_path:
        env_info["bash_available"] = True
        env_info["bash_path"] = bash_path
    
    return env_info

def run_bash_script_safely(script_path, env_info):
    """Run the bash script with enhanced error handling"""
    if not env_info["bash_available"]:
        print("❌ ERROR: bash is not available in your environment!")
        print()
        if env_info["is_wsl"]:
            print("🔧 WSL Environment Detected:")
            print("   Your WSL environment may not have bash properly installed.")
            print("   Try installing bash with:")
            print("   sudo apt update && sudo apt install bash")
            print()
        print("🛠️  Alternative solutions:")
        print("   1. Install bash in your environment")
        print("   2. Run the script manually using your shell")
        print("   3. Use Windows Command Prompt or PowerShell instead of WSL")
        return False
    
    bash_path = env_info["bash_path"]
    print(f"Using bash: {bash_path}")
    
    try:
        # Try to run the bash script
        result = subprocess.run(
            [bash_path, script_path], 
            cwd=os.path.dirname(script_path),
            check=False,
            capture_output=True,
            text=True,
            timeout=120  # 2 minute timeout
        )
        
        # Print output
        if result.stdout:
            print(result.stdout)
        if result.stderr and result.returncode != 0:
            print("Error output:")
            print(result.stderr)
        
        return result.returncode == 0
        
    except subprocess.TimeoutExpired:
        print("❌ ERROR: Script execution timed out after 2 minutes")
        return False
    except subprocess.CalledProcessError as e:
        print(f"❌ ERROR: Script execution failed with return code {e.returncode}")
        if e.stderr:
            print("Error details:")
            print(e.stderr)
        return False
    except FileNotFoundError:
        print(f"❌ ERROR: Could not execute bash at {bash_path}")
        if env_info["is_wsl"]:
            print("This may be a WSL configuration issue.")
        return False
    except Exception as e:
        print(f"❌ ERROR: Unexpected error during script execution: {e}")
        return False

def main():
    print("🚨 EXECUTION ERROR: validate_classic_log_spam_fix.sh is a bash script, not a Python script!")
    print()
    print("❌ You ran: python3 validate_classic_log_spam_fix.py")
    print("✅ You should run: ./validate_classic_log_spam_fix.sh")
    print("   or: bash validate_classic_log_spam_fix.sh")
    print()
    
    # Detect environment
    env_info = detect_environment()
    
    # Show environment information
    if env_info["is_wsl"]:
        print("🔍 WSL Environment Detected")
    
    if not env_info["bash_available"]:
        print("⚠️  WARNING: bash is not available in your environment!")
    else:
        print(f"✅ bash found at: {env_info['bash_path']}")
    
    print()
    
    # Check if the bash script exists and is executable
    script_path = os.path.join(os.path.dirname(__file__), "validate_classic_log_spam_fix.sh")
    if os.path.exists(script_path):
        print("💡 Would you like to run the correct bash script now? (y/n)")
        try:
            response = input().strip().lower()
            if response in ['y', 'yes']:
                print()
                print("Running: bash validate_classic_log_spam_fix.sh")
                print("=" * 50)
                
                success = run_bash_script_safely(script_path, env_info)
                sys.exit(0 if success else 1)
            else:
                print()
                print("📖 To run the script manually:")
                if env_info["bash_available"]:
                    print(f"   cd {os.path.dirname(script_path)}")
                    print("   ./validate_classic_log_spam_fix.sh")
                    print(f"   or: {env_info['bash_path']} validate_classic_log_spam_fix.sh")
                else:
                    print("   First install bash in your environment, then:")
                    print(f"   cd {os.path.dirname(script_path)}")
                    print("   ./validate_classic_log_spam_fix.sh")
        except KeyboardInterrupt:
            print("\n\nOperation cancelled.")
    else:
        print(f"❌ ERROR: validate_classic_log_spam_fix.sh not found at {script_path}")
    
    print()
    print("📚 For more information, see the README.md 'Script Execution Guide' section")
    
    if env_info["is_wsl"] and not env_info["bash_available"]:
        print()
        print("🔧 WSL Troubleshooting:")
        print("   If you're using WSL and bash is not available:")
        print("   1. sudo apt update && sudo apt install bash")
        print("   2. Or use Windows Command Prompt/PowerShell instead")
        print("   3. Or try: wsl --install to reinstall WSL")
    
    sys.exit(1)

if __name__ == "__main__":
    main()