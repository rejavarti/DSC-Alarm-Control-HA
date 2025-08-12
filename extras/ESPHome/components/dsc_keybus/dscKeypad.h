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

    // Keypad key
    byte key, keyAvailable;

    // Keypad lights
    Light lightReady = on, lightArmed, lightMemory, lightBypass, lightTrouble, lightProgram, lightFire, lightBacklight = on;
    Light lightZone1, lightZone2, lightZone3, lightZone4, lightZone5, lightZone6, lightZone7, lightZone8;

    // Timer interrupt function to capture data
    static void dscDataInterrupt();

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

#endif // dscKeypad_h
