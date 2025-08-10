/*
 * Core MQTT Command Processing Tests
 * Tests the essential command parsing and validation logic without Arduino dependencies
 */

#include <unity.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// Mock types to replace Arduino-specific types
typedef unsigned char byte;
typedef const char* String;

// Test fixture - core system variables
struct TestSystem {
    bool armed[8];
    bool armedAway[8];
    bool armedStay[8];
    bool noEntryDelay[8];
    bool exitDelay[8];
    bool alarm[8];
    bool ready[8];
    bool armedChanged[8];
    bool statusChanged;
    byte writePartition;
    char lastWrittenCommand[20];
    
    void reset() {
        for (int i = 0; i < 8; i++) {
            armed[i] = false;
            armedAway[i] = false;
            armedStay[i] = false;
            noEntryDelay[i] = false;
            exitDelay[i] = false;
            alarm[i] = false;
            ready[i] = true;
            armedChanged[i] = false;
        }
        statusChanged = false;
        writePartition = 1;
        strcpy(lastWrittenCommand, "");
    }
    
    void write(const char* command) {
        strcpy(lastWrittenCommand, command);
    }
    
    void write(char command) {
        lastWrittenCommand[0] = command;
        lastWrittenCommand[1] = '\0';
    }
};

// Global test system instance
TestSystem testSystem;

// System constants
const char* accessCode = "7730";

// Core MQTT callback function (extracted from main sketch)
void processMqttCommand(byte* payload, unsigned int length) {
    byte partition = 0;
    byte payloadIndex = 0;
    bool disarmWithAccessCode = false;
    char extractedAccessCode[10] = "";

    // Check if a partition number 1-8 has been sent
    if (payload[0] >= 0x31 && payload[0] <= 0x38) {
        partition = payload[0] - 49;
        payloadIndex = 1;
        
        // Check for "!XXXX" format indicating disarm with specific access code
        if (length > 2 && payload[1] == '!') {
            disarmWithAccessCode = true;
            byte codeLength = 0;
            for (byte i = 2; i < length && i < 11 && codeLength < 9; i++) {
                if (payload[i] >= '0' && payload[i] <= '9') {
                    extractedAccessCode[codeLength++] = payload[i];
                }
            }
            extractedAccessCode[codeLength] = '\0';
        }
    }

    // Panic alarm
    if (payload[payloadIndex] == 'P') {
        testSystem.write('p');
        return;
    }

    // Reset status if attempting to change armed mode while armed or not ready
    if (payload[payloadIndex] != 'D' && !testSystem.ready[partition]) {
        testSystem.armedChanged[partition] = true;
        testSystem.statusChanged = true;
        return;
    }

    // Arm stay
    if (payload[payloadIndex] == 'S' && !testSystem.armed[partition] && !testSystem.exitDelay[partition]) {
        testSystem.writePartition = partition + 1;
        testSystem.write('s');
    }
    // Arm away
    else if (payload[payloadIndex] == 'A' && !testSystem.armed[partition] && !testSystem.exitDelay[partition]) {
        testSystem.writePartition = partition + 1;
        testSystem.write('w');
    }
    // Arm night
    else if (payload[payloadIndex] == 'N' && !testSystem.armed[partition] && !testSystem.exitDelay[partition]) {
        testSystem.writePartition = partition + 1;
        testSystem.write('n');
    }
    // Disarm - either explicit 'D' or implied disarm with custom access code
    else if ((payload[payloadIndex] == 'D' || disarmWithAccessCode) && 
             (testSystem.armed[partition] || testSystem.exitDelay[partition] || testSystem.alarm[partition])) {
        testSystem.writePartition = partition + 1;
        if (disarmWithAccessCode) {
            testSystem.write(extractedAccessCode);
        } else {
            testSystem.write(accessCode);
        }
    }
}

// Helper function to simulate MQTT message
void simulateMqttMessage(const char* payload) {
    int len = strlen(payload);
    byte* payloadBytes = (byte*)malloc(len);
    memcpy(payloadBytes, payload, len);
    processMqttCommand(payloadBytes, len);
    free(payloadBytes);
}

// ==================== COMMAND PROCESSING TESTS ====================

void test_arm_stay_command() {
    testSystem.reset();
    
    simulateMqttMessage("1S");
    
    TEST_ASSERT_EQUAL_STRING("s", testSystem.lastWrittenCommand);
    TEST_ASSERT_EQUAL(1, testSystem.writePartition);
}

