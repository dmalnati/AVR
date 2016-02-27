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
    {
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
        MCUSR = 0;
        wdt_disable();
    }
    
    void SoftReset()
    {
        wdt_enable(WDTO_15MS);
        for(;;) { }
    }

    static int8_t GetArduinoPinFromPhysicalPin(uint8_t physicalPin);
    
    
private:

};




// Make the global instance known
extern PlatformAbstractionLayer PAL;




#endif  // __PAL_H__