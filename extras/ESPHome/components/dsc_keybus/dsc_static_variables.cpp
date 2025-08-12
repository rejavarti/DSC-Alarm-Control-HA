// Static member definitions for dscClassicInterface  
// These are defined here to avoid multiple definition errors
// The wrapper pattern ensures DSC headers are only included in controlled places

// Unique guard to prevent multiple static variable definitions
#define DSC_STATIC_VARIABLES_DEFINED

#include "esphome/core/defines.h"
#include "dsc_arduino_compatibility.h"

// ESP32 hardware timer includes for timer1 and timer1Mux
#if defined(ESP32)
#include "esp32-hal-timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#endif

#if defined(dscClassicSeries)

#include "dscClassic.h"

// Initialize static variables to prevent uninitialized access crashes (ESPHome LoadProhibited fix)
byte dscClassicInterface::dscClockPin = 255;
byte dscClassicInterface::dscReadPin = 255;
byte dscClassicInterface::dscPC16Pin = 255;
byte dscClassicInterface::dscWritePin = 255;
char dscClassicInterface::writeKey = 0;
byte dscClassicInterface::writePartition = 1;
byte dscClassicInterface::writeByte = 0;
byte dscClassicInterface::writeBit = 0;
bool dscClassicInterface::virtualKeypad = false;
bool dscClassicInterface::processModuleData = false;
byte dscClassicInterface::panelData[dscReadSize] = {0};
byte dscClassicInterface::pc16Data[dscReadSize] = {0};
byte dscClassicInterface::panelByteCount = 0;
byte dscClassicInterface::panelBitCount = 0;
volatile bool dscClassicInterface::writeKeyPending = false;
volatile bool dscClassicInterface::writeKeyWait = false;
volatile byte dscClassicInterface::moduleData[dscReadSize] = {0};
volatile bool dscClassicInterface::moduleDataCaptured = false;
volatile byte dscClassicInterface::moduleByteCount = 0;
volatile byte dscClassicInterface::moduleBitCount = 0;
volatile bool dscClassicInterface::writeAlarm = false;
volatile bool dscClassicInterface::starKeyDetected = false;
volatile bool dscClassicInterface::starKeyCheck = false;
volatile bool dscClassicInterface::starKeyWait = false;
volatile bool dscClassicInterface::bufferOverflow = false;
volatile byte dscClassicInterface::panelBufferLength = 0;
volatile byte dscClassicInterface::panelBuffer[dscBufferSize][dscReadSize] = {0};
volatile byte dscClassicInterface::pc16Buffer[dscBufferSize][dscReadSize] = {0};
volatile byte dscClassicInterface::panelBufferBitCount[dscBufferSize] = {0};
volatile byte dscClassicInterface::panelBufferByteCount[dscBufferSize] = {0};
volatile byte dscClassicInterface::isrPanelData[dscReadSize] = {0};
volatile byte dscClassicInterface::isrPC16Data[dscReadSize] = {0};
volatile byte dscClassicInterface::isrPanelByteCount = 0;
volatile byte dscClassicInterface::isrPanelBitCount = 0;
volatile byte dscClassicInterface::isrPanelBitTotal = 0;
volatile byte dscClassicInterface::isrModuleData[dscReadSize] = {0};
volatile byte dscClassicInterface::isrModuleByteCount = 0;
volatile byte dscClassicInterface::isrModuleBitCount = 0;
volatile byte dscClassicInterface::isrModuleBitTotal = 0;
volatile byte dscClassicInterface::moduleCmd = 0;
volatile bool dscClassicInterface::readyLight = false;
volatile bool dscClassicInterface::lightBlink = false;
volatile unsigned long dscClassicInterface::clockHighTime = 0;
volatile unsigned long dscClassicInterface::keybusTime = 0;
volatile unsigned long dscClassicInterface::writeCompleteTime = 0;

