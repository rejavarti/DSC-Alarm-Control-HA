/*
    DSC Keybus Interface

    https://github.com/taligentx/dscKeybusInterface

    This library is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef dscClassicKeypad_h
#define dscClassicKeypad_h

#include <cstdint>
#include <cstring>
#if defined(ESP_IDF_VERSION) || defined(USE_ARDUINO_VERSION_CODE) || defined(USE_ESP_IDF) || defined(USE_ARDUINO)
  // ESPHome or ESP-IDF framework includes
  #include <esp_attr.h>
  #include <esp_timer.h>
  #include <freertos/portmacro.h>
  #include <stdio.h>
  
  // Arduino compatibility for ESP-IDF/ESPHome
  #define F(str) (str)
  
  // Minimal Stream class for compatibility
  class Stream {
  public:
    virtual void print(const char* str) { printf("%s", str); }
    virtual void println(const char* str) { printf("%s\n", str); }
  };
  
  // Arduino compatibility functions and constants
  inline uint8_t bitRead(uint8_t value, uint8_t bit) { return (value >> bit) & 1; }
  inline void bitWrite(uint8_t &value, uint8_t bit, uint8_t bitValue) { if (bitValue) value |= (1 << bit); else value &= ~(1 << bit); }
  inline void pinMode(uint8_t pin, uint8_t mode) { /* stub */ }
  inline void digitalWrite(uint8_t pin, uint8_t value) { /* stub */ }
  inline uint8_t digitalRead(uint8_t pin) { return 0; /* stub */ }
  inline void attachInterrupt(uint8_t interrupt, void (*callback)(), uint8_t mode) { /* stub */ }
  inline void detachInterrupt(uint8_t interrupt) { /* stub */ }
  inline uint8_t digitalPinToInterrupt(uint8_t pin) { return pin; /* stub */ }
  inline unsigned long millis() { return esp_timer_get_time() / 1000; }
  inline void noInterrupts() { /* stub */ }
  inline void interrupts() { /* stub */ }
  
  #define INPUT 0
  #define OUTPUT 1
  #define LOW 0
  #define HIGH 1
  #define CHANGE 1
  
  // Global Serial object
  extern Stream Serial;
#else
  // Arduino framework include
  #include <Arduino.h>
#endif

// ESPHome compatible type definitions
#ifndef byte
typedef uint8_t byte;
#endif

// DSC Keybus constants - aligned with dscKeybus.h
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
#endif

enum Light {off, on, blink};    // Custom values for keypad lights status

class dscClassicKeypadInterface {

  public:
    dscClassicKeypadInterface(byte setClockPin, byte setReadPin, byte setWritePin);

    // Interface control
    void begin(Stream &_stream = Serial);                            // Initializes the stream output to Serial by default
    bool loop();                                                     // Returns true if valid panel data is available
    void beep(byte beeps = 0);                                       // Keypad beep, 1-128 beeps
    void tone(byte beep = 0, bool tone = false, byte interval = 0);  // Keypad tone pattern, 1-7 beeps at 1-15s interval, with optional constant tone
    void buzzer(byte seconds = 0);                                   // Keypad buzzer, 1-255 seconds

    // Keypad key
    byte key, keyAvailable;

    // Keypad lights
    Light lightReady = on, lightArmed, lightMemory, lightBypass, lightTrouble, lightProgram, lightFire, lightBacklight = on;
    Light lightZone1, lightZone2, lightZone3, lightZone4, lightZone5, lightZone6, lightZone7, lightZone8;

    // Panel Keybus commands
    byte classicCommand[2]  = {0x00, 0x80};

    /*
     * moduleData[] stores keypad data in an array: command [0], stop bit by itself [1], followed by the
     * remaining data.  These can be accessed directly in the sketch to get data that is not already tracked
     * in the library.  See dscKeybusPrintData.cpp for the currently known DSC commands and data.
     */
    static volatile byte moduleData[dscReadSize];

    // Key data buffer overflow, true if dscBufferSize needs to be increased
    static volatile bool bufferOverflow;

    // Timer interrupt function to capture data - declared as public for use by AVR Timer1
    static void dscClockInterrupt();

  private:

    void zoneLight(Light lightZone, byte zoneBit);
    void panelLight(Light lightPanel, byte zoneBit);

    Stream* stream;
    byte panelLights = 0x80, previousLights = 0x80;
    byte panelBlink, previousBlink;
    byte panelZones, previousZones;
    byte panelZonesBlink, previousZonesBlink;
    bool startupCycle = true;
    bool setBeep, setTone, setBuzzer;
    byte commandInterval = 26;   // Sets the milliseconds between panel commands
    bool keyBeep, beepStart;

    #if defined(ESP32)
    static hw_timer_t * timer1;
    static portMUX_TYPE timer1Mux;
    #endif

    static int clockInterval;
    static byte dscClockPin, dscReadPin, dscWritePin;
    static volatile byte keyData;
    static volatile byte keyBufferLength;
    static volatile byte keyBuffer[dscBufferSize];
    static volatile bool commandReady, moduleDataDetected;
    static volatile bool alarmKeyDetected, alarmKeyResponsePending;
    static volatile byte clockCycleCount, clockCycleTotal;
    static volatile byte panelCommand[dscReadSize], panelCommandByteCount, panelCommandByteTotal;
    static volatile byte isrPanelBitTotal, isrPanelBitCount;
    static volatile byte isrModuleData[dscReadSize], isrModuleBitTotal, isrModuleBitCount, isrModuleByteCount;
    static volatile unsigned long intervalStart, beepInterval, repeatInterval, keyInterval, alarmKeyTime, alarmKeyInterval;
};

#endif // dscKeypad_h
