#!/usr/bin/env python3
"""
Python wrapper for clean_build.sh

This script provides helpful guidance when users try to run the bash script
with Python instead of bash.
"""

import os
import subprocess
import sys

def main():
    print("🚨 EXECUTION ERROR: clean_build.sh is a bash script, not a Python script!")
    print()
    print("❌ You ran: python3 clean_build.sh")
    print("✅ You should run: ./clean_build.sh")
    print("   or: bash clean_build.sh")
    print()
    
    # Check if the bash script exists and is executable
    script_path = os.path.join(os.path.dirname(__file__), "clean_build.sh")
    if os.path.exists(script_path):
        print("💡 Would you like to run the correct bash script now? (y/n)")
        try:
            response = input().strip().lower()
            if response in ['y', 'yes']:
                print()
                print("Running: bash clean_build.sh")
                print("=" * 40)
                # Execute the bash script
                result = subprocess.run(["bash", script_path], cwd=os.path.dirname(__file__))
                sys.exit(result.returncode)
            else:
                print()
                print("📖 To run the script manually:")
                print(f"   cd {os.path.dirname(script_path)}")
                print("   ./clean_build.sh")
        except KeyboardInterrupt:
            print("\n\nOperation cancelled.")
    else:
        print(f"❌ ERROR: clean_build.sh not found at {script_path}")
    
    print()
    print("📚 For more information, see the comments at the top of clean_build.sh")
    sys.exit(1)

if __name__ == "__main__":
    main()