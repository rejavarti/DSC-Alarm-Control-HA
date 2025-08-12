#ifndef dscClassicKeypad_h
#define dscClassicKeypad_h

#include "dsc_common_constants.h"

class dscClassicKeypadInterface {

  public:
    dscClassicKeypadInterface(byte setClockPin, byte setReadPin, byte setWritePin);

    // Interface control
    void begin(Stream &_stream = Serial);
    bool loop();
    void stop();

    // Keypad key
    byte key, keyAvailable;

    // Keypad lights
    Light lightReady = on, lightArmed, lightMemory, lightBypass, lightTrouble, lightProgram, lightFire, lightBacklight = on;
    Light lightZone1, lightZone2, lightZone3, lightZone4, lightZone5, lightZone6, lightZone7, lightZone8;

    // Panel Keybus commands
    byte classicCommand[2]  = {0x00, 0x80};

    static volatile byte moduleData[dscReadSize];
    static volatile bool bufferOverflow;

  private:
    Stream* stream;

    static byte dscClockPin;
    static byte dscReadPin;
    static byte dscWritePin;
    static volatile bool moduleDataCaptured;
    static volatile byte moduleByteCount;
    static volatile byte moduleBitCount;

#if defined(ESP32)
    static hw_timer_t * timer1;
    static portMUX_TYPE timer1Mux;
#endif
};

#endif // dscClassicKeypad_h
