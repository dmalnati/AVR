#ifndef __SENSOR_CAPACITIVE_TOUCH_CAP1188_H__
#define __SENSOR_CAPACITIVE_TOUCH_CAP1188_H__


#include "PAL.h"
#include "I2C.h"
#include "TimedEventHandler.h"


// http://ww1.microchip.com/downloads/en/DeviceDoc/CAP1188%20.pdf

class SensorCapacitiveTouchCAP1188
{
    static const uint8_t REG_MAIN_CONTROL                 = 0x00;
    static const uint8_t REG_SENSOR_INPUT_STATUS          = 0x03;
    static const uint8_t REG_SENSITIVITY_CONTROL          = 0x1F;
    static const uint8_t REG_SENSOR_INPUT_ENABLE          = 0x21;
    static const uint8_t REG_MULTIPLE_TOUCH_CONFIGURATION = 0x2A;
    static const uint8_t REG_SENSOR_INPUT_LED_LINKING     = 0x72;
    
    static const uint8_t REG_SENSOR_STANDBY_INPUT_ENABLE  = 0x40;
    static const uint8_t REG_STANDBY_CONFIGURATION        = 0x41;
    static const uint8_t REG_STANDBY_SENSITIVITY_CONTROL  = 0x42;
    
    static const uint32_t DEFAULT_POLL_PERIOD_MS = 100;
    
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
    void SetInputEnable(uint8_t bitmap)
    {
        uint8_t bitmapUse = PAL.BitReverse(bitmap);
        I2C.WriteRegister(addr_, REG_SENSOR_INPUT_ENABLE, bitmapUse);
    }
    
    void PowerActive()
    {
        uint8_t regVal;
        I2C.ReadRegister(addr_, REG_MAIN_CONTROL, regVal);
        regVal &= ~0b00100000;
        I2C.WriteRegister(addr_, REG_MAIN_CONTROL, regVal);
    }
    
    void PowerStandby()
    {
        uint8_t regVal;
        I2C.ReadRegister(addr_, REG_MAIN_CONTROL, regVal);
        regVal |= 0b00100000;
        I2C.WriteRegister(addr_, REG_MAIN_CONTROL, regVal);
    }
    
    
    enum class StandbyAvgOrSum : uint8_t
    {
        AVG = 0,  // default
        SUM = 1,
    };
    
    enum class StandbySamplesPerMeasurement : uint8_t
    {
        COUNT_1   = 0,
        COUNT_2   = 1,
        COUNT_4   = 2,
        COUNT_8   = 3,  // default
        COUNT_16  = 4,
        COUNT_32  = 5,
        COUNT_64  = 6,
        COUNT_128 = 7,
    };
    
    enum class StandbySampleTime : uint8_t
    {
        US_320  = 0,
        US_640  = 1,
        US_1280 = 2,  // default
        US_2560 = 3,
    };
    
    enum class StandbyCycleTime : uint8_t
    {
        MS_35  = 0,
        MS_70  = 1,  // default
        MS_105 = 2,
        MS_140 = 3,
    };
    
    void StandbyConfigure(StandbyAvgOrSum               avgOrSum,
                          StandbySamplesPerMeasurement  samplesPerMeasurement,
                          StandbySampleTime             sampleTime,
                          StandbyCycleTime              cycleTime)
    {
        uint8_t regVal = 
            (((uint8_t)avgOrSum) << 7)              |
            (((uint8_t)samplesPerMeasurement) << 4) |
            (((uint8_t)sampleTime) << 2)            |
            ((uint8_t)cycleTime);
        
        I2C.WriteRegister(addr_, REG_STANDBY_CONFIGURATION, regVal);
    }
    
    
    
    // Multiplying factor, 2^x
    // Inputs range 0-7, for a result range of 1-128
    // Higher the value, the more sensitive
    void SetStandbySensitivity(uint8_t multFactor)
    {
        if (multFactor > 7)
        {
            multFactor = 7;
        }
        
        I2C.WriteRegister(addr_, REG_STANDBY_SENSITIVITY_CONTROL, ~multFactor);
    }
    
    
    void SetStandbyInputEnable(uint8_t bitmap)
    {
        uint8_t bitmapUse = PAL.BitReverse(bitmap);
        I2C.WriteRegister(addr_, REG_SENSOR_STANDBY_INPUT_ENABLE, bitmapUse);
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
    
    void SetCallback(function<void(uint8_t bitmapTouched)> cbFn)
    {
        cbFn_ = cbFn;
    }
    
    void SetPollPeriodMs(uint32_t pollPeriodMs)
    {
        pollPeriodMs_ = pollPeriodMs;
    }
    
    void Start()
    {
        Stop();
        
        ted_.SetCallback([this](){
            uint8_t bitmapTouched = GetTouched();
            
            if (bitmapTouched != bitmapTouchedLast_)
            {
                cbFn_(bitmapTouched);
                
                bitmapTouchedLast_ = bitmapTouched;
            }
        });
        
        ted_.RegisterForTimedEventInterval(pollPeriodMs_, 0);
    }
    
    void Stop()
    {
        ted_.DeRegisterForTimedEvent();
        
        bitmapTouchedLast_ = 0;
    }

private:

    uint8_t addr_;
    
    function<void(uint8_t bitmapTouched)> cbFn_;
    uint8_t bitmapTouchedLast_ = 0;
    
    TimedEventHandlerDelegate ted_;
    
    uint32_t pollPeriodMs_ = DEFAULT_POLL_PERIOD_MS;
};


#endif  // __SENSOR_CAPACITIVE_TOUCH_CAP1188_H__









