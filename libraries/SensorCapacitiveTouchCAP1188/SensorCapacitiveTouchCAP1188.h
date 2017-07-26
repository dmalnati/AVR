#ifndef __SENSOR_CAPACITIVE_TOUCH_CAP1188_H__
#define __SENSOR_CAPACITIVE_TOUCH_CAP1188_H__


#include "TWI.h"


// http://ww1.microchip.com/downloads/en/DeviceDoc/CAP1188%20.pdf

class SensorCapacitiveTouchCAP1188
{
    static const uint8_t REG_SENSOR_INPUT_STATUS = 0x03;
    
public:
    SensorCapacitiveTouchCAP1188(uint8_t addr)
    : addr_(addr)
    {
        // Nothing to do
    }

    uint8_t GetTouched()
    {
        uint8_t reg = REG_SENSOR_INPUT_STATUS;
        uint8_t touched = 0;
        
        TWI.SendAndReceive(addr_, &reg, 1, &touched, 1);
        
        return touched;
    }

private:

    uint8_t addr_;
};


#endif  // __SENSOR_CAPACITIVE_TOUCH_CAP1188_H__