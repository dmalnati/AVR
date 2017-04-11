#include "PAL.h"
#include "Pin.h"

Pin::Pin(uint8_t pin)
: port_(0)
, pinMask_(0)
, adcChannelBits_(0x0F)
{
    uint8_t portPin;
    
    if (PAL.GetPortAndPortPinFromPhysicalPin(pin, &port_, &portPin))
    {
        pinMask_ = _BV(portPin);
        
        // 23 is the lowest ADC input, and all remaining are indexed off of it
        if (pin >= 23)
        {
            adcChannelBits_ = (pin - 23);
            
            Serial.print("new Pin: ");
            Serial.println(pin);
            Serial.println(adcChannelBits_, 2);
        }
    }
}

Pin::Pin(uint8_t pin, uint8_t outputValue)
: Pin(pin)
{
    PAL.PinMode(*this, OUTPUT);
    PAL.DigitalWrite(*this, outputValue);
}

