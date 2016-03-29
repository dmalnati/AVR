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

// Necessary to allow Watchdog to be disabled after soft reboot.
void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));

class PlatformAbstractionLayer
{
public:
    static const uint8_t PORT_B = 0;
    static const uint8_t PORT_C = 1;
    static const uint8_t PORT_D = 2;
    
    PlatformAbstractionLayer()
    : mcusrCache_(MCUSR)
    {
        // Clear reset flag register value now that copy has been taken
        MCUSR = 0;
        
        DisableWatchdogAfterSoftReset();
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
    
    static uint8_t ShiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder)
    {
        uint8_t arduinoDataPin  = GetArduinoPinFromPhysicalPin(dataPin);
        uint8_t arduinoClockPin = GetArduinoPinFromPhysicalPin(clockPin);
        
        return shiftIn(arduinoDataPin, arduinoClockPin, bitOrder);
    }
    
    // 8-bit AVRs are Little Endian
    static inline uint16_t htons(uint16_t val)
    {
        return (((val & 0x00FF) << 8) | ((val & 0xFF00) >> 8));
    }
    
    static inline uint16_t ntohs(uint16_t val)
    {
        return htons(val);
    }
    
    static void DisableWatchdogAfterSoftReset()
    {
        wdt_disable();
    }
    
    void SoftReset()
    {
        wdt_enable(WDTO_15MS);
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

private:
    static volatile uint8_t *port__ddrxPtr[3];
    static volatile uint8_t *port__pinxPtr[3];
    static volatile uint8_t *port__portxPtr[3];

    uint8_t mcusrCache_;
};




// Make the global instance known
extern PlatformAbstractionLayer PAL;




#endif  // __PAL_H__