#ifndef __SHIFT_REGISTER_OUTPUT_H__
#define __SHIFT_REGISTER_OUTPUT_H__


#include "ShiftRegisterOut.h"


class ShiftRegisterOutputIface
{
public:
    virtual void DigitalWrite(uint8_t pinLogical, uint8_t val);
};


class ShiftRegisterOutputPin
{
    template <uint8_t REGISTER_COUNT>
    friend class ShiftRegisterOutput;
    
private:
    
    ShiftRegisterOutputPin(ShiftRegisterOutputIface *sroi, uint8_t pinLogical)
    : sroi_(sroi)
    , pinLogical_(pinLogical)
    {
        // Nothing to do
    }

    
public:

    ShiftRegisterOutputPin()
    : sroi_(NULL)
    , pinLogical_(0)
    {
        // Nothing to do
    }

    void DigitalWrite(uint8_t val)
    {
        if (sroi_)
        {
            sroi_->DigitalWrite(pinLogical_, val ? HIGH : LOW);
        }
    }

private:

    ShiftRegisterOutputIface *sroi_;
    
    uint8_t pinLogical_;
};
    

template <uint8_t REGISTER_COUNT>
class ShiftRegisterOutput
: private ShiftRegisterOutputIface
{
    friend class ShiftRegisterOutputPin;
    
    static const uint8_t BUF_SIZE = REGISTER_COUNT ? REGISTER_COUNT : 1;

public:

    ShiftRegisterOutput(ShiftRegisterOut &sro)
    : sro_(sro)
    {
        memset(buf, 0, BUF_SIZE);
        
        SendBuf();
    }
    
    ShiftRegisterOutputPin GetPin(uint8_t pinLogical)
    {
        return ShiftRegisterOutputPin{this, pinLogical};
    }

    virtual void DigitalWrite(uint8_t pinLogical, uint8_t val)
    {
        uint8_t bufIdx = pinLogical / 8;
        uint8_t bit    = pinLogical % 8;
        
        if (val)
        {
            buf[bufIdx] |= _BV(bit);
        }
        else
        {
            buf[bufIdx] &= ~_BV(bit);
        }
        
        SendBuf();
    }

    
private:

    void SendBuf()
    {
        sro_.ShiftOut(buf, BUF_SIZE);
    }

    ShiftRegisterOut &sro_;

    uint8_t buf[BUF_SIZE];
};





#endif  // __SHIFT_REGISTER_OUTPUT_H__



