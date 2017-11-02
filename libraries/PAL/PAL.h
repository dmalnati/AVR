#ifndef __PAL_H__
#define __PAL_H__


#include <avr/wdt.h>
#include <util/atomic.h>

#include "Pin.h"

#include <Arduino.h>


/*
 * Objectives:
 * - Keep inputs in terms of the physical hardware.
 *   - This is the opposite of Arduino's objective
 * - Map back to Arduino only when necessary, such as:
 *   - Handing off arduino pins to 3rd party libs
 *   - Implementing wrapper around arduino libs which take in physical pin
 * - Hide any Arduino libs currently in use such that they can be changed
 *   across the board at a later date.
 *   
 */


enum class WatchdogTimeout : uint8_t
{
    TIMEOUT_15_MS   = WDTO_15MS,
    TIMEOUT_30_MS   = WDTO_30MS,
    TIMEOUT_60_MS   = WDTO_60MS,
    TIMEOUT_120_MS  = WDTO_120MS,
    TIMEOUT_250_MS  = WDTO_250MS,
    TIMEOUT_500_MS  = WDTO_500MS,
    TIMEOUT_1000_MS = WDTO_1S,
    TIMEOUT_2000_MS = WDTO_2S,
    TIMEOUT_4000_MS = WDTO_4S,
    TIMEOUT_8000_MS = WDTO_8S,
};


// Necessary to allow Watchdog to be disabled after soft reboot.
void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));

class PlatformAbstractionLayer
{
public:
    static const uint8_t PORT_B = 0;
    static const uint8_t PORT_C = 1;
    static const uint8_t PORT_D = 2;
    
    static const uint16_t ANALOG_MAX_VALUE = 1024;  // 10-bit
    
    PlatformAbstractionLayer()
    : mcusrCache_(MCUSR)
    {
        // Clear reset flag register value now that copy has been taken
        MCUSR = 0;
        
        WatchdogDisable();
    }
    
    static void PinMode(Pin pin, uint8_t mode)
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            if (mode == INPUT)
            {
                // Set the DDRX register to input
                *port__ddrxPtr[pin.port_] &= (uint8_t)~pin.pinMask_;
                
                
                /*
                 * This was a copy of the functionality from Arduino.
                 *
                 * However, this line seems to screw up the Ivm processing
                 * and was seeming to cause lots of random interrupts.
                 * (Notably the Ivm processing only originally included the
                 *  line above, so the line below was actually a change to
                 *  the logic.)
                 *
                 * Didn't investigate a lot, but commenting this out sorted the
                 * issue for now.  Can look into it more deeply later.
                 *
                 */
                // Set PORTX bit to indicate non-PULLUP
                //*port__portxPtr[pin.port_] &= (uint8_t)~pin.pinMask_;
            }
            else if (mode == INPUT_PULLUP)
            {
                // Set the DDRX register to input
                *port__ddrxPtr[pin.port_] &= (uint8_t)~pin.pinMask_;
                
                // Set PORTX bit to indicate PULLUP
                *port__portxPtr[pin.port_] |= pin.pinMask_;
            }
            else // (mode == OUTPUT) // (or an error we handle with a default)
            {
                // Set the DDRX register to output
                *port__ddrxPtr[pin.port_] |= pin.pinMask_;
            }
        }
    }
    
    enum class ADCPrescaler : uint8_t
    {
        DIV_BY_2 = 0,
        DIV_BY_2_ALSO,
        DIV_BY_4,
        DIV_BY_8,
        DIV_BY_16,
        DIV_BY_32,
        DIV_BY_64,
        DIV_BY_128
    };
    
    static ADCPrescaler GetADCPrescaler()
    {
        return (ADCPrescaler)(ADCSRA & 0x07);
    }
    
    static void SetADCPrescaler(ADCPrescaler adcPrescaler)
    {
        ADCSRA = ((ADCSRA & 0xF8) | ((uint8_t)adcPrescaler & 0x07));
    }
    
    // Technique adapted from:
    // https://provideyourown.com/2012/secret-arduino-voltmeter-measure-battery-voltage/
    static inline uint16_t ReadVccMillivolts()
    {
        static const uint8_t ADC_CHANNEL_BITS_1_1V_INTERNAL_REF = 0b00001110;
        
        uint16_t retVal = AnalogReadInternal(ADC_CHANNEL_BITS_1_1V_INTERNAL_REF);
        
        retVal = 1125300L / retVal; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
        
        return retVal;
    }
    
    static inline uint16_t AnalogRead(Pin pin)
    {
        return AnalogReadInternal(pin.adcChannelBits_);
    }
    
