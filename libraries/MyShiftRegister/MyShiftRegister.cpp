/*
 * Douglas Malnati
 * 2016-01-31
 *
 * Practice from example at:
 * https://learn.sparkfun.com/tutorials/shift-registers
 *
 * Works on NXP 74HC165N PISO
 */


#include "PAL.h"
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
    PAL.PinMode(pinLoad_,        OUTPUT);
    PAL.PinMode(pinClock_,       OUTPUT);
    PAL.PinMode(pinClockEnable_, OUTPUT);
    PAL.PinMode(pinSerial_,      INPUT);
    
    // Set initial state of pins
    PAL.DigitalWrite(pinClockEnable_, HIGH);
    PAL.DigitalWrite(pinLoad_,        HIGH);
}

uint8_t MyShiftRegister::ShiftIn()
{
    uint8_t retVal = 0;
    
    // Tell the shift register that it should lock the parallel values
    PAL.DigitalWrite(pinLoad_, LOW);
    PAL.DelayMicroseconds(5);
    PAL.DigitalWrite(pinLoad_, HIGH);
    PAL.DelayMicroseconds(5);
    
    // Tell the shift register to start paying attention to the clock for
    // shifting
    PAL.DigitalWrite(pinClock_,       HIGH);
    PAL.DigitalWrite(pinClockEnable_, LOW);
    
    // Actually extract the serialized data
    retVal = PAL.ShiftIn(pinSerial_, pinClock_, MSBFIRST);
    
    // Tell the shift register to stop looking at the clock
    PAL.DigitalWrite(pinClockEnable_, HIGH);
    
    return retVal;
}





















