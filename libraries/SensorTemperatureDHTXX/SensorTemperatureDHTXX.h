#ifndef __SENSOR_TEMPERATURE_DHTXX_H__
#define __SENSOR_TEMPERATURE_DHTXX_H__


#include "PAL.h"


// Debug
#include "Utl.h"



template <uint8_t BIT_COUNT>
class BitList
{
public:
    BitList()
    {
        Reset();
    }

    void Reset()
    {
        for (uint8_t i = 0; i < BYTE_COUNT; ++i)
        {
            byteList_[i] = 0;
        }
        
        bitMask_     = 0x80;
        byteListIdx_ = 0;
    }
    
    inline uint8_t PushBack(uint8_t bitVal)
    {
        uint8_t retVal = 0;
        
        if (byteListIdx_ < BYTE_COUNT)
        {
            retVal = 1;
            
            if (bitVal)
            {
                byteList_[byteListIdx_] |= _BV(bitMask_);
            }
            else
            {
                byteList_[byteListIdx_] &= ~_BV(bitMask_);
            }
            
            bitMask_ >>= 1;
            
            if (!bitMask_)
            {
                bitMask_ = 0x80;
                
                ++byteListIdx_;
            }
        }
        
        return retVal;
    }
    
    inline uint8_t operator[](uint8_t bitIdx)
    {
        uint8_t retVal = 0;
        
        if (bitIdx < BIT_COUNT)
        {
            uint8_t byteListIdxTmp = (bitIdx >> 3);
            uint8_t bitMaskTmp     = (0x80 >> (bitIdx - (byteListIdxTmp << 3)));
            
            retVal = (byteList_[byteListIdxTmp] & bitMaskTmp) ? 1 : 0;
        }
        
        return retVal;
    }
    
    inline static uint8_t Size()
    {
        return BIT_COUNT;
    }

private:
    static const uint8_t BYTE_COUNT =
        (BIT_COUNT % 8 == 0 ?
         (BIT_COUNT / 8)    :
         (BIT_COUNT / 8) + 1);

    uint8_t byteList_[BYTE_COUNT];
    
    uint8_t bitMask_;
    uint8_t byteListIdx_;
};



class SensorTemperatureDHTXX
{
public:
    SensorTemperatureDHTXX() : pin_(0), D1(12), D2(13) { }
    ~SensorTemperatureDHTXX() { }
    
    uint8_t Init(uint8_t pin)
    {
        pin_ = pin;
        
        PAL.PinMode(pin_, INPUT_PULLUP);

        return 1;
    }

