#ifndef __PIN_H__
#define __PIN_H__


// Forward declaration
class PlatformAbstractionLayer;

class Pin
{
    friend class PlatformAbstractionLayer;
    
public:
    Pin(uint8_t pin);

private:
    uint8_t port_;
    uint8_t pinMask_;
};


#endif  // __PIN_H__