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

#ifndef dscKeybusInterface_h
#define dscKeybusInterface_h

#include "dsc_arduino_compatibility.h"

// DSC Classic Series
#if defined dscClassicSeries
#include "dscClassic.h"

// Static variables are defined in dsc_static_variables.cpp to avoid multiple definition errors
// Here we only provide extern declarations
extern byte dscClassicInterface::dscClockPin;
extern byte dscClassicInterface::dscReadPin;
extern byte dscClassicInterface::dscPC16Pin;
extern byte dscClassicInterface::dscWritePin;
extern char dscClassicInterface::writeKey;
extern byte dscClassicInterface::writePartition;
extern byte dscClassicInterface::writeByte;
extern byte dscClassicInterface::writeBit;
extern bool dscClassicInterface::virtualKeypad;
extern bool dscClassicInterface::processModuleData;
extern byte dscClassicInterface::panelData[dscReadSize];
extern byte dscClassicInterface::pc16Data[dscReadSize];
extern byte dscClassicInterface::panelByteCount;
extern byte dscClassicInterface::panelBitCount;
extern volatile bool dscClassicInterface::writeKeyPending;
extern volatile bool dscClassicInterface::writeKeyWait;
extern volatile byte dscClassicInterface::moduleData[dscReadSize];
extern volatile bool dscClassicInterface::moduleDataCaptured;
extern volatile byte dscClassicInterface::moduleByteCount;
extern volatile byte dscClassicInterface::moduleBitCount;
extern volatile bool dscClassicInterface::writeAlarm;
extern volatile bool dscClassicInterface::starKeyDetected;
extern volatile bool dscClassicInterface::starKeyCheck;
extern volatile bool dscClassicInterface::starKeyWait;
extern volatile bool dscClassicInterface::bufferOverflow;
extern volatile byte dscClassicInterface::panelBufferLength;
extern volatile byte dscClassicInterface::panelBuffer[dscBufferSize][dscReadSize];
extern volatile byte dscClassicInterface::pc16Buffer[dscBufferSize][dscReadSize];
extern volatile byte dscClassicInterface::panelBufferBitCount[dscBufferSize];
extern volatile byte dscClassicInterface::panelBufferByteCount[dscBufferSize];
extern volatile byte dscClassicInterface::isrPanelData[dscReadSize];
extern volatile byte dscClassicInterface::isrPC16Data[dscReadSize];
extern volatile byte dscClassicInterface::isrPanelByteCount;
extern volatile byte dscClassicInterface::isrPanelBitCount;
extern volatile byte dscClassicInterface::isrPanelBitTotal;
extern volatile byte dscClassicInterface::isrModuleData[dscReadSize];
extern volatile byte dscClassicInterface::isrModuleByteCount;
extern volatile byte dscClassicInterface::isrModuleBitCount;
extern volatile byte dscClassicInterface::isrModuleBitTotal;
extern volatile byte dscClassicInterface::moduleCmd;
extern volatile bool dscClassicInterface::readyLight;
extern volatile bool dscClassicInterface::lightBlink;
extern volatile unsigned long dscClassicInterface::clockHighTime;
extern volatile unsigned long dscClassicInterface::keybusTime;
extern volatile unsigned long dscClassicInterface::writeCompleteTime;

// Interrupt function called after 250us by dscClockInterrupt() using AVR Timer1, disables the timer and calls
// dscDataInterrupt() to read the data line
#if defined(__AVR__)
ISR(TIMER1_OVF_vect) {
  TCCR1B = 0;  // Disables Timer1
  dscClassicInterface::dscDataInterrupt();
}
#endif  // __AVR__


// DSC Keypad Interface
#elif defined dscKeypad
#include "dscKeypad.h"

byte dscKeypadInterface::dscClockPin;
byte dscKeypadInterface::dscReadPin;
byte dscKeypadInterface::dscWritePin;
int  dscKeypadInterface::clockInterval;
volatile byte dscKeypadInterface::keyData;
volatile byte dscKeypadInterface::keyBufferLength;
volatile byte dscKeypadInterface::keyBuffer[dscBufferSize];
volatile bool dscKeypadInterface::bufferOverflow;
volatile bool dscKeypadInterface::commandReady;
volatile bool dscKeypadInterface::moduleDataDetected;
volatile bool dscKeypadInterface::alarmKeyDetected;
volatile bool dscKeypadInterface::alarmKeyResponsePending;
volatile byte dscKeypadInterface::clockCycleCount;
volatile byte dscKeypadInterface::clockCycleTotal;
volatile byte dscKeypadInterface::panelCommand[dscReadSize];
volatile byte dscKeypadInterface::isrPanelBitTotal;
volatile byte dscKeypadInterface::isrPanelBitCount;
volatile byte dscKeypadInterface::panelCommandByteCount;
volatile byte dscKeypadInterface::isrModuleData[dscReadSize];
volatile byte dscKeypadInterface::isrModuleBitTotal;
volatile byte dscKeypadInterface::isrModuleBitCount;
volatile byte dscKeypadInterface::isrModuleByteCount;
volatile byte dscKeypadInterface::panelCommandByteTotal;
volatile byte dscKeypadInterface::moduleData[dscReadSize];

#if defined(__AVR__)
ISR(TIMER1_OVF_vect) {
  dscKeypadInterface::dscClockInterrupt();
}
#endif  // __AVR__