private:

    static inline uint16_t AnalogReadInternal(uint8_t adcChannelBits)
    {
        // Ensure the voltage reference is fixed to AVcc.  Left adjust disabled.
        ADMUX = (_BV(REFS0) | adcChannelBits);
        
        // Decide what to do whether in batch mode or not1
        if (ADCSRA & _BV(ADATE))
        {
            // Batch mode

            // Wait for interrupt flag to indicate 
            while (!(ADCSRA & _BV(ADIF)))
            {
                // Nothing to do
            }
            
            // Set the "start conversion" bit and the "interrupt flag" bit.
            // This prompts the next reading
            ADCSRA |= (_BV(ADSC) | _BV(ADIF));
        }
        else
        {
            // Regular mode
            
            // Set the "start conversion" bit
            ADCSRA |= _BV(ADSC);
            
            // Wait for bit to clear, indicating read completed
            while (ADCSRA & _BV(ADSC))
            {
                // Nothing to do
            }
        }
        
        // Read out low and high bytes of conversion.
        // Start with low byte, which locks the high byte in place while
        // you read it, at which point it becomes unlocked.
        uint8_t low  = ADCL;
        uint8_t high = ADCH;
        
        uint16_t retVal = ((high << 8) | low);
        
        return retVal;
    }
    
