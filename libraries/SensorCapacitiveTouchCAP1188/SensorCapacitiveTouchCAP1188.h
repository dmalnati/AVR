#ifndef __SENSOR_CAPACITIVE_TOUCH_CAP1188_H__
#define __SENSOR_CAPACITIVE_TOUCH_CAP1188_H__


#include "TWI.h"


// http://ww1.microchip.com/downloads/en/DeviceDoc/CAP1188%20.pdf

class SensorCapacitiveTouchCAP1188
{
    static const uint8_t REG_MAIN_CONTROL        = 0x00;
    static const uint8_t REG_SENSOR_INPUT_STATUS = 0x03;
    
public:
    SensorCapacitiveTouchCAP1188(uint8_t addr)
    : addr_(addr)
    {
        // Nothing to do
        
        EnableLEDs();
        EnableMultiTouch(); // Much faster response time from device
        //SetGain();
        //SetSensitivity();
    }
    
    void EnableLEDs()
    {
        // Enable LEDs
        TWI.WriteRegister(addr_, 0x72, 0xFF);
    }
    
    void EnableMultiTouch()
    {
        TWI.WriteRegister(addr_, 0x2A, 0x00);
    }
    
    void SetGain()
    {
        // Increase Gain
        uint8_t regVal;
        TWI.ReadRegister(addr_, REG_MAIN_CONTROL, regVal);
        regVal = ((regVal & 0b00111111) | 0b01000000);  // 2x
        //regVal = ((regVal & 0b00111111) | 0b10000000);  // 4x
        //regVal = ((regVal & 0b00111111) | 0b11000000);  // 8x -- way too sensitive
        TWI.WriteRegister(addr_, REG_MAIN_CONTROL, regVal);
    }
    
    void SetSensitivity()
    {
        uint8_t regVal;
        TWI.ReadRegister(addr_, 0x1F, regVal);
        regVal = ((regVal & 0b10001111) | 0b00010000);  // 64x (from default 32x)
        TWI.WriteRegister(addr_, REG_MAIN_CONTROL, regVal);
    }

    uint8_t GetTouched()
    {
        // Get current value
        uint8_t touched = 0;
        TWI.ReadRegister(addr_, REG_SENSOR_INPUT_STATUS, touched);

        // Tell sensor to clear non-current touches, so next read will
        // only show what is current (as opposed to what has been touched
        // ever since prior poll)
        //
        // The main control register has an interrupt flag which must be set
        // to zero when you want this to occur.
        // That flag is a single bit at 0b00000001
        uint8_t regVal;
        TWI.ReadRegister(addr_, REG_MAIN_CONTROL, regVal);
        regVal &= 0b11111110;
        TWI.WriteRegister(addr_, REG_MAIN_CONTROL, regVal);
        
        return touched;
    }

private:

    uint8_t addr_;
};


#endif  // __SENSOR_CAPACITIVE_TOUCH_CAP1188_H__