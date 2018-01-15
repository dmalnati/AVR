#ifndef __MODEM_ANALOG_DAC_H__
#define __MODEM_ANALOG_DAC_H__


#include "ModemAnalog.h"


class ModemAnalogDac
: public ModemAnalog
{
public:

    ModemAnalogDac()
    : ModemAnalog(OnInterrupt)
    {
        SetupOutputPins();
    }
    
private:

    virtual void StopInternal()
    {
        // Set output value to zero
        PORTD = 128;
    }

    static void SetupOutputPins()
    {
        // Port D, not including the Serial RX (so 7-bit)
        PAL.PinMode(2, OUTPUT);
        PAL.PinMode(3, OUTPUT);
        PAL.PinMode(4, OUTPUT);
        PAL.PinMode(5, OUTPUT);
        PAL.PinMode(6, OUTPUT);
        PAL.PinMode(11, OUTPUT);
        PAL.PinMode(12, OUTPUT);
        PAL.PinMode(13, OUTPUT);
    }
    
    static inline void OnInterrupt()
    {
        // Adjust to 0-255 range
        uint8_t val = 128 + (osc_.GetNextSample() / preEmph_);
        
        PORTD = val;
    }
};


#endif  // __MODEM_ANALOG_DAC_H__










