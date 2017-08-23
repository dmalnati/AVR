#ifndef __SENSOR_CAPACITIVE_TOUCH_CAP1188_H__
#define __SENSOR_CAPACITIVE_TOUCH_CAP1188_H__


#include "PAL.h"
#include "I2C.h"


// http://ww1.microchip.com/downloads/en/DeviceDoc/CAP1188%20.pdf

class SensorCapacitiveTouchCAP1188
{
    static const uint8_t REG_MAIN_CONTROL                 = 0x00;
    static const uint8_t REG_SENSOR_INPUT_STATUS          = 0x03;
    static const uint8_t REG_SENSITIVITY_CONTROL          = 0x1F;
    static const uint8_t REG_SENSOR_INPUT_ENABLE          = 0x21;
    static const uint8_t REG_MULTIPLE_TOUCH_CONFIGURATION = 0x2A;
    static const uint8_t REG_SENSOR_INPUT_LED_LINKING     = 0x72;
    
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
        I2C.WriteRegister(addr_, REG_SENSOR_INPUT_LED_LINKING, 0xFF);
    }
    
    void DisableLEDs()
    {
        // This is the default
        I2C.WriteRegister(addr_, REG_SENSOR_INPUT_LED_LINKING, 0x00);
    }
    
    void EnableMultiTouch()
    {
        // This is the default
        I2C.WriteRegister(addr_, REG_MULTIPLE_TOUCH_CONFIGURATION, 0x00);
    }
    
    void DisableMultiTouch()
    {
        I2C.WriteRegister(addr_, REG_MULTIPLE_TOUCH_CONFIGURATION, 0x80);
    }
    
    // Multiplying factor, 2^x
    // Inputs range 0-7, for a result range of 1-128
    // Higher the value, the more sensitive
    void SetSensitivity(uint8_t multFactor)
    {
        if (multFactor > 7)
        {
            multFactor = 7;
        }
        
        // For some unknowable reason, the implementation in the chip
        // essentially backwards in the sense that flipping the bits of the
        // input to this function is what yields the correct value.
        //
        // http://ww1.microchip.com/downloads/en/DeviceDoc/CAP1188%20.pdf
        // pdf p. 45
        // DELTA_SENSE[2:0] (a 3 bit range)
        
        uint8_t regVal;
        I2C.ReadRegister(addr_, REG_SENSITIVITY_CONTROL, regVal);
        regVal = 0b10001111 | (~multFactor << 4);
        I2C.WriteRegister(addr_, REG_SENSITIVITY_CONTROL, regVal);
    }
    
    // Set which inputs are monitored by the chip
    // Input to this function is a bitmap corresponding to 01234567
    // (this is despite the fact that the chip wants preferences specified in
    //  reverse of that)
    void SetInputEnable(uint8_t bitmap)
    {
        uint8_t bitmapUse = PAL.BitReverse(bitmap);
        I2C.WriteRegister(addr_, REG_SENSOR_INPUT_ENABLE, bitmapUse);
    }
    
    uint8_t GetTouched()
    {
        // Get current value
        uint8_t touched = 0;
        I2C.ReadRegister(addr_, REG_SENSOR_INPUT_STATUS, touched);

        // Tell sensor to clear non-current touches, so next read will
        // only show what is current (as opposed to what has been touched
        // ever since prior poll)
        //
        // The main control register has an interrupt flag which must be set
        // to zero when you want this to occur.
        // That flag is a single bit at 0b00000001
        uint8_t regVal;
        I2C.ReadRegister(addr_, REG_MAIN_CONTROL, regVal);
        regVal &= 0b11111110;
        I2C.WriteRegister(addr_, REG_MAIN_CONTROL, regVal);
        
        return touched;
    }

private:

    uint8_t addr_;
};


#endif  // __SENSOR_CAPACITIVE_TOUCH_CAP1188_H__