// ESP32-specific timer variables - must be initialized to prevent LoadProhibited crashes
#if defined(ESP32)
hw_timer_t * dscClassicInterface::timer1 = nullptr;
portMUX_TYPE dscClassicInterface::timer1Mux = portMUX_INITIALIZER_UNLOCKED;
#endif

#elif defined(dscKeypad)

#include "dsc_arduino_compatibility.h"
#include "dscKeypad.h"

// Initialize static variables to prevent uninitialized access crashes (ESPHome LoadProhibited fix)
byte dscKeypadInterface::dscClockPin = 255;
byte dscKeypadInterface::dscReadPin = 255;
byte dscKeypadInterface::dscWritePin = 255;
int  dscKeypadInterface::clockInterval = 1000;  // Default safe interval
volatile byte dscKeypadInterface::keyData = 0;
volatile byte dscKeypadInterface::keyBufferLength = 0;
volatile byte dscKeypadInterface::keyBuffer[dscBufferSize] = {0};
volatile bool dscKeypadInterface::bufferOverflow = false;
volatile bool dscKeypadInterface::commandReady = false;
volatile bool dscKeypadInterface::moduleDataDetected = false;
volatile bool dscKeypadInterface::alarmKeyDetected = false;
volatile bool dscKeypadInterface::alarmKeyResponsePending = false;
volatile byte dscKeypadInterface::clockCycleCount = 0;
volatile byte dscKeypadInterface::clockCycleTotal = 0;
volatile byte dscKeypadInterface::panelCommand[dscReadSize] = {0};
volatile byte dscKeypadInterface::isrPanelBitTotal = 0;
volatile byte dscKeypadInterface::isrPanelBitCount = 0;
volatile byte dscKeypadInterface::panelCommandByteCount = 0;
volatile byte dscKeypadInterface::isrModuleData[dscReadSize] = {0};
volatile byte dscKeypadInterface::isrModuleBitTotal = 0;
volatile byte dscKeypadInterface::isrModuleBitCount = 0;
volatile byte dscKeypadInterface::isrModuleByteCount = 0;
volatile byte dscKeypadInterface::panelCommandByteTotal = 0;
volatile byte dscKeypadInterface::moduleData[dscReadSize] = {0};

#elif defined(dscClassicKeypad)

#include "dsc_arduino_compatibility.h"
#include "dscClassicKeypad.h"

// Initialize static variables to prevent uninitialized access crashes (ESPHome LoadProhibited fix)
byte dscClassicKeypadInterface::dscClockPin = 255;
byte dscClassicKeypadInterface::dscReadPin = 255;
byte dscClassicKeypadInterface::dscWritePin = 255;
int  dscClassicKeypadInterface::clockInterval = 1000;  // Default safe interval
volatile byte dscClassicKeypadInterface::keyData = 0;
volatile byte dscClassicKeypadInterface::keyBufferLength = 0;
volatile byte dscClassicKeypadInterface::keyBuffer[dscBufferSize] = {0};
volatile bool dscClassicKeypadInterface::bufferOverflow = false;
volatile bool dscClassicKeypadInterface::commandReady = false;
volatile bool dscClassicKeypadInterface::moduleDataDetected = false;
volatile bool dscClassicKeypadInterface::alarmKeyDetected = false;
volatile bool dscClassicKeypadInterface::alarmKeyResponsePending = false;
volatile byte dscClassicKeypadInterface::clockCycleCount = 0;
volatile byte dscClassicKeypadInterface::clockCycleTotal = 0;
volatile byte dscClassicKeypadInterface::panelCommand[dscReadSize] = {0};
volatile byte dscClassicKeypadInterface::isrPanelBitTotal = 0;
volatile byte dscClassicKeypadInterface::isrPanelBitCount = 0;
volatile byte dscClassicKeypadInterface::panelCommandByteCount = 0;
volatile byte dscClassicKeypadInterface::isrModuleData[dscReadSize] = {0};
volatile byte dscClassicKeypadInterface::isrModuleBitTotal = 0;
volatile byte dscClassicKeypadInterface::isrModuleBitCount = 0;
volatile byte dscClassicKeypadInterface::isrModuleByteCount = 0;
volatile byte dscClassicKeypadInterface::panelCommandByteTotal = 0;
volatile byte dscClassicKeypadInterface::moduleData[dscReadSize] = {0};
volatile unsigned long dscClassicKeypadInterface::intervalStart = 0;
volatile unsigned long dscClassicKeypadInterface::beepInterval = 0;
volatile unsigned long dscClassicKeypadInterface::repeatInterval = 0;
volatile unsigned long dscClassicKeypadInterface::keyInterval = 0;
volatile unsigned long dscClassicKeypadInterface::alarmKeyTime = 0;
volatile unsigned long dscClassicKeypadInterface::alarmKeyInterval = 0;

