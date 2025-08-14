#ifndef DSC_ARDUINO_COMPATIBILITY_H
#define DSC_ARDUINO_COMPATIBILITY_H

// Prevent duplicate static variable definitions in ESPHome builds
// Static variables are defined in dsc_static_variables.cpp
#define DSC_STATIC_VARIABLES_DEFINED

#include <cstdint>
#include <cstring>

#if defined(ARDUINO)
  // Pure Arduino framework
  #include <Arduino.h>
#else
  // ESPHome/ESP-IDF or other non-Arduino environments - provide Arduino compatibility
  #include <stdio.h>
  // Define the appropriate interface series based on configuration
  // Choose the appropriate DSC series based on configuration
  // If neither is explicitly defined, default to PowerSeries for better ESPHome compatibility
  #if defined(dscClassicSeries)
    // Classic series is defined, make sure PowerSeries is not
    #ifdef dscPowerSeries
      #undef dscPowerSeries
    #endif
    #define DSC_SERIES_DEFINED
  #elif !defined(dscPowerSeries) && !defined(DSC_SERIES_DEFINED)
    // Neither series is defined, default to PowerSeries
    #define dscPowerSeries
    #define DSC_SERIES_DEFINED
  #endif
  
  #if defined(ESP32) || defined(ESP_PLATFORM)
    #include <esp_attr.h>
    #include <esp_timer.h>
    #include <esp_ipc.h>
    #include <freertos/portmacro.h>
    
    // ESP-IDF 5.3.2 compatibility fix: Define missing portYIELD_CORE() macro
    // This macro is expected by FreeRTOS.h when configNUMBER_OF_CORES > 1
    // CRITICAL: Must be defined before ANY FreeRTOS headers are included
    #ifndef portYIELD_CORE
    #define portYIELD_CORE(xCoreID) \
      do { \
        if (xCoreID == xPortGetCoreID()) { \
          portYIELD(); \
        } else { \
          esp_ipc_call(xCoreID, vTaskYield, NULL); \
        } \
      } while(0)
    #endif
    
    // For ESPHome/ESP-IDF, define hw_timer_t which is Arduino ESP32 framework specific
    #ifndef ARDUINO
      typedef esp_timer_handle_t hw_timer_t;
    #endif
    // DRAM_ATTR, portMUX_TYPE, and related macros are already defined in ESP32 framework headers
    // Don't redefine them to avoid conflicts
  #else
    // Define DRAM_ATTR and ESP32 types for other platforms
    #ifndef DRAM_ATTR
    #define DRAM_ATTR
    #endif
    // Define portMUX_TYPE for non-ESP32 platforms (we're already in the #else block for non-ESP32)
    #ifndef DSC_PORTMUX_TYPE_DEFINED
    typedef int portMUX_TYPE;
    #define DSC_PORTMUX_TYPE_DEFINED
    #endif
    typedef void* hw_timer_t;
    #ifndef portMUX_INITIALIZER_UNLOCKED
    #define portMUX_INITIALIZER_UNLOCKED 0
    #endif
    #ifndef portENTER_CRITICAL
    #define portENTER_CRITICAL(x)
    #endif
    #ifndef portEXIT_CRITICAL
    #define portEXIT_CRITICAL(x)
    #endif
  #endif
  
  // Arduino compatibility for ESP-IDF
  #define F(str) (str)
  
  // Minimal Stream class for compatibility
  class Stream {
  public:
    virtual void print(const char* str) { printf("%s", str); }
    virtual void print(int value) { printf("%d", value); }
    virtual void print(int value, int base) { 
      if (base == 16) printf("%X", value);
      else if (base == 8) printf("%o", value);
      else printf("%d", value);
    }
    virtual void println(const char* str) { printf("%s\n", str); }
    
    // Add virtual methods that DSCStream tries to override
    virtual size_t write(uint8_t data) { return 1; }
    virtual int available() { return 0; }
    virtual int read() { return -1; }
    virtual int peek() { return -1; }
  };
  
  // Global Stream instance for DSC interface
  extern Stream dscStream;
  inline uint8_t bitRead(uint8_t value, uint8_t bit) { return (value >> bit) & 1; }
  template<typename T>
  inline uint8_t bitRead(T value, uint8_t bit) { return (value >> bit) & 1; }
  inline void bitWrite(uint8_t &value, uint8_t bit, uint8_t bitValue) { if (bitValue) value |= (1 << bit); else value &= ~(1 << bit); }
  template<typename T>
  inline void bitWrite(T &value, uint8_t bit, uint8_t bitValue) { if (bitValue) value |= (1 << bit); else value &= ~(1 << bit); }
  inline void pinMode(uint8_t pin, uint8_t mode) { /* ESPHome stub */ }
  inline void digitalWrite(uint8_t pin, uint8_t value) { /* ESPHome stub */ }
  inline uint8_t digitalRead(uint8_t pin) { return 0; /* ESPHome stub */ }
  inline void attachInterrupt(uint8_t interrupt, void (*callback)(), uint8_t mode) { /* ESPHome stub */ }
  inline void detachInterrupt(uint8_t interrupt) { /* ESPHome stub */ }
  inline uint8_t digitalPinToInterrupt(uint8_t pin) { return pin; /* ESPHome stub */ }
  inline void yield() { /* ESPHome stub - task yielding handled by framework */ }
  
  // ESP8266-specific compatibility
  #ifdef ESP8266
    inline void timer1_attachInterrupt(void (*callback)()) { /* ESPHome stub */ }
    inline void timer1_enable(uint8_t divider, uint8_t int_type, uint8_t reload) { /* ESPHome stub */ }
    inline void timer1_write(uint32_t ticks) { /* ESPHome stub */ }
    inline void timer1_disable() { /* ESPHome stub */ }
    // Timer constants
    #ifndef TIM_DIV16
    #define TIM_DIV16 1
    #define TIM_EDGE 0
    #define TIM_SINGLE 0
    #endif
  #endif
  #if defined(ESP32) || defined(ESP_PLATFORM)
    inline unsigned long millis() { return esp_timer_get_time() / 1000; }
    inline unsigned long micros() { return esp_timer_get_time(); }
  #else
    // ESP8266 or other platforms
    #include <sys/time.h>
    inline unsigned long millis() { 
      struct timeval tv;
      gettimeofday(&tv, NULL);
      return (tv.tv_sec * 1000UL) + (tv.tv_usec / 1000UL);
    }
    inline unsigned long micros() { 
      struct timeval tv;
      gettimeofday(&tv, NULL);
      return (tv.tv_sec * 1000000UL) + tv.tv_usec;
    }
  #endif
  inline void noInterrupts() { /* stub */ }
  inline void interrupts() { /* stub */ }
  
  // Arduino delay function compatibility
  #if defined(ESP32) || defined(ESP_PLATFORM)
    // For ESP32/ESP-IDF, use FreeRTOS delay
    #include <freertos/FreeRTOS.h>
    #include <freertos/task.h>
    #include <esp_rom_delay.h>
    inline void delay(unsigned long ms) { 
      vTaskDelay(pdMS_TO_TICKS(ms));
    }
    inline void delayMicroseconds(unsigned long us) {
      esp_rom_delay_us(us);
    }
  #else
    // For other platforms, use standard sleep
    #include <chrono>
    #include <thread>
    inline void delay(unsigned long ms) { 
      std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }
    inline void delayMicroseconds(unsigned long us) {
      std::this_thread::sleep_for(std::chrono::microseconds(us));
    }
  #endif
  
  #define INPUT 0
  #define OUTPUT 1
  #define LOW 0
  #define HIGH 1
  #define CHANGE 1
  #define HEX 16
  #define DEC 10
  
  // Global Serial object - only if not already defined by Arduino
  #ifndef ARDUINO
    static Stream _serial_instance;
    static Stream& Serial = _serial_instance;
  #endif
#endif

// ESPHome compatible type definitions
#ifndef byte
typedef uint8_t byte;
#endif

#endif // DSC_ARDUINO_COMPATIBILITY_H