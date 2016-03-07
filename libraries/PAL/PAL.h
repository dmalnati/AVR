#ifndef __PAL_H__
#define __PAL_H__


#include <avr/wdt.h>

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
    PlatformAbstractionLayer()
    : mcusrCache_(MCUSR)
    {
        // Clear reset flag register value now that copy has been taken
        MCUSR = 0;
        
        DisableWatchdogAfterSoftReset();
    }

    void PinMode(uint8_t physicalPin, uint8_t mode)
    {
        uint8_t arduinoPin = GetArduinoPinFromPhysicalPin(physicalPin);
        
        pinMode(arduinoPin, mode);
    }
    
    uint8_t DigitalRead(uint8_t physicalPin)
    {
        uint8_t arduinoPin = GetArduinoPinFromPhysicalPin(physicalPin);
        
        return digitalRead(arduinoPin);
    }
    
    void DigitalWrite(uint8_t physicalPin, uint8_t value)
    {
        uint8_t arduinoPin = GetArduinoPinFromPhysicalPin(physicalPin);
        
        digitalWrite(arduinoPin, value);
    }
    
    void Delay(uint32_t ms)
    {
        delay(ms);
    }
    
    uint32_t Millis()
    {
        return millis();
    }
    
    uint32_t Micros()
    {
        return micros();
    }
    
    void DelayMicroseconds(uint32_t delay)
    {
        return delayMicroseconds(delay);
    }
    
    uint8_t ShiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder)
    {
        uint8_t arduinoDataPin  = GetArduinoPinFromPhysicalPin(dataPin);
        uint8_t arduinoClockPin = GetArduinoPinFromPhysicalPin(clockPin);
        
        return shiftIn(arduinoDataPin, arduinoClockPin, bitOrder);
    }
    
    void DisableWatchdogAfterSoftReset()
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
    
    
private:
    uint8_t mcusrCache_;
};




// Make the global instance known
extern PlatformAbstractionLayer PAL;




#endif  // __PAL_H__