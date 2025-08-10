#!/usr/bin/env python3
"""
DSC Alarm System Configuration Validator
Validates the enhanced YAML configuration for common issues and best practices
"""

import yaml
import re
import sys

class SecretLoader(yaml.SafeLoader):
    """Custom YAML loader that handles Home Assistant !secret tags"""
    pass

def secret_constructor(loader, node):
    """Constructor for !secret tag - returns placeholder for validation"""
    return f"SECRET_{loader.construct_scalar(node)}"

SecretLoader.add_constructor('!secret', secret_constructor)

def validate_yaml_syntax(yaml_file):
    """Validate YAML syntax and structure"""
    try:
        with open(yaml_file, 'r') as f:
            config = yaml.load(f, Loader=SecretLoader)
        print(f"✅ YAML syntax is valid in {yaml_file}")
        return config
    except yaml.YAMLError as e:
        print(f"❌ YAML syntax error in {yaml_file}: {e}")
        return None
    except FileNotFoundError:
        print(f"❌ File not found: {yaml_file}")
        return None

def validate_mqtt_topics(config):
    """Validate MQTT topic structure and naming"""
    print("\n🔍 Validating MQTT Topics...")
    
    topic_pattern = re.compile(r'^[a-zA-Z0-9/_-]+$')
    issues = []
    
    # Check alarm control panel
    if 'alarm_control_panel' in config:
        for panel in config['alarm_control_panel']:
            for topic_key in ['state_topic', 'availability_topic', 'command_topic']:
                if topic_key in panel:
                    topic = panel[topic_key]
                    if not topic_pattern.match(topic):
                        issues.append(f"Invalid MQTT topic format: {topic}")
    
    # Check sensors
    if 'sensor' in config:
        for sensor in config['sensor']:
            for topic_key in ['state_topic', 'availability_topic']:
                if topic_key in sensor:
                    topic = sensor[topic_key]
                    if not topic_pattern.match(topic):
                        issues.append(f"Invalid MQTT topic format: {topic}")
    
    # Check binary sensors
    if 'binary_sensor' in config:
        for sensor in config['binary_sensor']:
            for topic_key in ['state_topic', 'availability_topic']:
                if topic_key in sensor:
                    topic = sensor[topic_key]
                    if not topic_pattern.match(topic):
                        issues.append(f"Invalid MQTT topic format: {topic}")
    
    if issues:
        for issue in issues:
            print(f"❌ {issue}")
        return False
    else:
        print("✅ All MQTT topics are properly formatted")
        return True

def validate_unique_ids(config):
    """Validate that all entity unique_ids are actually unique"""
    print("\n🔍 Validating Unique IDs...")
    
    unique_ids = []
    duplicates = []
    
    for entity_type in ['alarm_control_panel', 'sensor', 'binary_sensor', 'button']:
        if entity_type in config:
            for entity in config[entity_type]:
                if 'unique_id' in entity:
                    uid = entity['unique_id']
                    if uid in unique_ids:
                        duplicates.append(uid)
                    else:
                        unique_ids.append(uid)
    
    if duplicates:
        for dup in duplicates:
            print(f"❌ Duplicate unique_id: {dup}")
        return False
    else:
        print(f"✅ All {len(unique_ids)} unique_ids are properly unique")
        return True

def validate_device_consistency(config):
    """Validate that device information is consistent across entities"""
    print("\n🔍 Validating Device Consistency...")
    
    device_identifiers = set()
    inconsistent_devices = []
    
    for entity_type in ['alarm_control_panel', 'sensor', 'binary_sensor', 'button']:
        if entity_type in config:
            for entity in config[entity_type]:
                if 'device' in entity and 'identifiers' in entity['device']:
                    identifiers = tuple(entity['device']['identifiers'])
                    device_identifiers.add(identifiers)
    
    if len(device_identifiers) > 1:
        print(f"⚠️  Multiple device identifier sets found: {len(device_identifiers)}")
        for identifiers in device_identifiers:
            print(f"   - {identifiers}")
        print("   Consider using consistent device identifiers across all entities")
    else:
        print("✅ Device identifiers are consistent")
    
    return len(device_identifiers) <= 1

