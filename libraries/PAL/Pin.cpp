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
