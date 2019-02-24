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
        
        uint8_t GetShuntMicroVolts(int16_t &microVolts)
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
        
        uint8_t GetShuntMilliAmps(int16_t &milliAmps)
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
        
        uint8_t GetBusMilliVolts(int16_t &milliVolts)
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

    ////////////////////////////////////////////////////////////////////////////
    //
    // Configuration
    //
    ////////////////////////////////////////////////////////////////////////////
    
    uint8_t GetChannel1Enable(uint8_t &enable)
    {
        return GetChannelEnable(1, enable);
    }
    
    uint8_t SetChannel1Enable(uint8_t enable)
    {
        return SetChannelEnable(1, enable);
    }
    
    uint8_t GetChannel2Enable(uint8_t &enable)
    {
        return GetChannelEnable(2, enable);
    }
    
    uint8_t SetChannel2Enable(uint8_t enable)
    {
        return SetChannelEnable(2, enable);
    }
    
    uint8_t GetChannel3Enable(uint8_t &enable)
    {
        return GetChannelEnable(3, enable);
    }

    uint8_t SetChannel3Enable(uint8_t enable)
    {
        return SetChannelEnable(3, enable);
    }

    
    uint8_t GetAverageSampleCount(uint16_t &averageSampleCount)
    {
        uint8_t retVal = 0;
        
        uint16_t regVal = 0;
        
        if (GetRegConfiguration(regVal))
        {
            retVal = 1;
            
            uint16_t avgCfg__averageSampleCount[] = {
                1,  // default
                4,
                16,
                64,
                128,
                256,
                512,
                1024
            };
            
            uint8_t avgCfg = (regVal & 0b0000111000000000) >> 9;
            
            averageSampleCount = avgCfg__averageSampleCount[avgCfg];
        }
        
        return retVal;
    }
    
    uint8_t SetAverageSampleCount(uint16_t averageSampleCount)
    {
        uint8_t retVal = 0;
        
        uint16_t regVal = 0;
        
        if (GetRegConfiguration(regVal))
        {
            uint16_t avgCfg = 0;
            
                 if (averageSampleCount ==    1) { avgCfg = 0; }
            else if (averageSampleCount ==    4) { avgCfg = 1; }
            else if (averageSampleCount ==   16) { avgCfg = 2; }
            else if (averageSampleCount ==   64) { avgCfg = 3; }
            else if (averageSampleCount ==  128) { avgCfg = 4; }
            else if (averageSampleCount ==  256) { avgCfg = 5; }
            else if (averageSampleCount ==  512) { avgCfg = 6; }
            else if (averageSampleCount == 1024) { avgCfg = 7; }
            
            uint16_t maskPlace  = (0b0000000000000111 << 9);
            uint16_t bits       = avgCfg << 9;
            
            regVal = (regVal & ~maskPlace) | bits;
            
            retVal = SetRegConfiguration(regVal);
        }
        
        return retVal;
    }
    
    uint8_t GetBusVoltageConversionTimeUs(uint16_t &cvtTimeUs)
    {
        return GetConversionTimeUs(0b0000000111000000, 6, cvtTimeUs);
    }
    
    uint8_t SetBusVoltageConversionTimeUs(uint16_t cvtTimeUs)
    {
        return SetConversionTimeUs(0b0000000111000000, 6, cvtTimeUs);
    }
    
    uint8_t GetShuntVoltageConversionTimeUs(uint16_t &cvtTimeUs)
    {
        return GetConversionTimeUs(0b0000000000111000, 3, cvtTimeUs);
    }
    
    uint8_t SetShuntVoltageConversionTimeUs(uint16_t cvtTimeUs)
    {
        return SetConversionTimeUs(0b0000000000111000, 3, cvtTimeUs);
    }
    
    uint8_t GetOperatingMode(uint8_t &operatingMode)
    {
        uint8_t retVal = 0;
        
        uint16_t regVal = 0;
        
        if (GetRegConfiguration(regVal))
        {
            retVal = 1;
            
            operatingMode = (regVal & 0b0000000000000111);
        }
        
        return retVal;
    }
    
    uint8_t SetOperatingMode(uint8_t operatingMode)
    {
        uint8_t retVal = 0;
        
        uint16_t regVal = 0;
        
        if (GetRegConfiguration(regVal))
        {
            uint16_t maskPlace  = 0b0000000000000111;
            uint16_t bits       = operatingMode & maskPlace;
            
            regVal = (regVal & ~maskPlace) | bits;
            
            retVal = SetRegConfiguration(regVal);            
        }
        
        return retVal;
    }
    
    void PrintConfiguration()
    {
        uint16_t regVal = 0;
        
        if (GetRegConfiguration(regVal))
        {
            uint8_t ch1Enabled = 0;
            uint8_t ch2Enabled = 0;
            uint8_t ch3Enabled = 0;
            GetChannel1Enable(ch1Enabled);
            GetChannel2Enable(ch2Enabled);
            GetChannel3Enable(ch3Enabled);
            Log(P("CH1: "), ch1Enabled);
            Log(P("CH2: "), ch2Enabled);
            Log(P("CH3: "), ch3Enabled);

            uint16_t averageSampleCount = 0;
            GetAverageSampleCount(averageSampleCount);
            Log(P("AvgSamples: "), averageSampleCount);
            
            uint16_t busVoltageConvertTimeUs = 0;
            GetBusVoltageConversionTimeUs(busVoltageConvertTimeUs);
            Log(P("BusCvtTimeUs: :"), busVoltageConvertTimeUs);
            
            uint16_t shuntVoltageConvertTimeUs = 0;
            GetShuntVoltageConversionTimeUs(shuntVoltageConvertTimeUs);
            Log(P("ShuntCvtTimeUs: :"), shuntVoltageConvertTimeUs);
            
            uint8_t operatingMode = 0;
            GetOperatingMode(operatingMode);
            Log(P("OpMode: "), LogBIN(operatingMode));
        }
    }
    
    ////////////////////////////////////////////////////////////////////////////
    //
    // Channel access objects
    //
    ////////////////////////////////////////////////////////////////////////////
    
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
    
    
    ////////////////////////////////////////////////////////////////////////////
    //
    // Channel reading
    //
    ////////////////////////////////////////////////////////////////////////////
    
    uint8_t GetChannel1ShuntMicroVolts(int16_t &microVolts)
    {
        return GetShuntMicroVolts(0x01, microVolts);
    }
    
    uint8_t GetChannel1ShuntMilliAmps(int16_t &milliAmps)
    {
        return GetShuntMilliAmps(0x01, milliAmps);
    }
    
    uint8_t GetChannel1BusMilliVolts(int16_t &milliVolts)
    {
        return GetBusMilliVolts(0x02, milliVolts);
    }
    

    uint8_t GetChannel2ShuntMicroVolts(int16_t &microVolts)
    {
        return GetShuntMicroVolts(0x03, microVolts);
    }
    
    uint8_t GetChannel2ShuntMilliAmps(int16_t &milliAmps)
    {
        return GetShuntMilliAmps(0x03, milliAmps);
    }
    
    uint8_t GetChannel2BusMilliVolts(int16_t &milliVolts)
    {
        return GetBusMilliVolts(0x04, milliVolts);
    }
    
    
    uint8_t GetChannel3ShuntMicroVolts(int16_t &microVolts)
    {
        return GetShuntMicroVolts(0x05, microVolts);
    }
    
    uint8_t GetChannel3ShuntMilliAmps(int16_t &milliAmps)
    {
        return GetShuntMilliAmps(0x05, milliAmps);
    }
    
    uint8_t GetChannel3BusMilliVolts(int16_t &milliVolts)
    {
        return GetBusMilliVolts(0x06, milliVolts);
    }
    
    

