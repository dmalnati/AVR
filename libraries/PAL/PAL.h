#ifndef __PAL_H__
#define __PAL_H__


#include <avr/boot.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/atomic.h>

#include "Function.h"

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
    TIMEOUT_15_MS = 0,
    TIMEOUT_30_MS,
    TIMEOUT_60_MS,
    TIMEOUT_120_MS,
    TIMEOUT_250_MS,
    TIMEOUT_500_MS,
    TIMEOUT_1000_MS,
    TIMEOUT_2000_MS,
    TIMEOUT_4000_MS,
    TIMEOUT_8000_MS,
};


// Necessary to allow Watchdog to be disabled after soft reboot.
void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));

class PlatformAbstractionLayer
{
public:

#include "PinPrivate.h"

    
public:
    using CbFnRaw = void (*)(); 

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
    
    // 5000 (5 seconds) seen emperically as enough time to level out after
    // delayed rise and sagging on startup.
    static inline uint16_t ReadVccMillivoltsMaxInTime(uint32_t msDuration = 5000)
    {
        uint16_t retVal = ReadVccMillivolts();
        
        uint32_t timeNow = Millis();
        while ((Millis() - timeNow) < msDuration)
        {
            uint16_t tmp = ReadVccMillivolts();
            
            if (tmp > retVal)
            {
                retVal = tmp;
            }
        }
        
        return retVal;
    }

    // Return a unit-less 10-bit value represesnting measured voltage against
    // the AVcc reference.
    //
    // Unit-less because AVcc isn't known at any given time.
    static inline uint16_t AnalogRead(Pin pin)
    {
        return AnalogReadInternal(pin.adcChannelBits_);
    }
    