    /*
    uint8_t SpeedTest(uint8_t pin)
    {
        pin_ = pin;
        
        // Bring line HIGH as default state
        PAL.PinMode(pin_, INPUT_PULLUP);
        
        
        // Test new Pin interface
        Pin p1(5);
        
        PAL.DigitalRead(3);
        
        
        // Speed test
        
        
        uint8_t level = (uint8_t)(PIND & _BV(PD5));
        
        // set pin 12 as output
        DDRD  |= _BV(PD6);
        PORTD &= ~_BV(PD6); // off
        
        volatile uint8_t *portPtr = &PORTD;
        uint8_t  pinMask = _BV(PD6);

        while (1)
        {
            // Mark beginning
            PAL.DigitalWrite(12, HIGH);
            PAL.DelayMicroseconds(50);
            PAL.DigitalWrite(12, LOW);
            
            PAL.DelayMicroseconds(20);

            
            
            
            // Read a digital value the slow way
            PORTD |= _BV(PD6);  // on
            uint8_t value1 = PAL.DigitalRead(pin_);
            PORTD &= ~_BV(PD6);  // off
            
            PAL.DelayMicroseconds(20);
            
            
            // Read a digital value the Pin prototype way
            PORTD |= _BV(PD6);  // on
            PAL.DigitalRead(p1);
            PORTD &= ~_BV(PD6);  // off
            
            PAL.DelayMicroseconds(20);
            
            
            
            // Read a digital value the fast way
            PORTD |= _BV(PD6);  // on
            uint8_t value2 = PORTD & _BV(PD5);
            PORTD &= ~_BV(PD6);  // off
            
            PAL.DelayMicroseconds(20);


            
            
            
            
            // Write a digital value the slow way
            PAL.DigitalWrite(12, HIGH);
            PAL.DigitalWrite(12, LOW);
            
            PAL.DelayMicroseconds(20);
            
            
            // Write a digital value the Pin prototype way
            PAL.DigitialWrite(p1, 1);
            PAL.DigitialWrite(p1, 0);
            
            PAL.DelayMicroseconds(20);


            
            // Write Indirect
            *portPtr |= pinMask;  // on
            *portPtr &= ~pinMask; // off
            
            PAL.DelayMicroseconds(20);
            


            // Write Direct
            PORTD |= _BV(PD6);  // on
            PORTD &= ~_BV(PD6); // off
            
            PAL.DelayMicroseconds(20);
            
            
            
            
            
            // Calculate time diff slow
            PORTD |= _BV(PD6);  // on
            uint32_t timeStart = PAL.Micros();
            uint32_t timeEnd   = PAL.Micros();
            uint32_t timeDiff  = timeEnd - timeStart;
            PORTD &= ~_BV(PD6); // off
            
            PAL.DelayMicroseconds(20);
            
            
            
            
            
            // Calculate time diff prototype
            PORTD |= _BV(PD6);  // on
            uint8_t timeStartProto = (uint8_t)PAL.Micros();
            uint8_t timeEndProto   = (uint8_t)PAL.Micros();
            uint8_t timeDiffProto  = timeEndProto - timeStartProto;
            PORTD &= ~_BV(PD6); // off
            
            PAL.DelayMicroseconds(20);





            // Mark end
            PAL.DigitalWrite(12, HIGH);
            PAL.DelayMicroseconds(50);
            PAL.DigitalWrite(12, LOW);
            
            PAL.DelayMicroseconds(20);
        }
        
        
        
        
        return 1;
    }
    */

    struct Measurement
    {
        uint8_t temperature;
    };
    
