// Static member definitions for dscClassicInterface  
// These are defined here to avoid multiple definition errors
// The wrapper pattern ensures DSC headers are only included in controlled places

// Unique guard to prevent multiple static variable definitions
#define DSC_STATIC_VARIABLES_DEFINED

#if defined(dscClassicSeries)

#include "dsc_arduino_compatibility.h"
#include "dscClassic.h"

// Static member definitions for dscClassicInterface
byte dscClassicInterface::dscClockPin;
byte dscClassicInterface::dscReadPin;
byte dscClassicInterface::dscPC16Pin;
byte dscClassicInterface::dscWritePin;
char dscClassicInterface::writeKey;
byte dscClassicInterface::writePartition;
byte dscClassicInterface::writeByte;
byte dscClassicInterface::writeBit;
bool dscClassicInterface::virtualKeypad;
bool dscClassicInterface::processModuleData;
byte dscClassicInterface::panelData[dscReadSize];
byte dscClassicInterface::pc16Data[dscReadSize];
byte dscClassicInterface::panelByteCount;
byte dscClassicInterface::panelBitCount;
volatile bool dscClassicInterface::writeKeyPending;
volatile bool dscClassicInterface::writeKeyWait;
volatile byte dscClassicInterface::moduleData[dscReadSize];
volatile bool dscClassicInterface::moduleDataCaptured;
volatile byte dscClassicInterface::moduleByteCount;
volatile byte dscClassicInterface::moduleBitCount;
volatile bool dscClassicInterface::writeAlarm;
volatile bool dscClassicInterface::starKeyDetected;
volatile bool dscClassicInterface::starKeyCheck;
volatile bool dscClassicInterface::starKeyWait;
volatile bool dscClassicInterface::bufferOverflow;
volatile byte dscClassicInterface::panelBufferLength;
volatile byte dscClassicInterface::panelBuffer[dscBufferSize][dscReadSize];
volatile byte dscClassicInterface::pc16Buffer[dscBufferSize][dscReadSize];
volatile byte dscClassicInterface::panelBufferBitCount[dscBufferSize];
volatile byte dscClassicInterface::panelBufferByteCount[dscBufferSize];
volatile byte dscClassicInterface::isrPanelData[dscReadSize];
volatile byte dscClassicInterface::isrPC16Data[dscReadSize];
volatile byte dscClassicInterface::isrPanelByteCount;
volatile byte dscClassicInterface::isrPanelBitCount;
volatile byte dscClassicInterface::isrPanelBitTotal;
volatile byte dscClassicInterface::isrModuleData[dscReadSize];
volatile byte dscClassicInterface::isrModuleByteCount;
volatile byte dscClassicInterface::isrModuleBitCount;
volatile byte dscClassicInterface::isrModuleBitTotal;
volatile byte dscClassicInterface::moduleCmd;
volatile bool dscClassicInterface::readyLight;
volatile bool dscClassicInterface::lightBlink;
volatile unsigned long dscClassicInterface::clockHighTime;
volatile unsigned long dscClassicInterface::keybusTime;
volatile unsigned long dscClassicInterface::writeCompleteTime;

#endif