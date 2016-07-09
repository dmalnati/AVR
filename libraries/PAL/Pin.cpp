#include "PAL.h"
#include "Pin.h"

Pin::Pin(uint8_t pin)
: port_(0)
, pinMask_(0)
{
    uint8_t portPin;
    
    if (PAL.GetPortAndPortPinFromPhysicalPin(pin, &port_, &portPin))
    {
        pinMask_ = _BV(portPin);
    }
}

Pin::Pin(uint8_t pin, uint8_t enableOutputMode)
: Pin(pin)
{
    if (enableOutputMode)
    {
        PAL.PinMode(*this, OUTPUT);
        PAL.DigitalWrite(*this, LOW);
    }
}

