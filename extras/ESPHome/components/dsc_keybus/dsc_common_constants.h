#ifndef DSC_COMMON_CONSTANTS_H
#define DSC_COMMON_CONSTANTS_H

#include "dsc_arduino_compatibility.h"

// ESPHome compatible type definitions
#ifndef byte
typedef uint8_t byte;
#endif

// DSC Keybus constants - defined once to avoid redefinition
#ifndef DSC_CONSTANTS_DEFINED
#define DSC_CONSTANTS_DEFINED

#if defined(__AVR__)
const byte dscPartitions = 4;   // Maximum number of partitions - requires 19 bytes of memory per partition
const byte dscZones = 4;        // Maximum number of zone groups, 8 zones per group - requires 6 bytes of memory per zone group
const byte dscBufferSize = 10;  // Number of commands to buffer if the sketch is busy - requires dscReadSize + 2 bytes of memory per command
const byte dscReadSize = 16;    // Maximum bytes of a Keybus command
#elif defined(ESP8266)
const byte dscPartitions = 8;
const byte dscZones = 8;
const byte dscBufferSize = 50;
const byte dscReadSize = 16;
#elif defined(ESP32)
const byte dscPartitions = 8;
const byte dscZones = 8;
const DRAM_ATTR byte dscBufferSize = 50;
const DRAM_ATTR byte dscReadSize = 16;
#else
// Default fallback for ESPHome/ESP-IDF and other platforms
const byte dscPartitions = 8;
const byte dscZones = 8;
const byte dscBufferSize = 50;
const byte dscReadSize = 16;
#endif

// Exit delay target states
#define DSC_EXIT_STAY 1
#define DSC_EXIT_AWAY 2
#define DSC_EXIT_NO_ENTRY_DELAY 3

// Custom values for keypad lights status - defined once to avoid redefinition
#ifndef DSC_LIGHT_ENUM_DEFINED
#define DSC_LIGHT_ENUM_DEFINED
enum Light {off, on, blink};
#endif

#endif // DSC_CONSTANTS_DEFINED

#endif // DSC_COMMON_CONSTANTS_H