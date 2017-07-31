#ifndef __SHIFT_REGISTER_OUT_H__
#define __SHIFT_REGISTER_OUT_H__


#include "PAL.h"


//
// Tested on 74HC595N (SIPO)
// http://www.ti.com/lit/ds/symlink/sn74hc595.pdf
// https://learn.adafruit.com/adafruit-arduino-lesson-4-eight-leds/arduino-code
// https://www.arduino.cc/en/Tutorial/ShiftOut


class ShiftRegisterOut
{
public:
    ShiftRegisterOut(uint8_t pinClock,
                     uint8_t pinLatch,
                     uint8_t pinSerial)
    : pinClock_(pinClock)
    , pinLatch_(pinLatch)
    , pinSerial_(pinSerial)
    {
        Setup();
    }
    
    void ShiftOut(uint8_t bitList)
    {
        ShiftOut(&bitList, 1);
    }
    
    void ShiftOut(uint8_t *buf, uint8_t bufLen)
    {
        // The first bit value is already available for reading.
        // However, tell the shift register to start paying attention to the
        // clock for all subsequent reads.
        PAL.DigitalWrite(pinLatch_, LOW);
        
        for (uint16_t i = 0; i < bufLen; ++i)
        {
            ClockOutOneByte(buf[i]);
        }
        
        // Tell the shift register to stop looking at the clock
        PAL.DigitalWrite(pinLatch_, HIGH);
    }


private:

    void ClockOutOneByte(uint8_t bitList)
    {
        // Actually extract the serialized data
        // Most significant bit first
        for (uint8_t i = 0; i < 8; ++i)
        {
            PAL.DigitalWrite(pinSerial_, (bitList & (1 << (7 - i))) ? HIGH : LOW);
            
            // Pulse the clock so the data is read
            PAL.DigitalWrite(pinClock_, HIGH);
            PAL.DigitalWrite(pinClock_, LOW);
        }
    }

    void Setup()
    {
        // Set pin input/output modes
        PAL.PinMode(pinClock_, OUTPUT);
        PAL.PinMode(pinLatch_, OUTPUT);
        PAL.PinMode(pinSerial_, OUTPUT);
        
        // Set initial state of pins
        PAL.DigitalWrite(pinClock_, LOW);
        PAL.DigitalWrite(pinLatch_, HIGH);
    }

    Pin pinClock_;
    Pin pinLatch_;
    Pin pinSerial_;
};


#endif  // __SHIFT_REGISTER_OUT_H__


