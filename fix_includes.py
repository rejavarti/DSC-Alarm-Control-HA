#!/usr/bin/env python3
"""
Fix ESPHome Include Path Corruption

This script specifically fixes the "components/d/alarm_control_panel.h" 
corruption issue by searching for and correcting malformed include paths.
"""

import os
import re
import sys
from pathlib import Path

def fix_include_paths(file_path):
    """Fix corrupted include paths in a source file."""
    fixes_made = []
    
    with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()
        lines = content.splitlines()
    
    modified = False
    
    for line_num, line in enumerate(lines):
        original_line = line
        
        # Fix the specific "components/d/alarm_control_panel.h" issue
        if 'esphome/components/d/alarm_control_panel.h' in line:
            line = line.replace(
                'esphome/components/d/alarm_control_panel.h',
                'esphome/components/alarm_control_panel/alarm_control_panel.h'
            )
            modified = True
            fixes_made.append({
                'line': line_num + 1,
                'from': original_line.strip(),
                'to': line.strip()
            })
        
        lines[line_num] = line
    
    if modified:
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write('\n'.join(lines))
            if not lines[-1].endswith('\n'):
                f.write('\n')
    
    return fixes_made

def main():
    """Main fix function."""
    if len(sys.argv) > 1:
        directory = sys.argv[1]
    else:
        directory = '.'
    
    print(f"🔧 Fixing ESPHome include path corruption in: {directory}")
    
    # Find all C++ source and header files
    source_files = []
    for ext in ['*.h', '*.hpp', '*.cpp', '*.cc']:
        source_files.extend(Path(directory).rglob(ext))
    
    # Filter out build directories
    source_files = [f for f in source_files if not any(part.startswith('.') for part in f.parts)]
    
    print(f"Found {len(source_files)} source files to check")
    
    total_fixes = 0
    
    for file_path in source_files:
        fixes = fix_include_paths(file_path)
        if fixes:
            print(f"\n✅ Fixed {len(fixes)} issues in: {file_path}")
            for fix in fixes:
                print(f"  Line {fix['line']}:")
                print(f"    - {fix['from']}")
                print(f"    + {fix['to']}")
            total_fixes += len(fixes)
    
    if total_fixes == 0:
        print("\n✅ No include path corruption found - all files are already correct!")
    else:
        print(f"\n🎉 Successfully fixed {total_fixes} corrupted include paths!")
    
    return 0

if __name__ == '__main__':
    sys.exit(main())