//private:

    uint8_t GetRegConfiguration(uint16_t &regVal)
    {
        return I2C.ReadRegister16(I2C_ADDR, REG_CONFIGURATION, regVal);
    }
    
    uint8_t SetRegConfiguration(uint16_t regVal)
    {
        return I2C.WriteRegister16(I2C_ADDR, REG_CONFIGURATION, regVal);
    }
    
    
    uint8_t GetChannelEnable(uint8_t chanNum, uint8_t &enabled)
    {
        uint8_t retVal = 0;
        
        uint16_t regVal = 0;
        
        if (GetRegConfiguration(regVal))
        {
            retVal = 1;
            
            enabled = !!((0b1000000000000000 >> chanNum) & regVal);
        }
        
        return retVal;
    }
    
    uint8_t SetChannelEnable(uint8_t chanNum, uint8_t enable)
    {
        uint8_t retVal = 0;
        
        uint16_t regVal = 0;
        
        if (GetRegConfiguration(regVal))
        {
            uint16_t maskPlace  =          (0b1000000000000000 >> chanNum);
            uint16_t bits       = enable ? (0b1000000000000000 >> chanNum) : 0;
            
            regVal = (regVal & ~maskPlace) | bits;
            
            retVal = SetRegConfiguration(regVal);
        }
        
        return retVal;
    }
    
    
    uint8_t GetConversionTimeUs(uint16_t mask, uint8_t shift, uint16_t &cvtTimeUs)
    {
        uint8_t retVal = 0;
        
        uint16_t regVal = 0;
        
        if (GetRegConfiguration(regVal))
        {
            retVal = 1;
            
            uint16_t ctCfg__conversionTimeUs[] = {
                140,
                204,
                332,
                588,
                1100,   // default
                2116,
                4156,
                8244,
            };
            
            uint8_t ctCfg = (regVal & mask) >> shift;
            
            cvtTimeUs = ctCfg__conversionTimeUs[ctCfg];
        }
        
        return retVal;
    }
    
    uint8_t SetConversionTimeUs(uint16_t mask, uint8_t shift, uint16_t cvtTimeUs)
    {
        uint8_t retVal = 0;
        
        uint16_t regVal = 0;
        
        if (GetRegConfiguration(regVal))
        {
            uint16_t ctCfg = 0;
            
                 if (cvtTimeUs ==  140) { ctCfg = 0; }
            else if (cvtTimeUs ==  204) { ctCfg = 1; }
            else if (cvtTimeUs ==  332) { ctCfg = 2; }
            else if (cvtTimeUs ==  588) { ctCfg = 3; }
            else if (cvtTimeUs == 1100) { ctCfg = 4; }
            else if (cvtTimeUs == 2116) { ctCfg = 5; }
            else if (cvtTimeUs == 4156) { ctCfg = 6; }
            else if (cvtTimeUs == 8244) { ctCfg = 7; }
            
            uint16_t maskPlace  = mask;
            uint16_t bits       = ctCfg << shift;
            
            regVal = (regVal & ~maskPlace) | bits;
            
            retVal = SetRegConfiguration(regVal);
        }
        
        return retVal;
    }

    


    uint8_t GetShuntMicroVolts(uint8_t addr, int16_t &microVolts)
    {
        uint16_t regVal = 0;
        uint8_t  retVal = I2C.ReadRegister16(I2C_ADDR, addr, regVal);
        
        microVolts = ConvertShuntRegValToMicroVolts(regVal);
        
        return retVal;
    }
    
    uint8_t GetShuntMilliAmps(uint8_t addr, int16_t &milliAmps)
    {
        uint16_t regVal = 0;
        uint8_t  retVal = I2C.ReadRegister16(I2C_ADDR, addr, regVal);
        
        milliAmps = ConvertShuntRegValToMilliAmps(regVal);
        
        return retVal;
    }
    
    uint8_t GetBusMilliVolts(uint8_t addr, int16_t &milliVolts)
    {
        uint16_t regVal = 0;
        uint8_t  retVal = I2C.ReadRegister16(I2C_ADDR, addr, regVal);
        
        milliVolts = ConvertBusRegValToMilliVolts(regVal);
        
        return retVal;
    }
    

    /*
     * Some values conveyed by the module may be returned in two's compliment.
     *
     * However, the absolute value of the value may require processing before
     * being actually useful.
     *
     * So this function effectively returns the absolute integer value along
     * with an indication of whether it was negative or not.
     *
     * It is up to the calling code to apply the negation post-processing.
     *
     */
    uint8_t ComplimentIfNecessary(uint16_t &val)
    {
        uint8_t isNegative = 0;
        
        if ((int16_t)val < 0)
        {
            isNegative = 1;
            
            val = ~val + 1;
        }
        
        return isNegative;
    }
    

    /*
     * Register values top out at expressing 163.8mV.
     * Increments of 40uV.
     *
     * Can decode by:
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
    int16_t ConvertShuntRegValToMicroVolts(uint16_t regVal)
    {
        uint8_t isNegative = ComplimentIfNecessary(regVal);
        int16_t retVal     = (int16_t)((regVal >> 1) * 10);
        
        retVal = isNegative ? -retVal : retVal;
        
        return retVal;
    }
    
    /*
     * Register values top out at expressing 163.8mV.
     * Increments of 40uV.
     *
     * Can decode by:
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
    int16_t ConvertShuntRegValToMilliAmps(uint16_t regVal)
    {
        uint8_t isNegative = ComplimentIfNecessary(regVal);
        int16_t retVal     = (int16_t)((regVal >> 1) / 10);
        
        retVal = isNegative ? -retVal : retVal;
        
        return retVal;
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
     *
     * 0b0001001100001000 = 4872 -- this is 4.872V
     *
     * or, determine how many steps into the 12-bit number we are, and scale
     * to top range of 32.76V
     *
     * (4872 >> 3 = 609) / (2^12=4096) * 32.76 = 4.870810546875
     * 
     */
    int16_t ConvertBusRegValToMilliVolts(uint16_t regVal)
    {
        uint8_t isNegative = ComplimentIfNecessary(regVal);
        int16_t retVal     = (int16_t)regVal;
        
        retVal = isNegative ? -retVal : retVal;
        
        return retVal;
    }
    

private:

    Channel channel1_;
    Channel channel2_;
    Channel channel3_;
};



#endif  // __SENSOR_CURRENT_VOLTAGE_INA3221_H__











