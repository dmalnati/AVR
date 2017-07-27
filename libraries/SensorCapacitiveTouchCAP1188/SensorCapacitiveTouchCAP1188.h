#ifndef __SENSOR_CAPACITIVE_TOUCH_CAP1188_H__
#define __SENSOR_CAPACITIVE_TOUCH_CAP1188_H__


#include "TWI.h"


// http://ww1.microchip.com/downloads/en/DeviceDoc/CAP1188%20.pdf

class SensorCapacitiveTouchCAP1188
{
    static const uint8_t REG_MAIN_CONTROL        = 0x00;
    static const uint8_t REG_SENSOR_INPUT_STATUS = 0x03;
    static const uint8_t REG_SENSOR_INPUT_LED_LINKING = 0x72;
    static const uint8_t REG_MULTIPLE_TOUCH_CONFIGURATION = 0x2A;
    
public:
    SensorCapacitiveTouchCAP1188(uint8_t addr)
    : addr_(addr)
    {
        // Nothing to do
        
        DisableLEDs();
        EnableMultiTouch();
    }
    
    void EnableLEDs()
    {
        TWI.WriteRegister(addr_, REG_SENSOR_INPUT_LED_LINKING, 0xFF);
    }
    
    void DisableLEDs()
    {
        // This is the default
        TWI.WriteRegister(addr_, REG_SENSOR_INPUT_LED_LINKING, 0x00);
    }
    
    void EnableMultiTouch()
    {
        // This is the default
        TWI.WriteRegister(addr_, REG_MULTIPLE_TOUCH_CONFIGURATION, 0x00);
    }
    
    void DisableMultiTouch()
    {
        TWI.WriteRegister(addr_, REG_MULTIPLE_TOUCH_CONFIGURATION, 0x80);
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









