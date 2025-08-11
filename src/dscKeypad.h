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

#ifndef dscKeypad_h
#define dscKeypad_h

#include <cstdint>
#if (defined(ESP_IDF_VERSION) || (defined(PLATFORMIO) && !defined(ESP32) && !defined(ESP8266) && !defined(__AVR__))) && !defined(ARDUINO)
  // ESP-IDF framework includes or native build (not Arduino platforms)
  #ifdef ESP_IDF_VERSION
    #include <esp_attr.h>
    #include <esp_timer.h>
    #include <freertos/portmacro.h>
  #endif
  #include <stdio.h>
  #include <cstring>
  #include <chrono>
  #include <thread>
  
  // Arduino compatibility
  #define F(str) (str)
  
  // Minimal Stream class for compatibility
  class Stream {
  public:
    virtual void print(const char* str) { printf("%s", str); }
    virtual void print(int value) { printf("%d", value); }
    virtual void print(int value, int format) { 
      if (format == 16) printf("%X", value); 
      else printf("%d", value); 
    }
    virtual void print(unsigned int value) { printf("%u", value); }
    virtual void print(unsigned int value, int format) { 
      if (format == 16) printf("%X", value); 
      else printf("%u", value); 
    }
    virtual void print(long value) { printf("%ld", value); }
    virtual void print(unsigned long value) { printf("%lu", value); }
    virtual void println(const char* str) { printf("%s\n", str); }
    virtual void println(int value) { printf("%d\n", value); }
  };
  
  // Arduino compatibility functions and constants
  inline uint8_t bitRead(uint8_t value, uint8_t bit) { return (value >> bit) & 1; }
  inline void bitWrite(uint8_t &value, uint8_t bit, uint8_t bitvalue) { 
    if (bitvalue) value |= (1 << bit); 
    else value &= ~(1 << bit); 
  }
  inline void pinMode(uint8_t pin, uint8_t mode) { /* stub */ }
  inline void digitalWrite(uint8_t pin, uint8_t value) { /* stub */ }
  inline int digitalRead(uint8_t pin) { return 0; /* stub */ }
  inline void attachInterrupt(int pin, void (*isr)(), int mode) { /* stub */ }
  inline void detachInterrupt(int pin) { /* stub */ }
  inline int digitalPinToInterrupt(int pin) { return pin; /* stub */ }
  inline void noInterrupts() { /* stub */ }
  inline void interrupts() { /* stub */ }
  inline void delay(unsigned long ms) { 
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
  }
  inline char* itoa(int value, char* str, int base) {
    sprintf(str, (base == 16) ? "%X" : "%d", value);
    return str;
  }
  
  // Time functions
  inline unsigned long millis() { 
    static auto start = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
  }
  inline unsigned long micros() { 
    static auto start = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(now - start).count();
  }
  
  // Constants
  #define INPUT 0
  #define OUTPUT 1
  #define HIGH 1
  #define LOW 0
  #define CHANGE 1
  #define RISING 2
  #define FALLING 3
  #define HEX 16
  #define DEC 10
  
  // Global Serial object
  extern Stream Serial;
#else
  // Arduino framework include
  #include <Arduino.h>
#endif

// Compatible type definitions
#ifndef byte
typedef uint8_t byte;
#endif

#if defined(__AVR__)
const byte dscBufferSize = 10;  // Number of keys to buffer if the sketch is busy
#elif defined(ESP8266) || defined (ESP32)
const byte dscBufferSize = 50;
#else
// Native/test environment default
const byte dscBufferSize = 50;
#endif
const byte dscReadSize = 16;    // Maximum bytes of a Keybus command

enum Light {off, on, blink};    // Custom values for keypad lights status

class dscKeypadInterface {

  public:
    dscKeypadInterface(byte setClockPin, byte setReadPin, byte setWritePin);

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
    byte panelCommand05[5]  = {0x05, 0x81, 0x01, 0x10, 0xC7};                                            // Partition 1: Ready Backlight - Partition ready | Partition 2: disabled
    byte panelCommand16[5]  = {0x16, 0x0E, 0x23, 0xF1, 0x38};                                            // Panel version: v2.3 | Zone wiring: NC | Code length: 4 digits | *8 programming: no
    byte panelCommand27[7]  = {0x27, 0x81, 0x01, 0x10, 0xC7, 0x00, 0x80};                                // Partition 1: Ready Backlight - Partition ready | Partition 2: disabled | Zones 1-8 open: none
    byte panelCommand4C[12] = {0x4C, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};  // Module tamper query
    byte panelCommand5D[7]  = {0x5D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5D};                                // Partition 1 | Status lights flashing: none | Zones 1-32 flashing: none
    byte panelCommand64[3]  = {0x64, 0x0, 0x64};                                                         // Beep pattern, 1-128 beeps
    byte panelCommand75[3]  = {0x75, 0x0, 0x75};                                                         // Tone pattern, beeps at interval with optional constant tone
    byte panelCommand7F[3]  = {0x7F, 0x0, 0x7F};                                                         // Buzzer, 1-255 seconds
    byte panelCommandA5[8]  = {0xA5, 0x18, 0x0E, 0xED, 0x80, 0x00, 0x00, 0x38};                          // Date, time, system status messages - partitions 1-2
    byte panelCommandB1[10] = {0xB1, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xAD};              // Enabled zones 1-32
    byte panelCommandD5[9]  = {0xD5, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};                    // Keypad zone query

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
    byte panelLights = 0x81, previousLights = 0x81;
    byte panelBlink, previousBlink;
    byte panelZones, previousZones;
    byte panelZonesBlink, previousZonesBlink;
    bool startupCycle = true;
    bool setBeep, setTone, setBuzzer;
    byte commandInterval = 5;   // Sets the milliseconds between panel commands
    unsigned long intervalStart;

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
};

#endif // dscKeypad_h
