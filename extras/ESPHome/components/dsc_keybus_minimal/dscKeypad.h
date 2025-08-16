#ifndef dscKeypad_h
#define dscKeypad_h

#include "dsc_common_constants.h"

class dscKeypadInterface {

  public:
    dscKeypadInterface(byte setClockPin, byte setReadPin, byte setWritePin = 255);

    // Interface control
    void begin(Stream &_stream = Serial);
    bool loop();
    void stop();
    void beep(byte beeps = 0);                                       // Keypad beep, 1-128 beeps
    void tone(byte beep = 0, bool tone = false, byte interval = 0);  // Keypad tone pattern, 1-7 beeps at 1-15s interval, with optional constant tone
    void buzzer(byte seconds = 0);                                   // Keypad buzzer, 1-255 seconds

    // Keypad key
    byte key, keyAvailable;

    // Keypad lights
    Light lightReady = on, lightArmed, lightMemory, lightBypass, lightTrouble, lightProgram, lightFire, lightBacklight = on;
    Light lightZone1, lightZone2, lightZone3, lightZone4, lightZone5, lightZone6, lightZone7, lightZone8;

    // Timer interrupt function to capture data
    static void dscDataInterrupt();
    static void dscClockInterrupt();

    static volatile byte moduleData[dscReadSize];
    static volatile bool bufferOverflow;

  private:
    void panelLight(Light lightPanel, byte zoneBit);
    void zoneLight(Light lightZone, byte zoneBit);

    Stream* stream;
    byte panelLights = 0x81, previousLights = 0x81;
    byte panelBlink, previousBlink;
    byte panelZones, previousZones;
    byte panelZonesBlink, previousZonesBlink;
    bool startupCycle = true;
    bool setBeep, setTone, setBuzzer;
    byte commandInterval = 5;   // Sets the milliseconds between panel commands
    unsigned long intervalStart;

    // Panel command arrays
    byte panelCommand05[5]  = {0x05, 0x81, 0x01, 0x00, 0x87};                                                // Partition 1: Ready Backlight - Partition ready | Partition 2: disabled | Zones 1-8 open: none
    byte panelCommand16[5]  = {0x16, 0x00, 0x00, 0x00, 0x16};                                                // Module tamper
    byte panelCommand27[7]  = {0x27, 0x81, 0x01, 0x10, 0xC7, 0x00, 0x80};                                // Partition 1: Ready Backlight - Partition ready | Partition 2: disabled | Zones 1-8 open: none
    byte panelCommand4C[12] = {0x4C, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};  // Module tamper query
    byte panelCommand5D[7]  = {0x5D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5D};                                // Partition 1 | Status lights flashing: none | Zones 1-32 flashing: none
    byte panelCommand64[3]  = {0x64, 0x0, 0x64};                                                         // Beep pattern, 1-128 beeps
    byte panelCommand75[3]  = {0x75, 0x0, 0x75};                                                         // Tone pattern, beeps at interval with optional constant tone
    byte panelCommand7F[3]  = {0x7F, 0x0, 0x7F};                                                         // Buzzer, 1-255 seconds
    byte panelCommandA5[8]  = {0xA5, 0x18, 0x0E, 0xED, 0x80, 0x00, 0x00, 0x38};                          // Date, time, system status messages - partitions 1-2
    byte panelCommandB1[10] = {0xB1, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xAD};              // Enabled zones 1-32
    byte panelCommandD5[9]  = {0xD5, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};                    // Keypad zone query
    
    static int clockInterval;
    static byte dscClockPin;
    static byte dscReadPin; 
    static byte dscWritePin;
    static volatile byte keyData;
    static volatile byte keyBufferLength;
    static volatile byte keyBuffer[dscBufferSize];
    static volatile bool commandReady, moduleDataDetected;
    static volatile bool alarmKeyDetected, alarmKeyResponsePending;
    static volatile byte clockCycleCount, clockCycleTotal;
    static volatile byte panelCommand[dscReadSize], panelCommandByteCount, panelCommandByteTotal;
    static volatile byte isrPanelBitTotal, isrPanelBitCount;
    static volatile byte isrModuleData[dscReadSize], isrModuleBitTotal, isrModuleBitCount, isrModuleByteCount;
    static volatile bool moduleDataCaptured;
    static volatile byte moduleByteCount;
    static volatile byte moduleBitCount;

#if defined(ESP32) || defined(ESP_PLATFORM)
    static hw_timer_t * timer1;
    static portMUX_TYPE timer1Mux;
    
    // Additional ESP32 safety variables to prevent LoadProhibited crashes
    static volatile bool esp32_hardware_initialized;
    static volatile bool esp32_timers_configured;
    static volatile unsigned long esp32_init_timestamp;
#endif
};

#endif // dscKeypad_h
