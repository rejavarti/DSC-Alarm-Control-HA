#!/usr/bin/env python3
"""
DSC Classic Panel Connection Diagnostic and Fix Tool

This tool helps diagnose DSC Classic panel connection issues and provides
automated fixes by detecting timing problems that occur when migrating
from Arduino dscKeybusInterface to ESPHome.

Usage:
    python3 dsc_classic_panel_connection_diagnostic.py [config_file.yaml]
    
The tool will:
1. Analyze your ESPHome configuration for classic panel timing issues
2. Check for known symptoms of Arduino -> ESPHome migration problems
3. Generate a fixed configuration with classic timing compatibility
4. Provide specific troubleshooting steps for your panel type
"""

import os
import sys
import re
import yaml
import argparse
from pathlib import Path
from typing import Dict, List, Optional, Tuple

class DSCClassicPanelDiagnostic:
    """Diagnostic tool for DSC Classic panel connection issues."""
    
    def __init__(self):
        self.issues_found = []
        self.recommendations = []
        self.classic_panel_indicators = [
            'dscClassicSeries',
            'dscClassic',
            'classic_series: true',
            'PC16',
            'dscPC16Pin'
        ]
        
        # Known error patterns from Arduino -> ESPHome migration
        self.classic_timing_error_patterns = [
            r'Hardware initialization rate limiting exceeded maximum attempts',
            r'DSC hardware initialization permanently failed',
            r'This usually indicates no DSC panel is connected',
            r'Task watchdog got triggered.*IDLE0',
            r'LoadProhibited.*0xcececece',
            r'Could not initialize timer',
            r'Timed out waiting for DSC connection'
        ]
        
    def analyze_configuration(self, config_path: str) -> Dict:
        """Analyze ESPHome configuration for classic panel timing issues."""
        results = {
            'has_classic_indicators': False,
            'missing_classic_timing': False,
            'problematic_settings': [],
            'recommended_fixes': [],
            'config_valid': True,
            'arduino_migration_detected': False
        }
        
        try:
            with open(config_path, 'r') as f:
                config_content = f.read()
                
            # Check for classic panel indicators
            for indicator in self.classic_panel_indicators:
                if indicator in config_content:
                    results['has_classic_indicators'] = True
                    break
                    
            # Check if classic timing compatibility is enabled
            if 'DSC_CLASSIC_TIMING_COMPATIBILITY' not in config_content:
                results['missing_classic_timing'] = True
                results['recommended_fixes'].append('Enable DSC_CLASSIC_TIMING_COMPATIBILITY mode')
                
            # Check for problematic ESPHome-specific settings
            problematic_patterns = [
                (r'hardware_detection_delay:\s*([0-9]+)ms', 'hardware_detection_delay > 5000ms', lambda x: int(x) > 5000),
                (r'initialization_timeout:\s*([0-9]+)ms', 'initialization_timeout > 30000ms', lambda x: int(x) > 30000),
                (r'retry_delay:\s*([0-9]+)ms', 'retry_delay > 2000ms', lambda x: int(x) > 2000),
                (r'task_wdt_timeout:\s*([0-9]+)', 'task_wdt_timeout > 300s', lambda x: int(x) > 300)
            ]
            
            for pattern, description, check_func in problematic_patterns:
                matches = re.findall(pattern, config_content)
                for match in matches:
                    if check_func(match):
                        results['problematic_settings'].append(description)
                        
            # Detect potential Arduino migration (comments or references)
            arduino_indicators = [
                'arduino', 'Arduino', 'dscKeybusInterface', 'taligentx',
                'worked with arduino', 'previously worked', 'migration'
            ]
            
            for indicator in arduino_indicators:
                if indicator in config_content:
                    results['arduino_migration_detected'] = True
                    break
                    
        except Exception as e:
            results['config_valid'] = False
            results['error'] = str(e)
            
        return results
        
    def analyze_log_file(self, log_path: str) -> Dict:
        """Analyze ESPHome logs for classic panel timing error patterns."""
        results = {
            'timing_errors_found': [],
            'classic_panel_symptoms': [],
            'error_count': 0,
            'log_valid': True
        }
        
        try:
            with open(log_path, 'r') as f:
                log_content = f.read()
                
            # Check for timing error patterns
            for pattern in self.classic_timing_error_patterns:
                matches = re.findall(pattern, log_content, re.IGNORECASE)
                if matches:
                    results['timing_errors_found'].append(pattern)
                    results['error_count'] += len(matches)
                    
            # Look for specific classic panel symptoms
            classic_symptoms = [
                (r'rate limiting exceeded.*([0-9]+)', 'Rate limiting exceeded attempts'),
                (r'LoadProhibited.*0x([a-fA-F0-9]+)', 'Memory access violations'),
                (r'Task watchdog.*IDLE([0-9])', 'Task watchdog timeouts'),
                (r'Failed to initialize.*timer', 'Timer initialization failures')
            ]
            
            for pattern, description in classic_symptoms:
                matches = re.findall(pattern, log_content)
                if matches:
                    results['classic_panel_symptoms'].append(f"{description}: {len(matches)} occurrences")
                    
        except Exception as e:
            results['log_valid'] = False
            results['error'] = str(e)
            
        return results
        
    def generate_fix_configuration(self, original_config: str, output_path: str) -> bool:
        """Generate a fixed configuration with classic timing compatibility."""
        try:
            # Read the original configuration
            with open(original_config, 'r') as f:
                config_content = f.read()
                
            # Apply classic timing fixes
            fixed_config = self._apply_classic_timing_fixes(config_content)
            
            # Write the fixed configuration
            with open(output_path, 'w') as f:
                f.write(fixed_config)
                
            return True
            
        except Exception as e:
            print(f"Error generating fix configuration: {e}")
            return False
            
    def _apply_classic_timing_fixes(self, config_content: str) -> str:
        """Apply classic timing fixes to configuration content."""
        lines = config_content.split('\n')
        fixed_lines = []
        
        # Track if we've added the classic timing flag
        classic_timing_added = False
        
        for line in lines:
            # Add classic timing compatibility to build flags
            if 'build_flags:' in line and not classic_timing_added:
                fixed_lines.append(line)
                fixed_lines.append('      # CRITICAL: Enable classic timing compatibility mode')
                fixed_lines.append('      - "-DDSC_CLASSIC_TIMING_COMPATIBILITY"')
                classic_timing_added = True
            # Fix excessive timing values
            elif 'hardware_detection_delay:' in line:
                fixed_lines.append('  hardware_detection_delay: 5000ms      # Classic panel compatible timing')
            elif 'initialization_timeout:' in line:
                fixed_lines.append('  initialization_timeout: 30000ms       # Classic panel compatible timeout')
            elif 'retry_delay:' in line:
                fixed_lines.append('  retry_delay: 2000ms                    # Classic panel compatible retry timing')
            elif 'task_wdt_timeout:' in line:
                fixed_lines.append('  task_wdt_timeout: 300s                 # Extended timeout for classic initialization')
            else:
                fixed_lines.append(line)
                
        # If we didn't find build_flags section, add it to platformio_options
        if not classic_timing_added:
            # Find platformio_options section and add build_flags
            for i, line in enumerate(fixed_lines):
                if 'platformio_options:' in line:
                    fixed_lines.insert(i + 1, '    build_flags:')
                    fixed_lines.insert(i + 2, '      # CRITICAL: Enable classic timing compatibility mode') 
                    fixed_lines.insert(i + 3, '      - "-DDSC_CLASSIC_TIMING_COMPATIBILITY"')
                    break
                    
        return '\n'.join(fixed_lines)
        
    def print_diagnostic_summary(self, config_analysis: Dict, log_analysis: Optional[Dict] = None):
        """Print a comprehensive diagnostic summary."""
        print("=" * 80)
        print("DSC CLASSIC PANEL CONNECTION DIAGNOSTIC SUMMARY")
        print("=" * 80)
        
        # Configuration Analysis
        print("\n📋 CONFIGURATION ANALYSIS:")
        if config_analysis['has_classic_indicators']:
            print("✅ DSC Classic panel indicators detected")
        else:
            print("❌ No classic panel indicators found")
            
        if config_analysis['missing_classic_timing']:
            print("🔧 ISSUE: DSC_CLASSIC_TIMING_COMPATIBILITY mode not enabled")
            print("   This is the primary cause of Arduino -> ESPHome migration failures")
        else:
            print("✅ Classic timing compatibility mode is enabled")
            
        if config_analysis['problematic_settings']:
            print("\n⚠️  PROBLEMATIC SETTINGS DETECTED:")
            for setting in config_analysis['problematic_settings']:
                print(f"   • {setting}")
                
        if config_analysis['arduino_migration_detected']:
            print("\n📱 Arduino migration detected - applying compatibility fixes")
            
        # Log Analysis (if provided)
        if log_analysis and log_analysis['log_valid']:
            print("\n📊 LOG ANALYSIS:")
            if log_analysis['timing_errors_found']:
                print(f"❌ {log_analysis['error_count']} timing-related errors found:")
                for error in log_analysis['timing_errors_found']:
                    print(f"   • {error}")
            else:
                print("✅ No timing errors detected in logs")
                
            if log_analysis['classic_panel_symptoms']:
                print("\n🔍 CLASSIC PANEL SYMPTOMS:")
                for symptom in log_analysis['classic_panel_symptoms']:
                    print(f"   • {symptom}")
                    
        # Recommendations
        print("\n💡 RECOMMENDATIONS:")
        
        if config_analysis['missing_classic_timing']:
            print("\n1. ENABLE CLASSIC TIMING COMPATIBILITY (CRITICAL):")
            print("   Add to your ESPHome configuration build_flags:")
            print("   platformio_options:")
            print("     build_flags:")
            print("       - \"-DDSC_CLASSIC_TIMING_COMPATIBILITY\"")
            print()
            print("   OR use the pre-configured fix file:")
            print("   esphome run user_dsc_config_classic_panel_fix.yaml")
            
        print("\n2. VERIFY CLASSIC PANEL TIMING VALUES:")
        print("   hardware_detection_delay: 5000ms")
        print("   initialization_timeout: 30000ms") 
        print("   retry_delay: 2000ms")
        
        print("\n3. USE CLASSIC PANEL SPECIFIC CONFIGURATION:")
        print("   classic_series: true")
        print("   standalone_mode: false  # Try hardware connection first")
        
        print("\n4. MONITOR CONNECTION STATUS:")
        print("   Watch for 'DSC Classic Panel Compatibility Mode' in logs")
        print("   Successful connection should show no rate limiting errors")
        
        print("\n📖 DETAILED TROUBLESHOOTING:")
        print("   For complete troubleshooting guide, see:")
        print("   • DSC_CLASSIC_HARDWARE_CONNECTION_FIX.md")
        print("   • user_dsc_config_classic_panel_fix.yaml")
        
        print("\n" + "=" * 80)
        
