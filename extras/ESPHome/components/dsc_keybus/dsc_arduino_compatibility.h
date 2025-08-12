#ifndef DSC_ARDUINO_COMPATIBILITY_H
#define DSC_ARDUINO_COMPATIBILITY_H

#include <cstdint>
#include <cstring>

#if defined(ARDUINO)
  // Pure Arduino framework
  #include <Arduino.h>
#else
  // ESPHome/ESP-IDF or other non-Arduino environments - provide Arduino compatibility
  #include <stdio.h>
  #ifdef ESP32
    #include <esp_attr.h>
    #include <esp_timer.h>
    #include <freertos/portmacro.h>
    // DRAM_ATTR is already defined in esp_attr.h
    // portMUX_TYPE and hw_timer_t are defined in the ESP32 framework
  #else
    // Define DRAM_ATTR and ESP32 types for other platforms
    #define DRAM_ATTR
    typedef int portMUX_TYPE;
    typedef void* hw_timer_t;
    #define portMUX_INITIALIZER_UNLOCKED 0
    #define portENTER_CRITICAL(x)
    #define portEXIT_CRITICAL(x)
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
  };
  
  // Arduino compatibility functions and constants
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
  #ifdef ESP32
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
  
  #define INPUT 0
  #define OUTPUT 1
  #define LOW 0
  #define HIGH 1
  #define CHANGE 1
  #define HEX 16
  
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