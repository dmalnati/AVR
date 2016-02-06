/*
 * Douglas Malnati
 * 2016-01-31
 *
 * Practice from example at:
 * https://learn.sparkfun.com/tutorials/shift-registers
 *
 * Works on NXP 74HC165N PISO
 */


#include "MyShiftRegister.h"


MyShiftRegister::
MyShiftRegister(uint8_t pinLoad,
                uint8_t pinClock,
                uint8_t pinClockEnable,
                uint8_t pinSerial)
: pinLoad_(pinLoad)
, pinClock_(pinClock)
, pinClockEnable_(pinClockEnable)
, pinSerial_(pinSerial)
{
    Setup();
}

void MyShiftRegister::Setup()
{
    // Set pin input/output modes
    pinMode(pinLoad_,        OUTPUT);
    pinMode(pinClock_,       OUTPUT);
    pinMode(pinClockEnable_, OUTPUT);
    pinMode(pinSerial_,      INPUT);
    
    // Set initial state of pins
    digitalWrite(pinClockEnable_, HIGH);
    digitalWrite(pinLoad_,        HIGH);
}

uint8_t MyShiftRegister::ShiftIn()
{
    uint8_t retVal = 0;
    
    // Tell the shift register that it should lock the parallel values
    digitalWrite(pinLoad_, LOW);
    delayMicroseconds(5);
    digitalWrite(pinLoad_, HIGH);
    delayMicroseconds(5);
    
    // Tell the shift register to start paying attention to the clock for
    // shifting
    digitalWrite(pinClock_,       HIGH);
    digitalWrite(pinClockEnable_, LOW);
    
    // Actually extract the serialized data
    retVal = shiftIn(pinSerial_, pinClock_, MSBFIRST);
    
    // Tell the shift register to stop looking at the clock
    digitalWrite(pinClockEnable_, HIGH);
    
    return retVal;
}





