    Measurement GetMeasurement()
    {
        Measurement m = GetDefaultMeasurement();
        
        
        
        // Step 1 - Request data from sensor:
        // (from spec for DHT22)
        // - Default state of line should be HIGH
        // - Bring LOW for at least 500us or 18ms (which?)
        // - Bring HIGH for at least 18us
        
        PAL.PinMode(pin_, OUTPUT);
        
        PAL.DigitalWrite(pin_, LOW);
        PAL.Delay(1);
        
        // Restore pin to INPUT_PULLUP in order to:
        // - Allow sensor to now adjust the level
        // - Be able to read from the pin
        // - Set initial state for next time
        
        PAL.DigitalWrite(pin_, HIGH);
        PAL.DelayMicroseconds(20);
        
        PAL.PinMode(pin_, INPUT_PULLUP);
        
        
        // Debug
        PAL.PinMode(D1, OUTPUT);
        PAL.DigitalWrite(D1, HIGH);
        PAL.DigitalWrite(D1, LOW);
        
        PAL.PinMode(D2, OUTPUT);
        PAL.DigitalWrite(D2, HIGH);
        PAL.DigitalWrite(D2, LOW);

        
        uint8_t cont = 1;
        
        // Try to lock on to bit stream
        // 3 transitions -- HIGH to LOW, LOW to HIGH, HIGH to LOW
        for (uint8_t i = 0; i < 3 && cont; ++i)
        {
            cont = WaitForLevelChange();
        }
        
        if (cont)
        {
            uint8_t byteArray[5] = { 0 };
            
            // Now read bit stream
            for (uint8_t i = 0; i < 40 && cont; ++i)
            {
                uint8_t bitValue;
                
                static const uint16_t DURATION_US_BIT_ZERO_CEILING = 65;
                
                // Skip low period
                if (WaitForLevelChange())
                {
                    // Observe high period duration
                    if (CalculateBitValueByPulseHighDuration(&bitValue,
                                                             DURATION_US_BIT_ZERO_CEILING))
                    {
                        uint8_t byteArrayIdx = i >> 3;
                        
                        // Shift everything in the accumulator byte left, which
                        // defaults the lowest bit to zero.
                        byteArray[byteArrayIdx] <<= 1;
                        
                        // Conditionally set the lowest bit to 1
                        if (bitValue)
                        {
                            byteArray[byteArrayIdx] |= 0x01;
                        }
                    }
                    else
                    {
                        cont = 0;
                    }
                }
                else
                {
                    cont = 0;
                }
            }
            
            if (cont)
            {
                
                // Read all the bits and reproduce the signal for visual inspection
                for (uint8_t i = 0; i < 40; ++i)
                {
                    PAL.DigitalWrite(D1, HIGH);
                    
            uint8_t byteListIdxTmp = (i >> 3);
            uint8_t bitMaskTmp     = (0x80 >> (i - (byteListIdxTmp << 3)));
            
            uint8_t bitVal = (byteArray[byteListIdxTmp] & bitMaskTmp) ? 1 : 0;
                    
                    
                    if (bitVal)
                    {
                        PAL.DelayMicroseconds(60);
                    }
                    else
                    {
                        PAL.DelayMicroseconds(12);
                    }
                    
                    PAL.DigitalWrite(D1, LOW);
                    
                    PAL.DelayMicroseconds(55);
                }
                
                
                
                // Let's check the checksum
                uint8_t total = byteArray[0] + byteArray[1] + byteArray[2] + byteArray[3];
                
                
                
                // first, does this checksum math make sense?
                
                PAL.DigitalWrite(D2, HIGH);
                
                PAL.DigitalWrite(D1, HIGH);
                PAL.DigitalWrite(D1, LOW);
                
                if (byteArray[4] == total)
                {
                    PAL.DigitalWrite(D1, HIGH);
                    PAL.DigitalWrite(D1, LOW);
                }
                
                PAL.DigitalWrite(D2, LOW);
                
            }
            else
            {
                // Couldn't read all the bits
            }
        }
        else
        {
            // Didn't lock on to bit stream
        }
        
        return m;
    }

private:
    Measurement GetDefaultMeasurement()
    {
        Measurement m;
        
        m.temperature = 0;
        
        return m;
    }
    
    uint8_t WaitForLevelChange()
    {
        uint8_t level        = PAL.DigitalRead(pin_);
        uint8_t levelChanged = 0;
        
        uint8_t cont = 1;
        
        uint16_t loopCount      = 0;
        uint16_t loopCountLimit = 10000;
        
        while (cont)
        {
            if (level != PAL.DigitalRead(pin_))
            {
                cont = 0;
                
                levelChanged = 1;
            }
            else
            {
                ++loopCount;
                
                if (loopCount >= loopCountLimit)
                {
                    cont = 0;
                    
                    PAL.DigitalWrite(D2, HIGH);
                    PAL.DigitalWrite(D2, LOW);
                }
            }
        }
        
        return levelChanged;
    }
    
    uint8_t CalculateBitValueByPulseHighDuration(uint8_t  *bitValue,
                                                 uint16_t  bitZeroCeilingDuration)
    {
        uint8_t retVal = 0;
        
        uint16_t timeStart = (uint16_t)PAL.Micros();
        
        if (WaitForLevelChange())
        {
            retVal = 1;
            
            uint16_t timeEnd  = (uint16_t)PAL.Micros();
            uint16_t timeDiff = timeEnd - timeStart;
            
            if (timeDiff <= bitZeroCeilingDuration)
            {
                *bitValue = 0;
            }
            else
            {
                *bitValue = 1;
            }
        }
        
        
        // PAL.DigitalWrite(D1, HIGH);
        // PAL.DigitalWrite(D1, LOW);
        
        /*
        if (*bitValue)
        {
            PAL.DigitalWrite(D1, HIGH);
            PAL.DigitalWrite(D1, LOW);
        }
        */
        
        
        return retVal;
    }


    Pin pin_;
    Pin D1;
    Pin D2;
};



#endif  // __SENSOR_TEMPERATURE_DHTXX_H__

