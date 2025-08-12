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

extern byte dscKeypadInterface::dscClockPin;
extern byte dscKeypadInterface::dscReadPin;
extern byte dscKeypadInterface::dscWritePin;
extern int  dscKeypadInterface::clockInterval;
extern volatile byte dscKeypadInterface::keyData;
extern volatile byte dscKeypadInterface::keyBufferLength;
extern volatile byte dscKeypadInterface::keyBuffer[dscBufferSize];
extern volatile bool dscKeypadInterface::bufferOverflow;
extern volatile bool dscKeypadInterface::commandReady;
extern volatile bool dscKeypadInterface::moduleDataDetected;
extern volatile bool dscKeypadInterface::alarmKeyDetected;
extern volatile bool dscKeypadInterface::alarmKeyResponsePending;
extern volatile byte dscKeypadInterface::clockCycleCount;
extern volatile byte dscKeypadInterface::clockCycleTotal;
extern volatile byte dscKeypadInterface::panelCommand[dscReadSize];
extern volatile byte dscKeypadInterface::isrPanelBitTotal;
extern volatile byte dscKeypadInterface::isrPanelBitCount;
extern volatile byte dscKeypadInterface::panelCommandByteCount;
extern volatile byte dscKeypadInterface::isrModuleData[dscReadSize];
extern volatile byte dscKeypadInterface::isrModuleBitTotal;
extern volatile byte dscKeypadInterface::isrModuleBitCount;
extern volatile byte dscKeypadInterface::isrModuleByteCount;
extern volatile byte dscKeypadInterface::panelCommandByteTotal;
extern volatile byte dscKeypadInterface::moduleData[dscReadSize];

#if defined(__AVR__)
ISR(TIMER1_OVF_vect) {
  dscKeypadInterface::dscClockInterrupt();
}
#endif  // __AVR__

// DSC Classic Keypad Interface
#elif defined dscClassicKeypad
#include "dscClassicKeypad.h"

extern byte dscClassicKeypadInterface::dscClockPin;
extern byte dscClassicKeypadInterface::dscReadPin;
extern byte dscClassicKeypadInterface::dscWritePin;
extern int  dscClassicKeypadInterface::clockInterval;
extern volatile byte dscClassicKeypadInterface::keyData;
extern volatile byte dscClassicKeypadInterface::keyBufferLength;
extern volatile byte dscClassicKeypadInterface::keyBuffer[dscBufferSize];
extern volatile bool dscClassicKeypadInterface::bufferOverflow;
extern volatile bool dscClassicKeypadInterface::commandReady;
extern volatile bool dscClassicKeypadInterface::moduleDataDetected;
extern volatile bool dscClassicKeypadInterface::alarmKeyDetected;
extern volatile bool dscClassicKeypadInterface::alarmKeyResponsePending;
extern volatile byte dscClassicKeypadInterface::clockCycleCount;
extern volatile byte dscClassicKeypadInterface::clockCycleTotal;
extern volatile byte dscClassicKeypadInterface::panelCommand[dscReadSize];
extern volatile byte dscClassicKeypadInterface::isrPanelBitTotal;
extern volatile byte dscClassicKeypadInterface::isrPanelBitCount;
extern volatile byte dscClassicKeypadInterface::panelCommandByteCount;
extern volatile byte dscClassicKeypadInterface::isrModuleData[dscReadSize];
extern volatile byte dscClassicKeypadInterface::isrModuleBitTotal;
extern volatile byte dscClassicKeypadInterface::isrModuleBitCount;
extern volatile byte dscClassicKeypadInterface::isrModuleByteCount;
extern volatile byte dscClassicKeypadInterface::panelCommandByteTotal;
extern volatile byte dscClassicKeypadInterface::moduleData[dscReadSize];
extern volatile unsigned long dscClassicKeypadInterface::intervalStart;
extern volatile unsigned long dscClassicKeypadInterface::beepInterval;
extern volatile unsigned long dscClassicKeypadInterface::repeatInterval;
extern volatile unsigned long dscClassicKeypadInterface::keyInterval;
extern volatile unsigned long dscClassicKeypadInterface::alarmKeyTime;
extern volatile unsigned long dscClassicKeypadInterface::alarmKeyInterval;

#if defined(__AVR__)
ISR(TIMER1_OVF_vect) {
  dscClassicKeypadInterface::dscClockInterrupt();
}
#endif  // __AVR__


// DSC PowerSeries
#else
#include "dscKeybus.h"

extern byte dscKeybusInterface::dscClockPin;
extern byte dscKeybusInterface::dscReadPin;
extern byte dscKeybusInterface::dscWritePin;
extern char dscKeybusInterface::writeKey;
extern byte dscKeybusInterface::writePartition;
extern byte dscKeybusInterface::writeByte;
extern byte dscKeybusInterface::writeBit;
extern bool dscKeybusInterface::virtualKeypad;
extern bool dscKeybusInterface::processModuleData;
extern byte dscKeybusInterface::panelData[dscReadSize];
extern byte dscKeybusInterface::panelByteCount;
extern byte dscKeybusInterface::panelBitCount;
extern volatile bool dscKeybusInterface::writeKeyPending;
extern volatile byte dscKeybusInterface::moduleData[dscReadSize];
extern volatile bool dscKeybusInterface::moduleDataCaptured;
extern volatile bool dscKeybusInterface::moduleDataDetected;
extern volatile byte dscKeybusInterface::moduleByteCount;
extern volatile byte dscKeybusInterface::moduleBitCount;
extern volatile bool dscKeybusInterface::writeAlarm;
extern volatile bool dscKeybusInterface::starKeyCheck;
extern volatile bool dscKeybusInterface::starKeyWait[dscPartitions];
extern volatile bool dscKeybusInterface::bufferOverflow;
extern volatile byte dscKeybusInterface::panelBufferLength;
extern volatile byte dscKeybusInterface::panelBuffer[dscBufferSize][dscReadSize];
extern volatile byte dscKeybusInterface::panelBufferBitCount[dscBufferSize];
extern volatile byte dscKeybusInterface::panelBufferByteCount[dscBufferSize];
extern volatile byte dscKeybusInterface::isrPanelData[dscReadSize];
extern volatile byte dscKeybusInterface::isrPanelByteCount;
extern volatile byte dscKeybusInterface::isrPanelBitCount;
extern volatile byte dscKeybusInterface::isrPanelBitTotal;
extern volatile byte dscKeybusInterface::isrModuleData[dscReadSize];
extern volatile byte dscKeybusInterface::currentCmd;
extern volatile byte dscKeybusInterface::statusCmd;
extern volatile byte dscKeybusInterface::moduleCmd;
extern volatile byte dscKeybusInterface::moduleSubCmd;
extern volatile unsigned long dscKeybusInterface::clockHighTime;
extern volatile unsigned long dscKeybusInterface::keybusTime;

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