void test_arm_away_command() {
    testSystem.reset();
    
    simulateMqttMessage("1A");
    
    TEST_ASSERT_EQUAL_STRING("w", testSystem.lastWrittenCommand);
    TEST_ASSERT_EQUAL(1, testSystem.writePartition);
}

void test_arm_night_command() {
    testSystem.reset();
    
    simulateMqttMessage("1N");
    
    TEST_ASSERT_EQUAL_STRING("n", testSystem.lastWrittenCommand);
    TEST_ASSERT_EQUAL(1, testSystem.writePartition);
}

void test_disarm_command_default_code() {
    testSystem.reset();
    testSystem.armed[0] = true;
    
    simulateMqttMessage("1D");
    
    TEST_ASSERT_EQUAL_STRING("7730", testSystem.lastWrittenCommand);
    TEST_ASSERT_EQUAL(1, testSystem.writePartition);
}

void test_disarm_command_custom_code() {
    testSystem.reset();
    testSystem.armed[0] = true;
    
    simulateMqttMessage("1!9999");
    
    TEST_ASSERT_EQUAL_STRING("9999", testSystem.lastWrittenCommand);
    TEST_ASSERT_EQUAL(1, testSystem.writePartition);
}

void test_panic_alarm_command() {
    testSystem.reset();
    
    simulateMqttMessage("P");
    
    TEST_ASSERT_EQUAL_STRING("p", testSystem.lastWrittenCommand);
}

void test_multiple_partitions() {
    testSystem.reset();
    
    // Test Partition 2 arm away
    simulateMqttMessage("2A");
    TEST_ASSERT_EQUAL_STRING("w", testSystem.lastWrittenCommand);
    TEST_ASSERT_EQUAL(2, testSystem.writePartition);
    
    // Test Partition 8 arm stay
    simulateMqttMessage("8S");
    TEST_ASSERT_EQUAL_STRING("s", testSystem.lastWrittenCommand);
    TEST_ASSERT_EQUAL(8, testSystem.writePartition);
}

// ==================== EDGE CASE TESTS ====================

void test_arm_while_not_ready() {
    testSystem.reset();
    testSystem.ready[0] = false;
    
    simulateMqttMessage("1S");
    
    // Should trigger status change but not send arm command
    TEST_ASSERT_TRUE(testSystem.armedChanged[0]);
    TEST_ASSERT_TRUE(testSystem.statusChanged);
    TEST_ASSERT_EQUAL_STRING("", testSystem.lastWrittenCommand);
}

void test_arm_while_already_armed() {
    testSystem.reset();
    testSystem.armed[0] = true;
    
    simulateMqttMessage("1S");
    
    // Should not send arm command
    TEST_ASSERT_EQUAL_STRING("", testSystem.lastWrittenCommand);
}

void test_arm_while_in_exit_delay() {
    testSystem.reset();
    testSystem.exitDelay[0] = true;
    
    simulateMqttMessage("1A");
    
    // Should not send arm command
    TEST_ASSERT_EQUAL_STRING("", testSystem.lastWrittenCommand);
}

void test_disarm_while_disarmed() {
    testSystem.reset();
    // All states default to false/disarmed
    
    simulateMqttMessage("1D");
    
    // Should not send disarm command
    TEST_ASSERT_EQUAL_STRING("", testSystem.lastWrittenCommand);
}

void test_disarm_during_alarm() {
    testSystem.reset();
    testSystem.alarm[0] = true;
    
    simulateMqttMessage("1D");
    
    // Should send disarm command
    TEST_ASSERT_EQUAL_STRING("7730", testSystem.lastWrittenCommand);
    TEST_ASSERT_EQUAL(1, testSystem.writePartition);
}

void test_disarm_during_exit_delay() {
    testSystem.reset();
    testSystem.exitDelay[0] = true;
    
    simulateMqttMessage("1D");
    
    // Should send disarm command during exit delay
    TEST_ASSERT_EQUAL_STRING("7730", testSystem.lastWrittenCommand);
    TEST_ASSERT_EQUAL(1, testSystem.writePartition);
}

// ==================== MALFORMED COMMAND TESTS ====================

void test_invalid_partition_number() {
    testSystem.reset();
    
    simulateMqttMessage("9S");
    
    // Should not process command for invalid partition
    TEST_ASSERT_EQUAL_STRING("", testSystem.lastWrittenCommand);
}

