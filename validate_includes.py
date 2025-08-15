#!/usr/bin/env python3
"""
ESPHome Include Path Validator

This script validates that all ESPHome component include paths are correct
and prevents common typos like "components/d/alarm_control_panel.h" instead
of "components/alarm_control_panel/alarm_control_panel.h".
"""

import os
import re
import sys
from pathlib import Path

def find_source_files(directory):
    """Find all C++ source and header files."""
    source_files = []
    for ext in ['*.h', '*.hpp', '*.cpp', '*.cc']:
        source_files.extend(Path(directory).rglob(ext))
    return [f for f in source_files if not any(part.startswith('.') for part in f.parts)]

def validate_include_paths(file_path):
    """Validate include paths in a source file."""
    issues = []
    
    with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()
        lines = content.splitlines()
    
    for line_num, line in enumerate(lines, 1):
        # Check for #include statements
        include_match = re.match(r'\s*#include\s*[<"]([^>"]+)[>"]', line)
        if include_match:
            include_path = include_match.group(1)
            
            # Check for suspicious ESPHome component includes
            if 'esphome/components/' in include_path:
                # Look for common corruption patterns
                suspicious_patterns = [
                    r'components/d/',           # "alarm_control_panel" truncated to "d"
                    r'components/[a-z]/',       # Any single letter component
                    r'components/\w{1,2}/',     # Very short component names (likely typos)
                ]
                
                for pattern in suspicious_patterns:
                    if re.search(pattern, include_path):
                        # Exception for legitimate single-letter components if any exist
                        if not include_path.endswith('/d.h'):  # Allow hypothetical 'd.h' component
                            issues.append({
                                'file': file_path,
                                'line': line_num,
                                'issue': f'Suspicious include path: {include_path}',
                                'suggestion': 'Check if component name is complete'
                            })
                
                # Check for specific known issues
                if 'components/d/alarm_control_panel.h' in include_path:
                    issues.append({
                        'file': file_path,
                        'line': line_num,
                        'issue': f'Corrupted include path: {include_path}',
                        'suggestion': 'Should be: esphome/components/alarm_control_panel/alarm_control_panel.h'
                    })
    
    return issues

def main():
    """Main validation function."""
    if len(sys.argv) > 1:
        directory = sys.argv[1]
    else:
        directory = '.'
    
    print(f"Validating ESPHome include paths in: {directory}")
    
    source_files = find_source_files(directory)
    print(f"Found {len(source_files)} source files to check")
    
    all_issues = []
    
    for file_path in source_files:
        issues = validate_include_paths(file_path)
        all_issues.extend(issues)
    
    if all_issues:
        print(f"\n❌ Found {len(all_issues)} include path issues:")
        for issue in all_issues:
            print(f"\n  File: {issue['file']}")
            print(f"  Line {issue['line']}: {issue['issue']}")
            print(f"  Suggestion: {issue['suggestion']}")
        return 1
    else:
        print("\n✅ All include paths look correct!")
        return 0

if __name__ == '__main__':
    sys.exit(main())