// DSC Classic Keypad Interface
#elif defined dscClassicKeypad
#include "dscClassicKeypad.h"

byte dscClassicKeypadInterface::dscClockPin;
byte dscClassicKeypadInterface::dscReadPin;
byte dscClassicKeypadInterface::dscWritePin;
int  dscClassicKeypadInterface::clockInterval;
volatile byte dscClassicKeypadInterface::keyData;
volatile byte dscClassicKeypadInterface::keyBufferLength;
volatile byte dscClassicKeypadInterface::keyBuffer[dscBufferSize];
volatile bool dscClassicKeypadInterface::bufferOverflow;
volatile bool dscClassicKeypadInterface::commandReady;
volatile bool dscClassicKeypadInterface::moduleDataDetected;
volatile bool dscClassicKeypadInterface::alarmKeyDetected;
volatile bool dscClassicKeypadInterface::alarmKeyResponsePending;
volatile byte dscClassicKeypadInterface::clockCycleCount;
volatile byte dscClassicKeypadInterface::clockCycleTotal;
volatile byte dscClassicKeypadInterface::panelCommand[dscReadSize];
volatile byte dscClassicKeypadInterface::isrPanelBitTotal;
volatile byte dscClassicKeypadInterface::isrPanelBitCount;
volatile byte dscClassicKeypadInterface::panelCommandByteCount;
volatile byte dscClassicKeypadInterface::isrModuleData[dscReadSize];
volatile byte dscClassicKeypadInterface::isrModuleBitTotal;
volatile byte dscClassicKeypadInterface::isrModuleBitCount;
volatile byte dscClassicKeypadInterface::isrModuleByteCount;
volatile byte dscClassicKeypadInterface::panelCommandByteTotal;
volatile byte dscClassicKeypadInterface::moduleData[dscReadSize];
volatile unsigned long dscClassicKeypadInterface::intervalStart;
volatile unsigned long dscClassicKeypadInterface::beepInterval;
volatile unsigned long dscClassicKeypadInterface::repeatInterval;
volatile unsigned long dscClassicKeypadInterface::keyInterval;
volatile unsigned long dscClassicKeypadInterface::alarmKeyTime;
volatile unsigned long dscClassicKeypadInterface::alarmKeyInterval;

#if defined(__AVR__)
ISR(TIMER1_OVF_vect) {
  dscClassicKeypadInterface::dscClockInterrupt();
}
#endif  // __AVR__


// DSC PowerSeries
#else
#include "dscKeybus.h"

byte dscKeybusInterface::dscClockPin;
byte dscKeybusInterface::dscReadPin;
byte dscKeybusInterface::dscWritePin;
char dscKeybusInterface::writeKey;
byte dscKeybusInterface::writePartition;
byte dscKeybusInterface::writeByte;
byte dscKeybusInterface::writeBit;
bool dscKeybusInterface::virtualKeypad;
bool dscKeybusInterface::processModuleData;
byte dscKeybusInterface::panelData[dscReadSize];
byte dscKeybusInterface::panelByteCount;
byte dscKeybusInterface::panelBitCount;
volatile bool dscKeybusInterface::writeKeyPending;
volatile byte dscKeybusInterface::moduleData[dscReadSize];
volatile bool dscKeybusInterface::moduleDataCaptured;
volatile bool dscKeybusInterface::moduleDataDetected;
volatile byte dscKeybusInterface::moduleByteCount;
volatile byte dscKeybusInterface::moduleBitCount;
volatile bool dscKeybusInterface::writeAlarm;
volatile bool dscKeybusInterface::starKeyCheck;
volatile bool dscKeybusInterface::starKeyWait[dscPartitions];
volatile bool dscKeybusInterface::bufferOverflow;
volatile byte dscKeybusInterface::panelBufferLength;
volatile byte dscKeybusInterface::panelBuffer[dscBufferSize][dscReadSize];
volatile byte dscKeybusInterface::panelBufferBitCount[dscBufferSize];
volatile byte dscKeybusInterface::panelBufferByteCount[dscBufferSize];
volatile byte dscKeybusInterface::isrPanelData[dscReadSize];
volatile byte dscKeybusInterface::isrPanelByteCount;
volatile byte dscKeybusInterface::isrPanelBitCount;
volatile byte dscKeybusInterface::isrPanelBitTotal;
volatile byte dscKeybusInterface::isrModuleData[dscReadSize];
volatile byte dscKeybusInterface::currentCmd;
volatile byte dscKeybusInterface::statusCmd;
volatile byte dscKeybusInterface::moduleCmd;
volatile byte dscKeybusInterface::moduleSubCmd;
volatile unsigned long dscKeybusInterface::clockHighTime;
volatile unsigned long dscKeybusInterface::keybusTime;

// Interrupt function called after 250us by dscClockInterrupt() using AVR Timer1, disables the timer and calls
// dscDataInterrupt() to read the data line
#if defined(__AVR__)
ISR(TIMER1_OVF_vect) {
  TCCR1B = 0;  // Disables Timer1
  dscKeybusInterface::dscDataInterrupt();
}
#endif  // __AVR__
#endif  // dscClassicSeries, dscKeypadInterface
#endif  // dscKeybusInterface_h
