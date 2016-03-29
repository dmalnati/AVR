#ifndef __SENSOR_TEMPERATURE_DHTXX_H__
#define __SENSOR_TEMPERATURE_DHTXX_H__


#include "PAL.h"


// Debug
#include "Utl.h"


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
        
        //ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
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

            

            // observed
            const uint32_t TIMEOUT_US_WAIT_FIRST_HIGH_TO_LOW_STATE = 8000;
            if (WaitForLevelChange(TIMEOUT_US_WAIT_FIRST_HIGH_TO_LOW_STATE))
            {
                // Signal should now be low.  And should stay that way for
                // around 80us.
                
                const uint8_t TIMEOUT_US_WAIT_FIRST_LOW_TO_HIGH_STATE = 100;
                if (WaitForLevelChange(TIMEOUT_US_WAIT_FIRST_LOW_TO_HIGH_STATE) &&
                    WaitForLevelChange(TIMEOUT_US_WAIT_FIRST_LOW_TO_HIGH_STATE))
                {
                    // 40 bits follow.
                    // Each bit is a two-level signal.
                    // First part is LOW for a constant time of around ~55us.
                    // Second part is HIGH for a variable time of:
                    // - ~24us for a 0
                    // - ~72us for a 1
                    
                    uint8_t cont = 1;
                    for (uint8_t i = 0; i < 40 && cont; ++i)
                    {
                        const uint8_t TIMEOUT_US_WAIT_LOW_PART_OF_BIT = 75;
                        if (WaitForLevelChange(TIMEOUT_US_WAIT_LOW_PART_OF_BIT))
                        {
                            
                            const uint8_t DURATION_US_BIT_ZERO_CEILING = 50;
                            const uint8_t TIMEOUT_US_BIT_VALUE_READING = 100;
                            
                            uint8_t bitValue = 0;
                            
                            if (CalculateBitValueByPulseHighDuration(&bitValue,
                                                                     DURATION_US_BIT_ZERO_CEILING,
                                                                     TIMEOUT_US_BIT_VALUE_READING))
                            {
                                // ...
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
                        // calculate
                    }
                    else
                    {
                        // error
                    }
                    
                }
            }
        }
        
        
        // Step 2 - Read data from sensor
        
        return m;
    }

private:
    Measurement GetDefaultMeasurement()
    {
        Measurement m;
        
        m.temperature = 0;
        
        return m;
    }
    
    uint8_t WaitForLevelChange(uint32_t timeoutUs)
    {
        // PAL.DigitalWrite(D1, HIGH);
        // PAL.DigitalWrite(D1, LOW);

        
        uint8_t level        = PAL.DigitalRead(pin_);
        uint8_t levelChanged = 0;
        
        uint8_t cont = 1;
        
        uint32_t loopCount = 0;
        uint32_t loopCountLimit = microsecondsToClockCycles(timeoutUs);
        loopCountLimit = 5000;
        loopCountLimit = 2000;
        loopCountLimit = 10000;
        
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
            // else if ((PAL.Micros() - timeStart) > timeoutUs)
            // {
                // cont = 0;
                
                // PAL.DigitalWrite(D2, HIGH);
                // PAL.DigitalWrite(D2, LOW);
            // }
        }
        
        
        // PAL.DigitalWrite(D1, HIGH);
        // PAL.DigitalWrite(D1, LOW);

        
        return levelChanged;
    }
    
    uint8_t CalculateBitValueByPulseHighDuration(uint8_t  *bitValue,
                                                 uint32_t  bitZeroCeilingDuration,
                                                 uint32_t  timeoutUs)
    {
        uint8_t retVal = 0;
        
        uint32_t timeStart = PAL.Micros();
        
        if (WaitForLevelChange(timeoutUs))
        {
            retVal = 1;
            
            uint32_t timeEnd  = PAL.Micros();
            uint32_t timeDiff = timeEnd - timeStart;
            
            if (timeDiff <= bitZeroCeilingDuration)
            {
                *bitValue = 0;
            }
            else
            {
                *bitValue = 1;
            }
        }
        
        PAL.DigitalWrite(D1, HIGH);
        PAL.DigitalWrite(D1, LOW);
        
        if (*bitValue)
        {
            PAL.DigitalWrite(D1, HIGH);
            PAL.DigitalWrite(D1, LOW);
        }
        
        
        return retVal;
    }


    Pin pin_;
    Pin D1;
    Pin D2;
};



#endif  // __SENSOR_TEMPERATURE_DHTXX_H__

