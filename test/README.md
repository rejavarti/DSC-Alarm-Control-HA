# DSC Alarm System MQTT Testing

## Overview
This directory contains comprehensive tests for the DSC Alarm Control system's MQTT functionality, specifically focusing on arm/disarm state management and MQTT communication.

## Test Structure

### test_mqtt_arm_disarm.cpp
Comprehensive test suite that validates:

#### 1. MQTT Command Processing
- **Arm Stay Command (`1S`)**: Tests that Home Assistant command translates to DSC 's' keypad command
- **Arm Away Command (`1A`)**: Tests that Home Assistant command translates to DSC 'w' keypad command  
- **Arm Night Command (`1N`)**: Tests that Home Assistant command translates to DSC 'n' keypad command
- **Disarm Command (`1D`)**: Tests default access code usage for disarming
- **Custom Access Code Disarm (`1!XXXX`)**: Tests custom access code extraction and usage
- **Panic Alarm (`P`)**: Tests panic alarm trigger functionality
- **Multi-Partition Support**: Tests commands for partitions 1-8

#### 2. State Transitions and MQTT Publishing
- **Armed States**: Validates proper MQTT state publishing for `armed_home`, `armed_away`, `armed_night`
- **Disarmed State**: Tests `disarmed` state publishing
- **Exit Delay**: Tests `pending` state during exit delay
- **Alarm Triggered**: Tests `triggered` state when alarm is activated
- **Status Topic**: Tests `online`/`offline` status publishing

#### 3. Edge Cases and Error Handling
- **System Not Ready**: Tests rejection of arm commands when system not ready
- **Already Armed**: Tests prevention of duplicate arm commands
- **Exit Delay Active**: Tests prevention of multiple arm commands during exit delay
- **Disarm When Disarmed**: Tests graceful handling of unnecessary disarm commands
- **Alarm Disarm**: Tests disarming during active alarm
- **Exit Delay to Disarmed**: Tests specific bug fix for state synchronization

#### 4. Malformed Command Handling
- **Invalid Partition Numbers**: Tests rejection of commands for non-existent partitions
- **Malformed Access Codes**: Tests extraction of valid digits from invalid input
- **Empty Access Codes**: Tests handling of empty custom access codes

#### 5. MQTT Connection Status
- **Online Status**: Tests keybus connection status reporting
- **Offline Status**: Tests keybus disconnection status reporting

## Running Tests

### Using PlatformIO
```bash
# Run all tests
pio test

# Run specific test file
pio test -f test_mqtt_arm_disarm

# Run tests with verbose output
pio test -v
```

### Expected Test Coverage
- MQTT callback function behavior with all supported payloads
- State publishing logic for all partition states
- Command template processing matching HA_Yaml.YAML configuration
- Access code handling (both default and custom)
- Edge cases and error conditions
- State synchronization between DSC system and MQTT

## Mock Architecture
The test suite uses mock implementations of:
- **MockDSCInterface**: Simulates the DSC keybus interface without hardware
- **MockMQTTClient**: Simulates MQTT client for testing message publishing/receiving
- **DSCAlarmTestFixture**: Provides test setup and teardown with state tracking

## Integration with Home Assistant Configuration
These tests validate the exact command and state mappings defined in `HA_Yaml.YAML`:

- `payload_disarm: "1D"` → Tests `test_disarm_command_default_code()`
- `payload_arm_home: "1S"` → Tests `test_arm_stay_command()`  
- `payload_arm_away: "1A"` → Tests `test_arm_away_command()`
- `payload_arm_night: "1N"` → Tests `test_arm_night_command()`
- `command_template` with `!{{ code }}` → Tests `test_disarm_command_custom_code()`

## Validation Points
Each test validates:
1. **Input Processing**: Correct parsing of MQTT payload
2. **Command Translation**: Proper conversion to DSC keypad commands
3. **Partition Targeting**: Correct partition selection
4. **Access Code Handling**: Proper code extraction and usage
5. **State Publishing**: Accurate MQTT state topic updates
6. **Error Handling**: Graceful handling of invalid inputs
7. **Edge Cases**: Behavior under various system conditions

## Test Results Interpretation
- **PASS**: All functionality works as expected per HA configuration
- **FAIL**: Indicates potential issues with MQTT command processing or state management
- Failures should be investigated for possible security or functionality issues

## Maintenance
- Update tests when adding new partition support
- Modify tests when changing MQTT topic structure  
- Add tests for new arm/disarm modes or features
- Validate tests against actual hardware when possible