#ifndef __SENSOR_CURRENT_VOLTAGE_INA3221_H__
#define __SENSOR_CURRENT_VOLTAGE_INA3221_H__


#include "I2C.h"


/*
 * http://www.ti.com/lit/ds/symlink/ina3221.pdf
 *
 * 12-bit bus voltage readings (plus pos/neg bit)
 *
 * Assumes shim resistor value of 0.1 ohm.
 *
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
        
        uint8_t GetShuntMicroVolts(uint16_t &microVolts)
        {
            uint8_t retVal = 0;
            
            if (channelNumber_ == 1)
            {
                retVal = sensor_->GetChannel1ShuntMicroVolts(microVolts);
            }
            else if (channelNumber_ == 2)
            {
                retVal = sensor_->GetChannel2ShuntMicroVolts(microVolts);
            }
            else if (channelNumber_ == 3)
            {
                retVal = sensor_->GetChannel3ShuntMicroVolts(microVolts);
            }
            
            return retVal;
        }
        
        uint8_t GetShuntMilliAmps(uint16_t &milliAmps)
        {
            uint8_t retVal = 0;
            
            if (channelNumber_ == 1)
            {
                retVal = sensor_->GetChannel1ShuntMilliAmps(milliAmps);
            }
            else if (channelNumber_ == 2)
            {
                retVal = sensor_->GetChannel2ShuntMilliAmps(milliAmps);
            }
            else if (channelNumber_ == 3)
            {
                retVal = sensor_->GetChannel3ShuntMilliAmps(milliAmps);
            }
            
            return retVal;
        }
        
        uint8_t GetBusMilliVolts(uint16_t &milliVolts)
        {
            uint8_t retVal = 0;
            
            if (channelNumber_ == 1)
            {
                retVal = sensor_->GetChannel1BusMilliVolts(milliVolts);
            }
            else if (channelNumber_ == 2)
            {
                retVal = sensor_->GetChannel2BusMilliVolts(milliVolts);
            }
            else if (channelNumber_ == 3)
            {
                retVal = sensor_->GetChannel3BusMilliVolts(milliVolts);
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
    
    
    uint8_t GetChannel1ShuntMicroVolts(uint16_t &microVolts)
    {
        return GetShuntMicroVolts(0x01, microVolts);
    }
    
    uint8_t GetChannel1ShuntMilliAmps(uint16_t &milliAmps)
    {
        return GetShuntMilliAmps(0x01, milliAmps);
    }
    
    uint8_t GetChannel1BusMilliVolts(uint16_t &milliVolts)
    {
        return GetBusMilliVolts(0x02, milliVolts);
    }
    
    
    uint8_t GetChannel2ShuntMicroVolts(uint16_t &microVolts)
    {
        return GetShuntMicroVolts(0x03, microVolts);
    }
    
    uint8_t GetChannel2ShuntMilliAmps(uint16_t &milliAmps)
    {
        return GetShuntMilliAmps(0x03, milliAmps);
    }
    
    uint8_t GetChannel2BusMilliVolts(uint16_t &milliVolts)
    {
        return GetBusMilliVolts(0x04, milliVolts);
    }
    
    
    uint8_t GetChannel3ShuntMicroVolts(uint16_t &microVolts)
    {
        return GetShuntMicroVolts(0x05, microVolts);
    }
    
    uint8_t GetChannel3ShuntMilliAmps(uint16_t &milliAmps)
    {
        return GetShuntMilliAmps(0x05, milliAmps);
    }
    
    uint8_t GetChannel3BusMilliVolts(uint16_t &milliVolts)
    {
        return GetBusMilliVolts(0x06, milliVolts);
    }
    
    
    
    
    

private:


    uint8_t GetShuntMicroVolts(uint8_t addr, uint16_t &microVolts)
    {
        uint16_t regVal = 0;
        uint8_t  retVal = I2C.ReadRegister16(I2C_ADDR, addr, regVal);
        
        microVolts = ConvertShuntRegValToMicroVolts(regVal);
        
        return retVal;
    }
    
    uint8_t GetShuntMilliAmps(uint8_t addr, uint16_t &milliAmps)
    {
        uint16_t regVal = 0;
        uint8_t  retVal = I2C.ReadRegister16(I2C_ADDR, addr, regVal);
        
        milliAmps = ConvertShuntRegValToMilliAmps(regVal);
        
        return retVal;
    }
    
    uint8_t GetBusMilliVolts(uint8_t addr, uint16_t &milliVolts)
    {
        uint16_t regVal = 0;
        uint8_t  retVal = I2C.ReadRegister16(I2C_ADDR, addr, regVal);
        
        milliVolts = ConvertBusRegValToMilliVolts(regVal);
        
        return retVal;
    }
    




    /*
     * Register values top out at expressing 163.8mV.
     * Increments of 40uV.
     *
     * Can decode by:
     * [todo: take care of sign bit]
     *
     * 0b0000000111110000 = 496
     * - shift right by 3 to remove trailing 3 0-padded bitset (0b0000000000111110 =     62   )
     * - multiply by 40uV step                                 (0b0000100110110000 =  2,480 uV)
     *
     * However, notice that
     * - shift right by 3 is divide by 8
     * - multiply by 40 is multiply by 4 then multiply by 10
     * 
     * So, why not:
     * - take initial value 496
     * - divide by 2 (combine div by 8 and mult by 4)
     * - multiply by 10 (the rest of the multiply by 40 after the initial 4 above)
     *
     */
    uint16_t ConvertShuntRegValToMicroVolts(uint16_t regVal)
    {
        return ((regVal >> 1) * 10);
    }
    
    /*
     * Register values top out at expressing 163.8mV.
     * Increments of 40uV.
     *
     * Can decode by:
     * [todo: take care of sign bit]
     *
     * 0b0000000111110000 = 496
     * - shift right by 3 to remove trailing 3 0-padded bitset (0b0000000000111110 =     62   )
     * - multiply by 40uV step                                 (0b0000100110110000 =  2,480 uV)
     * - divide by shim resistor value (mOhm = 100 (0.1 ohm))
     *   which is the same as multiplying by 10                (0b0110000011100000 = 24,800 uV)
     * - divide by 1000 to go from uV to mV                    (0b0000000000011000 =     24 mV)
     *
     * However, notice that
     * - shift right by 3 is divide by 8
     * - multiply by 40 is multiply by 4 then multiply by 10
     * - then another multiply by 10
     * - then divide by 1000
     * 
     * So, why not:
     * - take initial value 496
     * - divide by 2 (combine div by 8 and mult by 4)
     * - divide by 10 (combine 10 * 10 / 1000)
     *
     */
    uint16_t ConvertShuntRegValToMilliAmps(uint16_t regVal)
    {
        return ((regVal >> 1) / 10);
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
    uint16_t ConvertBusRegValToMilliVolts(uint16_t regVal)
    {
        return regVal;
    }
    

private:

    uint16_t mOhm_;
    
    Channel channel1_;
    Channel channel2_;
    Channel channel3_;

};



#endif  // __SENSOR_CURRENT_VOLTAGE_INA3221_H__











