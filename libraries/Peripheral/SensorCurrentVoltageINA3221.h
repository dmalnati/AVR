#ifndef __SENSOR_CURRENT_VOLTAGE_INA3221_H__
#define __SENSOR_CURRENT_VOLTAGE_INA3221_H__


#include "I2C.h"


/*
 * http://www.ti.com/lit/ds/symlink/ina3221.pdf
 *
 * 12-bit bus voltage readings (plus pos/neg bit)
 */
class SensorCurrentVoltageINA3221
{
    static const uint8_t I2C_ADDR = 0x40;
    
    static const uint8_t REG_CONFIGURATION = 0x00;
    
public:

    class Channel
    {
    public:
    
        Channel(SensorCurrentVoltageINA3221 *sensor, uint8_t channelNumber)
        : sensor_(sensor)
        , channelNumber_(channelNumber)
        {
            // Nothing to do
        }
        
        uint8_t GetShuntVoltage(uint16_t &regVal)
        {
            uint8_t retVal = 0;
            
            if (channelNumber_ == 1)
            {
                retVal = sensor_->GetChannel1ShuntVoltage(regVal);
            }
            else if (channelNumber_ == 2)
            {
                retVal = sensor_->GetChannel2ShuntVoltage(regVal);
            }
            else if (channelNumber_ == 3)
            {
                retVal = sensor_->GetChannel3ShuntVoltage(regVal);
            }
            
            return retVal;
        }
        
        uint8_t GetBusVoltage(uint16_t &regVal)
        {
            uint8_t retVal = 0;
            
            if (channelNumber_ == 1)
            {
                retVal = sensor_->GetChannel1BusVoltage(regVal);
            }
            else if (channelNumber_ == 2)
            {
                retVal = sensor_->GetChannel2BusVoltage(regVal);
            }
            else if (channelNumber_ == 3)
            {
                retVal = sensor_->GetChannel3BusVoltage(regVal);
            }
            
            return retVal;
        }
        
    private:
        SensorCurrentVoltageINA3221 *sensor_;
        
        uint8_t channelNumber_;
    };
    
public:

    SensorCurrentVoltageINA3221()
    : channel1_(this, 1)
    , channel2_(this, 2)
    , channel3_(this, 3)
    {
        // Nothing to do
    }
    
    uint8_t GetRegConfiguration(uint16_t &regVal)
    {
        return I2C.ReadRegister16(I2C_ADDR, REG_CONFIGURATION, regVal);
    }
    
    uint8_t SetRegConfiguration(uint16_t regVal)
    {
        return I2C.WriteRegister16(I2C_ADDR, REG_CONFIGURATION, regVal);
    }
    
    Channel *GetChannel1()
    {
        return &channel1_;
    }
    
    Channel *GetChannel2()
    {
        return &channel2_;
    }
    
    Channel *GetChannel3()
    {
        return &channel3_;
    }
    
    uint8_t GetChannel1ShuntVoltage(uint16_t &regVal)
    {
        const uint8_t ADDR = 0x01;
        
        return I2C.ReadRegister16(I2C_ADDR, ADDR, regVal);
    }
    
    /*
     * 8.6.2.3 Channel-1 Bus-Voltage Register (address = 02h) [reset = 00h]
     *
     * This register stores the bus voltage reading, VBUS, for channel 1.
     *
     * Full-scale range = 32.76 V (decimal = 7FF8);
     * LSB (BD0) = 8 mV.
     *
     * Although the input range is 26 V,
     * the full-scale range of the ADC scaling is 32.76 V.
     * 
     * 15   14   13   12  11  10  9   8   7   6   5   4   3   2 1 0
     * SIGN BD11 BD10 BD9 BD8 BD7 BD6 BD5 BD4 BD3 BD2 BD1 BD0 — — —
     *
     *
     * Can decode by:
     * [todo: take care of sign bit]
     *
     * 0b0001001100001000 = 4872 -- this is 4.872V
     *
     * or, determine how many steps into the 12-bit number we are, and scale
     * to top range of 32.76V
     *
     * (4872 >> 3 = 609) / (2^12=4096) * 32.76 = 4.870810546875
     * 
     */
    uint8_t GetChannel1BusVoltage(uint16_t &regVal)
    {
        const uint8_t ADDR = 0x02;
        
        return I2C.ReadRegister16(I2C_ADDR, ADDR, regVal);
    }
    
    uint8_t GetChannel2ShuntVoltage(uint16_t &regVal)
    {
        const uint8_t ADDR = 0x03;
        
        return I2C.ReadRegister16(I2C_ADDR, ADDR, regVal);
    }
    
    uint8_t GetChannel2BusVoltage(uint16_t &regVal)
    {
        const uint8_t ADDR = 0x04;
        
        return I2C.ReadRegister16(I2C_ADDR, ADDR, regVal);
    }
    
    uint8_t GetChannel3ShuntVoltage(uint16_t &regVal)
    {
        const uint8_t ADDR = 0x05;
        
        return I2C.ReadRegister16(I2C_ADDR, ADDR, regVal);
    }
    
    uint8_t GetChannel3BusVoltage(uint16_t &regVal)
    {
        const uint8_t ADDR = 0x06;
        
        return I2C.ReadRegister16(I2C_ADDR, ADDR, regVal);
    }

    

private:

    Channel channel1_;
    Channel channel2_;
    Channel channel3_;

};



#endif  // __SENSOR_CURRENT_VOLTAGE_INA3221_H__