def main():
    parser = argparse.ArgumentParser(
        description="DSC Classic Panel Connection Diagnostic and Fix Tool",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python3 dsc_classic_panel_connection_diagnostic.py config.yaml
  python3 dsc_classic_panel_connection_diagnostic.py config.yaml --log esphome.log
  python3 dsc_classic_panel_connection_diagnostic.py config.yaml --generate-fix
        """
    )
    
    parser.add_argument('config', help='ESPHome configuration file to analyze')
    parser.add_argument('--log', help='ESPHome log file to analyze for errors')
    parser.add_argument('--generate-fix', action='store_true', 
                       help='Generate a fixed configuration file')
    parser.add_argument('--output', default='config_classic_panel_fixed.yaml',
                       help='Output file for fixed configuration')
    
    args = parser.parse_args()
    
    if not os.path.exists(args.config):
        print(f"Error: Configuration file '{args.config}' not found")
        sys.exit(1)
        
    diagnostic = DSCClassicPanelDiagnostic()
    
    # Analyze configuration
    print("Analyzing ESPHome configuration...")
    config_analysis = diagnostic.analyze_configuration(args.config)
    
    # Analyze log file if provided
    log_analysis = None
    if args.log:
        if os.path.exists(args.log):
            print("Analyzing log file...")
            log_analysis = diagnostic.analyze_log_file(args.log)
        else:
            print(f"Warning: Log file '{args.log}' not found, skipping log analysis")
    
    # Print diagnostic summary
    diagnostic.print_diagnostic_summary(config_analysis, log_analysis)
    
    # Generate fix if requested
    if args.generate_fix:
        print(f"\nGenerating fixed configuration: {args.output}")
        success = diagnostic.generate_fix_configuration(args.config, args.output)
        if success:
            print(f"✅ Fixed configuration generated: {args.output}")
            print("   Upload this configuration to resolve classic panel timing issues")
        else:
            print("❌ Failed to generate fixed configuration")
            
if __name__ == '__main__':
    main()