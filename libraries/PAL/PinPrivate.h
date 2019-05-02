#ifndef __PIN_PRIVATE_H__
#define __PIN_PRIVATE_H__


class Pin
{
    friend class PlatformAbstractionLayer;
    
public:

    constexpr
    Pin(uint8_t pin)
    : port_(0)
    , pinMask_(0)
    , adcChannelBits_(0x0F)
    {
        uint8_t portPin = 0;
        
        if (PlatformAbstractionLayer::GetPortAndPortPinFromPhysicalPin(pin, &port_, &portPin))
        {
            pinMask_ = _BV(portPin);
            
            // 23 is the lowest ADC input, and all remaining are indexed off of it
            if (pin >= 23)
            {
                adcChannelBits_ = (pin - 23);
            }
        }
    }

    Pin(uint8_t pin, uint8_t outputValue)
    : Pin(pin)
    {
        PlatformAbstractionLayer::PinMode(*this, OUTPUT);
        PlatformAbstractionLayer::DigitalWrite(*this, outputValue);
    }

    constexpr
    Pin() : Pin(0) { }

private:
    uint8_t port_;
    uint8_t pinMask_;
    uint8_t adcChannelBits_;
};


#endif  // __PIN_PRIVATE_H__