public:
    
    // Between BatchBegin and BatchEnd, you can only read from a single channel
    static inline void AnalogReadBatchBegin()
    {
        // Start the conversion.
        // Also indicate that the ADC will auto-trigger (begin again)
        // after a reading.
        ADCSRA |= (_BV(ADSC) | _BV(ADATE));
    }
    
    static inline void AnalogReadBatchEnd()
    {
        // Turn off conversion and batch mode
        ADCSRA &= ~(_BV(ADSC) | _BV(ADATE));
    }
    
    static inline uint16_t AnalogMaxValue()
    {
        return ANALOG_MAX_VALUE;
    }
    
    

    static inline uint8_t DigitalRead(Pin pin)
    {
        return (*port__pinxPtr[pin.port_] & pin.pinMask_) ? 1 : 0;
    }

    static inline void DigitalWrite(Pin pin, uint8_t value)
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            if (value == HIGH)
            {
                *port__portxPtr[pin.port_] |= pin.pinMask_;
            }
            else
            {
                *port__portxPtr[pin.port_] &= (uint8_t)~pin.pinMask_;
            }
        }
    }
    
    static inline void DigitalToggle(Pin pin)
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            *port__pinxPtr[pin.port_] = pin.pinMask_;
        }
    }
    
    static uint8_t GetPortValueFromPhysicalPin(uint8_t physicalPin)
    {
        uint8_t retVal = 0;
        
        uint8_t port;
        uint8_t portPin;
        
        if (GetPortAndPortPinFromPhysicalPin(physicalPin, &port, &portPin))
        {
            retVal = *port__pinxPtr[port];
        }
        
        return retVal;
    }
    
    static void Delay(uint32_t ms)
    {
        delay(ms);
    }
    
    static uint32_t Millis()
    {
        return millis();
    }
    
    static uint32_t Micros()
    {
        return micros();
    }
    
    static void DelayMicroseconds(uint32_t delay)
    {
        return delayMicroseconds(delay);
    }
    
    static void BitReverse(uint8_t *buf, uint8_t bufLen)
    {
        for (uint8_t i = 0; i < bufLen; ++i)
        {
            buf[i] = BitReverse(buf[i]);
        }
    }
    
    static uint8_t BitReverse(uint8_t val)
    {
        // Get the byte as-is
        uint8_t b = val;
        
        // Get container for new byte
        uint8_t bNew = 0;
        
        // Transfer bits
        for (uint8_t i = 0; i < 8; ++i)
        {
            bNew <<= 1;
            
            bNew |= (uint8_t)(b & 0x01);
            
            b >>= 1;
        }
        
        return bNew;
    }
    
    static void Swap2(uint8_t *buf)
    {
        uint8_t b = buf[0];
        
        buf[0] = buf[1];
        buf[1] = b;
    }
    
    static void Swap4(uint8_t *buf)
    {
        uint8_t b = buf[0];
        
        buf[0] = buf[3];
        buf[3] = b;
        
        b = buf[1];
        
        buf[1] = buf[2];
        buf[2] = b;
    }
    
    // 8-bit AVRs are Little Endian
    static inline uint16_t htons(uint16_t val)
    {
        uint16_t retVal = val;
        
        Swap2((uint8_t *)&retVal);
        
        return retVal;
    }
    
    static inline uint16_t ntohs(uint16_t val)
    {
        return htons(val);
    }
    
    static inline uint32_t htonl(uint32_t val)
    {
        uint32_t retVal = val;
        
        Swap4((uint8_t *)&retVal);
        
        return retVal;
    }
    
    static inline uint32_t ntohl(uint32_t val)
    {
        return htonl(val);
    }
    
    static uint8_t GetCpuPrescalerValue()
    {
        return (uint8_t)(1 << (uint8_t)(CLKPR & 0x0F));
    }

    static uint32_t GetCpuFreq()
    {
        return (uint32_t)F_CPU / GetCpuPrescalerValue();
    }
    
    static void WatchdogEnable(WatchdogTimeout wt)
    {
        wdt_enable((uint8_t)wt);
    }
    
    static void WatchdogReset()
    {
        wdt_reset();
    }
    
    static void WatchdogDisable()
    {
        wdt_disable();
    }
    
    void SoftReset()
    {
        WatchdogEnable(WatchdogTimeout::TIMEOUT_15_MS);
        for(;;) { }
    }
    
    enum class StartupMode : uint8_t {
        UNKNOWN,
        NORMAL,
        RESET_WATCHDOG,
        RESET_BROWNOUT,
        RESET_EXTERNAL,
        RESET_POWER_ON
    };
    
    StartupMode GetStartupMode()
    {
        StartupMode retVal = StartupMode::UNKNOWN;
        
        if (mcusrCache_ == 0)
        {
            retVal = StartupMode::NORMAL;
        }
        else if (mcusrCache_ & _BV(WDRF))
        {
            retVal = StartupMode::RESET_WATCHDOG;
        }
        else if (mcusrCache_ & _BV(BORF))
        {
            retVal = StartupMode::RESET_BROWNOUT;
        }
        else if (mcusrCache_ & _BV(EXTRF))
        {
            retVal = StartupMode::RESET_EXTERNAL;
        }
        else if (mcusrCache_ & _BV(PORF))
        {
            retVal = StartupMode::RESET_POWER_ON;
        }
        
        return retVal;
    }
    

    static int8_t GetArduinoPinFromPhysicalPin(uint8_t physicalPin);
    
    static uint8_t GetPortAndPortPinFromPhysicalPin(uint8_t  physicalPin,
                                                    uint8_t *port,
                                                    uint8_t *portPin);
                                                    

    static void PowerUpTimer0()   { PRR &= (uint8_t)~_BV(PRTIM0); }
    static void PowerDownTimer0() { PRR |= _BV(PRTIM0);           }
    static void PowerUpTimer1()   { PRR &= (uint8_t)~_BV(PRTIM1); }
    static void PowerDownTimer1() { PRR |= _BV(PRTIM1);           }
    static void PowerUpTimer2()   { PRR &= (uint8_t)~_BV(PRTIM2); }
    static void PowerDownTimer2() { PRR |= _BV(PRTIM2);           }

private:
    static volatile uint8_t *port__ddrxPtr[3];
    static volatile uint8_t *port__pinxPtr[3];
    static volatile uint8_t *port__portxPtr[3];

    uint8_t mcusrCache_;
};




// Make the global instance known
extern PlatformAbstractionLayer PAL;




#endif  // __PAL_H__