void test_malformed_custom_access_code() {
    testSystem.reset();
    testSystem.armed[0] = true;
    
    simulateMqttMessage("1!A9B8C7");
    
    // Should extract only numeric digits: "987"
    TEST_ASSERT_EQUAL_STRING("987", testSystem.lastWrittenCommand);
}

void test_empty_custom_access_code() {
    testSystem.reset();
    testSystem.armed[0] = true;
    
    simulateMqttMessage("1!XYZ");
    
    // Should result in empty string (no valid digits)
    TEST_ASSERT_EQUAL_STRING("", testSystem.lastWrittenCommand);
}

void test_custom_code_boundary() {
    testSystem.reset();
    testSystem.armed[0] = true;
    
    simulateMqttMessage("1!123456789");
    
    // Should extract only first 9 digits max
    TEST_ASSERT_EQUAL_STRING("123456789", testSystem.lastWrittenCommand);
}

void test_no_partition_specified() {
    testSystem.reset();
    
    simulateMqttMessage("S");
    
    // Should use default partition (0) but partition index would be 0
    TEST_ASSERT_EQUAL_STRING("s", testSystem.lastWrittenCommand);
    TEST_ASSERT_EQUAL(1, testSystem.writePartition); // Default partition 1
}

void test_empty_command() {
    testSystem.reset();
    
    simulateMqttMessage("");
    
    // Should not crash or send any command
    TEST_ASSERT_EQUAL_STRING("", testSystem.lastWrittenCommand);
}

// ==================== HA CONFIGURATION VALIDATION TESTS ====================

void test_ha_yaml_payload_mapping() {
    testSystem.reset();
    
    // These test the exact payloads from HA_Yaml.YAML
    // payload_disarm: "1D"
    testSystem.armed[0] = true;
    simulateMqttMessage("1D");
    TEST_ASSERT_EQUAL_STRING("7730", testSystem.lastWrittenCommand);
    
    testSystem.reset();
    // payload_arm_home: "1S" 
    simulateMqttMessage("1S");
    TEST_ASSERT_EQUAL_STRING("s", testSystem.lastWrittenCommand);
    
    testSystem.reset();
    // payload_arm_away: "1A"
    simulateMqttMessage("1A");
    TEST_ASSERT_EQUAL_STRING("w", testSystem.lastWrittenCommand);
    
    testSystem.reset();
    // payload_arm_night: "1N"
    simulateMqttMessage("1N");
    TEST_ASSERT_EQUAL_STRING("n", testSystem.lastWrittenCommand);
}

void test_ha_command_template_custom_code() {
    testSystem.reset();
    testSystem.armed[0] = true;
    
    // Test command_template: 1!{{ code }} format
    simulateMqttMessage("1!1234");
    
    TEST_ASSERT_EQUAL_STRING("1234", testSystem.lastWrittenCommand);
    TEST_ASSERT_EQUAL(1, testSystem.writePartition);
}

// ==================== MAIN TEST RUNNER ====================

void setUp() {
    testSystem.reset();
}

void tearDown() {
    // Nothing to clean up
}

int main() {
    UNITY_BEGIN();
    
    // Core Command Processing Tests
    RUN_TEST(test_arm_stay_command);
    RUN_TEST(test_arm_away_command);
    RUN_TEST(test_arm_night_command);
    RUN_TEST(test_disarm_command_default_code);
    RUN_TEST(test_disarm_command_custom_code);
    RUN_TEST(test_panic_alarm_command);
    RUN_TEST(test_multiple_partitions);
    
    // Edge Case Tests
    RUN_TEST(test_arm_while_not_ready);
    RUN_TEST(test_arm_while_already_armed);
    RUN_TEST(test_arm_while_in_exit_delay);
    RUN_TEST(test_disarm_while_disarmed);
    RUN_TEST(test_disarm_during_alarm);
    RUN_TEST(test_disarm_during_exit_delay);
    
    // Malformed Command Tests
    RUN_TEST(test_invalid_partition_number);
    RUN_TEST(test_malformed_custom_access_code);
    RUN_TEST(test_empty_custom_access_code);
    RUN_TEST(test_custom_code_boundary);
    RUN_TEST(test_no_partition_specified);
    RUN_TEST(test_empty_command);
    
    // HA Configuration Validation Tests
    RUN_TEST(test_ha_yaml_payload_mapping);
    RUN_TEST(test_ha_command_template_custom_code);
    
    return UNITY_END();
}