    // Return a 10-bit value represesnting measured voltage against
    // the 1.1V internal reference.
    //
    // The step size is known in advance due to it being 10-bit resolution from
    // 0V-1.1V.
    //
    // Nothing is done with that information here.
    static inline uint16_t AnalogRead1V1(Pin pin)
    {
        // I know AnalogReadInternal is going to set REFS0 to 1, which if no
        // other changes, will reference AVcc.
        //
        // However, I want to use the internal 1.1V reference, which
        // conveniently requires only that REFS1 is set also.
        //
        // Since I also know that AnalogReadInternal logically ORs the passed-in
        // value with REFS0, I know I can piggy-back the REFS1 into this
        // parameter and cause the 1.1V reference to be selected, unbeknownst to
        // AnalogReadInternal.
        
        return AnalogReadInternal(_BV(REFS1) | pin.adcChannelBits_);
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
    
    
    uint8_t DigitalWatch(Pin       pin,
                         uint8_t   levelToWatch,
                         uint32_t  timeoutUs,
                         uint32_t *measuredDurationUs = NULL)
    {
        uint8_t retVal = 0;
        
        // Confirm we're on the expected initial level
        uint8_t  logicLevel = DigitalRead(pin);
        uint32_t timeStart  = Micros();
        
        if (logicLevel == levelToWatch)
        {
            // Hold this current level
            uint32_t timeElapsed = 0;
            
            while (timeElapsed < timeoutUs && logicLevel == levelToWatch)
            {
                timeElapsed = Micros() - timeStart;
                
                logicLevel = DigitalRead(pin);
            }
            
            // Confirm that we haven't timed out in order to indicate success
            if (timeElapsed < timeoutUs)
            {
                retVal = 1;
                
                if (measuredDurationUs)
                {
                    *measuredDurationUs = timeElapsed;
                }
            }
        }
        else
        {
            // failed preconditions, quit
        }
        
        return retVal;
    }
    
    
    // Expected use:
    // - Decoding bits in pulse-width-encoded signal
    // - If you want to measure, say, HIGH peaks
    //   - you're already in a LOW, you want to wait to go HIGH, then
    //     measure the time there before going LOW again
    //       - there is a giveup time on waiting for the first transition
    // - Works for both HIGH and LOW peaks
    //
    // Specify the HIGH_LOW_VALUE as HIGH if you want to measure the HIGH duration
    uint8_t DigitalChangeWatch(Pin      pin,
                               uint8_t  levelToWatchFor,
                               uint32_t timeoutCurrentLevelUs,
                               uint32_t timeoutReqdLevelUs,
                               uint32_t *measuredDurationUs = NULL)
    {
        uint8_t retVal = 0;
        
        // Check we are at the expected initial level, and that we transition
        // to the next level within our giveup time
        if (DigitalWatch(pin, !levelToWatchFor, timeoutCurrentLevelUs))
        {
            if (DigitalWatch(pin, levelToWatchFor, timeoutReqdLevelUs, measuredDurationUs))
            {
                retVal = 1;
            }
        }
        
        return retVal;
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
    
    static void DelayUnderWatchdog(uint32_t ms, uint32_t kickEveryMs = 1000)
    {
        uint32_t msRemaining = ms;
        
        WatchdogReset();
        
        while (msRemaining)
        {
            uint32_t delayMs = msRemaining > kickEveryMs ? kickEveryMs : msRemaining;
            
            Delay(delayMs);
            
            WatchdogReset();
            
            msRemaining -= delayMs;
        }
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
    
    constexpr
    static void BitReverse(uint8_t *buf, uint8_t bufLen)
    {
        for (uint8_t i = 0; i < bufLen; ++i)
        {
            buf[i] = BitReverse(buf[i]);
        }
    }
    
    constexpr
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
    
    constexpr
    static void Swap2(uint8_t *buf)
    {
        uint8_t b = buf[0];
        
        buf[0] = buf[1];
        buf[1] = b;
    }
    
    constexpr
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
    constexpr
    static inline uint16_t htons(uint16_t val)
    {
        uint16_t retVal = val;
        
        Swap2((uint8_t *)&retVal);
        
        return retVal;
    }
    
    constexpr
    static inline uint16_t ntohs(uint16_t val)
    {
        return htons(val);
    }
    
    constexpr
    static inline uint32_t htonl(uint32_t val)
    {
        uint32_t retVal = val;
        
        Swap4((uint8_t *)&retVal);
        
        return retVal;
    }
    
    constexpr
    static inline uint32_t ntohl(uint32_t val)
    {
        return htonl(val);
    }
    
    static uint8_t GetCpuPrescalerValue()
    {
        return (uint8_t)(1 << (uint8_t)(CLKPR & 0x0F));
    }
    
    constexpr
    static uint32_t GetOscillatorFreq()
    {
        return F_CPU;
    }

    static uint32_t GetCpuFreq()
    {
        return GetOscillatorFreq() / GetCpuPrescalerValue();
    }
    
    static void WatchdogEnableInterrupt(WatchdogTimeout wt)
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            uint8_t wdtcsrNew = (_BV(WDIE)) |
                !!((uint8_t)wt & 0b0001000) << WDP3 |
                !!((uint8_t)wt & 0b0000100) << WDP2 |
                !!((uint8_t)wt & 0b0000010) << WDP1 |
                !!((uint8_t)wt & 0b0000001) << WDP0;

            WDTCSR = _BV(WDCE) | _BV(WDE);
            WDTCSR = wdtcsrNew;
        }
        
        WatchdogReset();
    }
    
    static void SetInterruptHandlerWDT(function<void()> cbFn)
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            UnSetInterruptHandlerRawWDT();
            
            cbFnWDT_ = cbFn;
        }
    }
    
    static void UnSetInterruptHandlerWDT()
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            cbFnWDT_ = [](){};
        }
    }
    
    static void SetInterruptHandlerRawWDT(CbFnRaw cbFnRaw)
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            UnSetInterruptHandlerWDT();
            
            cbFnRawWDT_ = cbFnRaw;
        }
    }
    
    static void UnSetInterruptHandlerRawWDT()
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            UnSetInterruptHandlerWDT();
            
            cbFnRawWDT_ = OnFnRawWDTDefault;
        }
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
    

    constexpr
    static int8_t
    GetArduinoPinFromPhysicalPin(uint8_t physicalPin)
    {
        int8_t retVal = -1;
        
        /* Physical | Arduino | Alias */

        if      (physicalPin ==  0) { retVal = -1; }   /* DNE  */
        else if (physicalPin ==  1) { retVal = -1; }
        else if (physicalPin ==  2) { retVal =  0; }
        else if (physicalPin ==  3) { retVal =  1; }
        else if (physicalPin ==  4) { retVal =  2; }
        else if (physicalPin ==  5) { retVal =  3; }
        else if (physicalPin ==  6) { retVal =  4; }
        else if (physicalPin ==  7) { retVal = -1; }   /* VCC  */
        else if (physicalPin ==  8) { retVal = -1; }   /* GND  */
        else if (physicalPin ==  9) { retVal = -1; }
        else if (physicalPin == 10) { retVal = -1; }
        else if (physicalPin == 11) { retVal =  5; }
        else if (physicalPin == 12) { retVal =  6; }
        else if (physicalPin == 13) { retVal =  7; }
        else if (physicalPin == 14) { retVal =  8; }
        else if (physicalPin == 15) { retVal =  9; }
        else if (physicalPin == 16) { retVal = 10; }   /* SS   */
        else if (physicalPin == 17) { retVal = 11; }   /* MOSI */
        else if (physicalPin == 18) { retVal = 12; }   /* MISO */
        else if (physicalPin == 19) { retVal = 13; }   /* SCK  */
        else if (physicalPin == 20) { retVal = -1; }   /* AVCC */
        else if (physicalPin == 21) { retVal = -1; }   /* AREF */
        else if (physicalPin == 22) { retVal = -1; }   /* GND  */
        else if (physicalPin == 23) { retVal = 14; }   /* A0   */
        else if (physicalPin == 24) { retVal = 15; }   /* A1   */
        else if (physicalPin == 25) { retVal = 16; }   /* A2   */
        else if (physicalPin == 26) { retVal = 17; }   /* A3   */
        else if (physicalPin == 27) { retVal = 18; }   /* A4   */ /* SDA */
        else if (physicalPin == 28) { retVal = 19; }   /* A5   */ /* SDL */
        
        return retVal;
    }
    
    // Return 1 for successfully found, 0 for failure
    // 'port' is really an opaque handle, not the actual port.
    // same with portPin.
    constexpr
    static uint8_t
    GetPortAndPortPinFromPhysicalPin(uint8_t  physicalPin,
                                     uint8_t *port,
                                     uint8_t *portPin)
    {
        uint8_t retVal = 1;
        
        if      (physicalPin ==  0) { retVal = 0;                       }
        else if (physicalPin ==  1) { *port = PORT_C; *portPin = PINC6; }
        else if (physicalPin ==  2) { *port = PORT_D; *portPin = PIND0; }
        else if (physicalPin ==  3) { *port = PORT_D; *portPin = PIND1; }
        else if (physicalPin ==  4) { *port = PORT_D; *portPin = PIND2; }
        else if (physicalPin ==  5) { *port = PORT_D; *portPin = PIND3; }
        else if (physicalPin ==  6) { *port = PORT_D; *portPin = PIND4; }
        else if (physicalPin ==  7) { retVal = 0;                       }
        else if (physicalPin ==  8) { retVal = 0;                       }
        else if (physicalPin ==  9) { *port = PORT_B; *portPin = PINB6; }
        else if (physicalPin == 10) { *port = PORT_B; *portPin = PINB7; }
        else if (physicalPin == 11) { *port = PORT_D; *portPin = PIND5; }
        else if (physicalPin == 12) { *port = PORT_D; *portPin = PIND6; }
        else if (physicalPin == 13) { *port = PORT_D; *portPin = PIND7; }
        else if (physicalPin == 14) { *port = PORT_B; *portPin = PINB0; }
        else if (physicalPin == 15) { *port = PORT_B; *portPin = PINB1; }
        else if (physicalPin == 16) { *port = PORT_B; *portPin = PINB2; }
        else if (physicalPin == 17) { *port = PORT_B; *portPin = PINB3; }
        else if (physicalPin == 18) { *port = PORT_B; *portPin = PINB4; }
        else if (physicalPin == 19) { *port = PORT_B; *portPin = PINB5; }
        else if (physicalPin == 20) { retVal = 0;                       }
        else if (physicalPin == 21) { retVal = 0;                       }
        else if (physicalPin == 22) { retVal = 0;                       }
        else if (physicalPin == 23) { *port = PORT_C; *portPin = PINC0; }
        else if (physicalPin == 24) { *port = PORT_C; *portPin = PINC1; }
        else if (physicalPin == 25) { *port = PORT_C; *portPin = PINC2; }
        else if (physicalPin == 26) { *port = PORT_C; *portPin = PINC3; }
        else if (physicalPin == 27) { *port = PORT_C; *portPin = PINC4; }
        else if (physicalPin == 28) { *port = PORT_C; *portPin = PINC5; }
        else                        { retVal = 0;                       }
        
        return retVal;
    }

                                                    
    static void PowerDownSerial0() { PRR |= _BV(PRUSART0);           }
    static void PowerUpSerial0()   { PRR &= (uint8_t)~_BV(PRUSART0); }
    
    static void PowerDownTimer0() { PRR |= _BV(PRTIM0);           }
    static void PowerUpTimer0()   { PRR &= (uint8_t)~_BV(PRTIM0); }
    static void PowerDownTimer1() { PRR |= _BV(PRTIM1);           }
    static void PowerUpTimer1()   { PRR &= (uint8_t)~_BV(PRTIM1); }
    static void PowerDownTimer2() { PRR |= _BV(PRTIM2);           }
    static void PowerUpTimer2()   { PRR &= (uint8_t)~_BV(PRTIM2); }
    
    static void PowerDownADC()    { ADCSRA &= (uint8_t)~_BV(ADEN); }
    static void PowerUpADC()      { ADCSRA |= _BV(ADEN);           }
    
    
    ////////////////////////////////////////////////////////////////////////////
    //
    // Fuses
    //
    ////////////////////////////////////////////////////////////////////////////
    
    /*
     * For standard fuses, this is what you get from functions:
     *
     * Fuse High: 0b11011011
     * Fuse Low : 0b11100010
     * Fuse Ext : 0b11111101
     * Fuse Lock: 0b11111111
     *
     * IE the confusion about setting fuses using inverted logic isn't present
     * in the actual reading of the bits.
     *
     * If the datasheet says BODLEVEL 2:0 fuses 0b101 is 2.7v, you'll see 0b101
     *
     */
    
    static uint8_t GetFuseHigh()
    {
        return boot_lock_fuse_bits_get(GET_HIGH_FUSE_BITS);
    }
    
    static uint8_t GetFuseLow()
    {
        return boot_lock_fuse_bits_get(GET_LOW_FUSE_BITS);
    }
    
    static uint8_t GetFuseExtended()
    {
        return boot_lock_fuse_bits_get(GET_EXTENDED_FUSE_BITS);
    }
    
    static uint8_t GetFuseLock()
    {
        return boot_lock_fuse_bits_get(GET_LOCK_BITS);
    }
    
    // retVal of 0 either means disabled, or reserved/invalid
    static uint16_t GetFuseBODLimMilliVolts()
    {
        uint16_t retVal = 0;
        
        uint8_t bodLevel = GetFuseExtended() & 0b111;
        
             if (bodLevel == 0b110) { retVal = 1800; }
        else if (bodLevel == 0b101) { retVal = 2700; }
        else if (bodLevel == 0b100) { retVal = 4300; }
        
        return retVal;
    }
    
    
    ////////////////////////////////////////////////////////////////////////////
    //
    // Low Power Sleep functionality
    //
    ////////////////////////////////////////////////////////////////////////////
    
    struct WdtData
    {
        WatchdogTimeout wt;
        uint16_t        ms;
        uint8_t         calibrated;
    };