def validate_security_practices(config):
    """Check for security best practices"""
    print("\n🔍 Validating Security Practices...")
    
    security_issues = []
    
    # Check for hard-coded credentials
    yaml_str = yaml.dump(config)
    
    # Check for secret usage
    secret_references = re.findall(r'SECRET_\w+', yaml_str)
    if secret_references:
        print(f"✅ Found {len(secret_references)} secret references:")
        for secret in secret_references:
            print(f"   - {secret.replace('SECRET_', '!secret ')}")
    else:
        # Check for hard-coded values that should be secrets
        if 'password' in yaml_str.lower() and 'SECRET_' not in yaml_str:
            security_issues.append("Potential hard-coded password detected")
    
    # Check alarm control panel security
    if 'alarm_control_panel' in config:
        for panel in config['alarm_control_panel']:
            if 'code_disarm_required' in panel and not panel['code_disarm_required']:
                security_issues.append("Disarm code not required - security risk")
            
            if 'code' in panel:
                code_value = str(panel['code'])
                if not code_value.startswith('SECRET_') and code_value.upper() != 'NONE':
                    security_issues.append("Hard-coded alarm code detected - use !secret instead")
    
    if security_issues:
        for issue in security_issues:
            print(f"⚠️  {issue}")
        return False
    else:
        print("✅ Security practices look good")
        return True

def validate_diagnostic_coverage(config):
    """Check that diagnostic sensors are properly configured"""
    print("\n🔍 Validating Diagnostic Coverage...")
    
    expected_diagnostics = [
        'dsc_system_uptime',
        'dsc_free_memory', 
        'dsc_wifi_rssi',
        'dsc_system_health',
        'dsc_keybus_connected',
        'dsc_buffer_overflow'
    ]
    
    found_diagnostics = []
    
    # Check sensors
    if 'sensor' in config:
        for sensor in config['sensor']:
            if 'unique_id' in sensor:
                uid = sensor['unique_id']
                if any(diag in uid for diag in expected_diagnostics):
                    found_diagnostics.append(uid)
    
    # Check binary sensors
    if 'binary_sensor' in config:
        for sensor in config['binary_sensor']:
            if 'unique_id' in sensor:
                uid = sensor['unique_id']
                if any(diag in uid for diag in expected_diagnostics):
                    found_diagnostics.append(uid)
    
    print(f"✅ Found {len(found_diagnostics)} diagnostic sensors:")
    for diag in found_diagnostics:
        print(f"   - {diag}")
    
    missing = [diag for diag in expected_diagnostics if not any(diag in found for found in found_diagnostics)]
    if missing:
        print(f"⚠️  Missing recommended diagnostic sensors:")
        for miss in missing:
            print(f"   - {miss}")
    
    return len(missing) == 0

def main():
    """Main validation function"""
    print("🔍 DSC Alarm System Configuration Validator")
    print("=" * 50)
    
    # Validate both YAML files
    original_config = validate_yaml_syntax('HA_Yaml.YAML')
    enhanced_config = validate_yaml_syntax('HA_Yaml_Enhanced.YAML')
    
    if not enhanced_config:
        print("\n❌ Cannot proceed with enhanced configuration validation")
        return 1
    
    # Run validation checks on enhanced configuration
    checks = [
        validate_mqtt_topics(enhanced_config),
        validate_unique_ids(enhanced_config), 
        validate_device_consistency(enhanced_config),
        validate_security_practices(enhanced_config),
        validate_diagnostic_coverage(enhanced_config)
    ]
    
    print("\n" + "=" * 50)
    print("📊 Validation Summary:")
    
    passed_checks = sum(checks)
    total_checks = len(checks)
    
    if passed_checks == total_checks:
        print(f"✅ All {total_checks} validation checks passed!")
        print("\n🎉 Configuration is ready for production use")
        return 0
    else:
        print(f"⚠️  {passed_checks}/{total_checks} validation checks passed")
        print(f"❌ {total_checks - passed_checks} issues need attention")
        return 1

if __name__ == '__main__':
    sys.exit(main())