#else // PowerSeries

#include "dsc_arduino_compatibility.h"
#include "dscKeybus.h"

// Initialize static variables to prevent uninitialized access crashes (ESPHome LoadProhibited fix)
byte dscKeybusInterface::dscClockPin = 255;
byte dscKeybusInterface::dscReadPin = 255;
byte dscKeybusInterface::dscWritePin = 255;
char dscKeybusInterface::writeKey = 0;
byte dscKeybusInterface::writePartition = 1;
byte dscKeybusInterface::writeByte = 0;
byte dscKeybusInterface::writeBit = 0;
bool dscKeybusInterface::virtualKeypad = false;
bool dscKeybusInterface::processModuleData = false;
byte dscKeybusInterface::panelData[dscReadSize] = {0};
byte dscKeybusInterface::panelByteCount = 0;
byte dscKeybusInterface::panelBitCount = 0;
volatile bool dscKeybusInterface::writeKeyPending = false;
volatile byte dscKeybusInterface::moduleData[dscReadSize] = {0};
volatile bool dscKeybusInterface::moduleDataCaptured = false;
volatile bool dscKeybusInterface::moduleDataDetected = false;
volatile byte dscKeybusInterface::moduleByteCount = 0;
volatile byte dscKeybusInterface::moduleBitCount = 0;
volatile bool dscKeybusInterface::writeAlarm = false;
volatile bool dscKeybusInterface::starKeyCheck = false;
volatile bool dscKeybusInterface::starKeyWait[dscPartitions] = {false};
volatile bool dscKeybusInterface::bufferOverflow = false;
volatile byte dscKeybusInterface::panelBufferLength = 0;
volatile byte dscKeybusInterface::panelBuffer[dscBufferSize][dscReadSize] = {0};
volatile byte dscKeybusInterface::panelBufferBitCount[dscBufferSize] = {0};
volatile byte dscKeybusInterface::panelBufferByteCount[dscBufferSize] = {0};
volatile byte dscKeybusInterface::isrPanelData[dscReadSize] = {0};
volatile byte dscKeybusInterface::isrPanelByteCount = 0;
volatile byte dscKeybusInterface::isrPanelBitCount = 0;
volatile byte dscKeybusInterface::isrPanelBitTotal = 0;
volatile byte dscKeybusInterface::isrModuleData[dscReadSize] = {0};
volatile byte dscKeybusInterface::currentCmd = 0;
volatile byte dscKeybusInterface::statusCmd = 0;
volatile byte dscKeybusInterface::moduleCmd = 0;
volatile byte dscKeybusInterface::moduleSubCmd = 0;
volatile unsigned long dscKeybusInterface::clockHighTime = 0;
volatile unsigned long dscKeybusInterface::keybusTime = 0;

// ESP32-specific timer variables - must be initialized to prevent LoadProhibited crashes
#if defined(ESP32)
hw_timer_t * dscKeybusInterface::timer1 = nullptr;
portMUX_TYPE dscKeybusInterface::timer1Mux = portMUX_INITIALIZER_UNLOCKED;
#endif

#endif