private:

    static WdtData wdtDataArr_[10];
    static volatile uint8_t watchdogTimeoutCalibrationTimeoutFlag_;
    
    static void WatchdogTimeoutCalibrationTimeout()
    {
        watchdogTimeoutCalibrationTimeoutFlag_ = 1;
    }
    
    static void CalibrateWdtTimeout(WdtData *wdtDataPtr)
    {
        // Catch interrupt from watchdog with custom function
        SetInterruptHandlerRawWDT(WatchdogTimeoutCalibrationTimeout);
        
        // See what time we're starting
        uint32_t timeStart = Millis();
        
        // Reset flag to indicate whether watchdog has gone off
        watchdogTimeoutCalibrationTimeoutFlag_ = 0;
        
        // Start watchdog and let it expire
        WatchdogEnableInterrupt(wdtDataPtr->wt);
        while (!watchdogTimeoutCalibrationTimeoutFlag_) { }
        
        // Calculate duration
        uint32_t timeEnd = Millis();
        uint16_t timeDiff = (uint16_t)(timeEnd - timeStart);
        
        // Cache result and indicate that the calibration took place
        wdtDataPtr->ms         = timeDiff;
        wdtDataPtr->calibrated = 1;
    }
    

public:
    
    static void DelayLowPower(uint32_t delaySleepDurationMs)
    {
        if (delaySleepDurationMs)
        {
            // keep track of remaining duration to sleep
            uint32_t remainingMs = delaySleepDurationMs;
            
            SetInterruptHandlerRawWDT(Wake);

            while (remainingMs)
            {
                // Calculate sleep step size
                
                // default to first element
                WdtData *wdtDataPtr = &(wdtDataArr_[0]);
                
                // but check if time is greater than each subsequent longer
                // duration, such that we can pick the largest timeout possible
                for (auto &wdtData : wdtDataArr_)
                {
                    if (remainingMs >= wdtData.ms)
                    {
                        wdtDataPtr = &wdtData;
                    }
                }
                
                // Actually delay
                if (!wdtDataPtr->calibrated)
                {
                    // We haven't slept for this duration before.
                    // We know these times are very variable between devices, so
                    // don't actually go to low-power sleep, and instead take
                    // the opportunity to measure duration associated with the
                    // watchdog timer, and cache the result.
                    // We'll do low-power sleep next time.
                    CalibrateWdtTimeout(wdtDataPtr);
                }
                else
                {
                    // Here we know the prior-measured duration is as good as
                    // we're going to get.  Use it.
                    WatchdogEnableInterrupt(wdtDataPtr->wt);
        
                    DeepSleep();
                    
                    WatchdogDisable();
                }
                
                // Calculate how much more sleep necessary
                if (remainingMs >= wdtDataPtr->ms)
                {
                    remainingMs -= wdtDataPtr->ms;
                }
                else
                {
                    remainingMs = 0;
                }
            }
            
            // Adjust the Arduino variable holding the current time in
            // milliseconds.
            // This keeps the clock as accurate as it can be
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
            {
                extern unsigned long timer0_millis;
                timer0_millis += delaySleepDurationMs;
            }
        }
    }
    
    static void DeepSleep()
    {
        PowerDownADC();
        PowerDownBODDuringSleep();  // Timing sensitive, must be immediately before sleep
        SleepModePowerDown();
    }

    static void PowerDownBODDuringSleep()
    {
        /*
        * The BODS bit must be written to logic one in order to turn off BOD
        * during sleep, see Table 10-1 on page 39.
        * Writing to the BODS bit is controlled by a timed sequence and an
        * enable bit, BODSE in MCUCR. To disable BOD in relevant sleep modes,
        * both BODS and BODSE must first be set to one. Then, to set the BODS
        * bit, BODS must be set to one and BODSE must be set to zero within four
        * clock cycles. The BODS bit is active three clock cycles after it is
        * set. A sleep instruction must be executed while BODS is active in
        * order to turn off the BOD for the actual sleep mode. The BODS bit is
        * automatically cleared after three clock cycles.
        */
       
       // Short story -- you have to call this function immediately before
       // sleeping for it to work.
        
        //MCUCR = (_BV(BODS | _BV(BODSE)));
        //MCUCR = _BV(BODS);
        
        // Had to skip the above bit-twiddling to get the timing right
        // on 8MHz
        
        MCUCR = 0b01100000;
        MCUCR = 0b01000000;
    }
    
    static void SleepModePowerDown()
    {
        Sleep(SleepMode::POWER_DOWN);
    }
    
    enum class SleepMode : uint8_t
    {
        IDLE = 0,
        ADC_NOISE_REDUCTION,
        POWER_DOWN,
        POWER_SAVE,
        STANDBY,
        EXTENDED_STANDBY,
    };
    
    static void Sleep(SleepMode sleepMode)
    {
        // No need to clear power save register to ensure known state, the
        // assigned values from the enum take up the entire bit width of useful
        // configuration bits.
        
        SMCR = ((uint8_t)(sleepMode) << 1) | 0b00000001;
        
        sleep_cpu();
        
        /*
         * If an enabled interrupt occurs while the MCU is in a sleep mode, the
         * MCU wakes up. The MCU is then halted for four cycles in addition to
         * the start-up time, executes the interrupt routine, and resumes
         * execution from the instruction following SLEEP.
         * The contents of the Register File and SRAM are unaltered when the
         * device wakes up from sleep. If a reset occurs during sleep mode, the
         * MCU wakes up and executes from the Reset Vector.
         */
        
        // So basically the function returns from here as if nothing happened.
    }
    
    static void Wake()
    {
        sleep_disable();
    }
    
    
private:
    static function<void()> cbFnWDT_;
public:     // would be private but need the ISR to be able to call this
    static CbFnRaw          cbFnRawWDT_;
private:
    static void OnFnRawWDTDefault() { cbFnWDT_(); }

    static volatile uint8_t *port__ddrxPtr[3];
    static volatile uint8_t *port__pinxPtr[3];
    static volatile uint8_t *port__portxPtr[3];
    
    uint8_t mcusrCache_;
};




// Make the global instance known
extern PlatformAbstractionLayer PAL;

using Pin = PlatformAbstractionLayer::Pin;



#endif  // __PAL_H__
