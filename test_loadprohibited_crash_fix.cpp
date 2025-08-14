/*
 * Test for ESP32 LoadProhibited Crash Fix
 * 
 * This test validates that static variables are properly initialized 
 * and can prevent the 0xcececece LoadProhibited crash pattern.
 */

#include <Arduino.h>
#include <unity.h>

// Test the DSC static variable initialization
void test_dsc_static_variables_initialized() {
    // Declare extern to access the static initialization flag
    extern volatile bool dsc_static_variables_initialized;
    
    // The static variables should be properly initialized by constructor functions
    TEST_ASSERT_TRUE_MESSAGE(dsc_static_variables_initialized, 
                           "DSC static variables should be initialized by constructor");
}

// Test that timer variables have safe default values (not 0xcececece pattern)
void test_timer_variables_safe_initialization() {
    #if defined(ESP32)
    
    // For dscClassicInterface
    #ifdef dscClassicSeries
    extern class dscClassicInterface {
    public:
        static hw_timer_t* timer1;
        static portMUX_TYPE timer1Mux;
    };
    
    // Timer should be nullptr (safe default), not 0xcececece
    TEST_ASSERT_NOT_EQUAL_MESSAGE((void*)0xcececece, dscClassicInterface::timer1,
                                "timer1 should not have 0xcececece pattern");
    TEST_ASSERT_NOT_EQUAL_MESSAGE((void*)0xa5a5a5a5, dscClassicInterface::timer1,
                                "timer1 should not have 0xa5a5a5a5 pattern");
    TEST_ASSERT_EQUAL_MESSAGE(nullptr, dscClassicInterface::timer1,
                            "timer1 should be safely initialized to nullptr");
    #endif
    
    #endif
}

// Test heap memory availability
void test_heap_memory_sufficient() {
    size_t freeHeap = ESP.getFreeHeap();
    TEST_ASSERT_GREATER_THAN_MESSAGE(20000, freeHeap,
                                   "Should have sufficient heap memory for DSC initialization");
}

// Test DSC constants are valid
void test_dsc_constants_valid() {
    // These constants should be non-zero for valid operation
    extern const byte dscReadSize;
    extern const byte dscBufferSize;
    extern const byte dscPartitions;
    
    TEST_ASSERT_GREATER_THAN_MESSAGE(0, dscReadSize, "dscReadSize should be greater than 0");
    TEST_ASSERT_GREATER_THAN_MESSAGE(0, dscBufferSize, "dscBufferSize should be greater than 0");
    TEST_ASSERT_GREATER_THAN_MESSAGE(0, dscPartitions, "dscPartitions should be greater than 0");
    TEST_ASSERT_LESS_OR_EQUAL_MESSAGE(8, dscPartitions, "dscPartitions should be <= 8");
}

void setup() {
    Serial.begin(115200);
    delay(2000); // Allow system to stabilize
    
    UNITY_BEGIN();
    
    RUN_TEST(test_dsc_static_variables_initialized);
    RUN_TEST(test_timer_variables_safe_initialization);
    RUN_TEST(test_heap_memory_sufficient);
    RUN_TEST(test_dsc_constants_valid);
    
    UNITY_END();
}

void loop() {
    // Nothing to do in loop for tests
    delay(1000);
}