#ifndef __UTL_H__
#define __UTL_H__


#include "PAL.h"
#include "IdleTimeEventHandler.h"
#include "TimedEventHandler.h"
#include "IdleTimeHiResTimedEventHandler.h"




extern void PinToggle(uint8_t pin, uint16_t delayMs = 500);
extern uint32_t GetRandomInRange(uint32_t rangeLow, uint32_t rangeHigh);


class TimedPinToggler : public TimedEventHandler
{
public:
    TimedPinToggler(uint8_t pin) : pin_(pin)
    {
        PAL.PinMode(pin_, OUTPUT);
    }
    
    void OnTimedEvent()
    {
        PAL.DigitalWrite(pin_, HIGH);
        PAL.DigitalWrite(pin_, LOW);
    }

private:
    uint8_t pin_;
};


class TimedPinTogglerHiRes
: public IdleTimeHiResTimedEventHandler
{
public:
    TimedPinTogglerHiRes(uint8_t pin) : pin_(pin)
    {
        PAL.PinMode(pin_, OUTPUT);
    }
    
    void OnIdleTimeHiResTimedEvent()
    {
        PAL.DigitalWrite(pin_, HIGH);
        PAL.DigitalWrite(pin_, LOW);
    }
    
private:
    uint8_t pin_;
};


class IdlePinToggler : public IdleTimeEventHandler
{
public:
    IdlePinToggler(uint8_t pin) : pin_(pin)
    {
        PAL.PinMode(pin_, OUTPUT);
    }
    
    virtual void OnIdleTimeEvent()
    {
        PAL.DigitalWrite(pin_, HIGH);
        PAL.DigitalWrite(pin_, LOW);
    }
    
private:
    uint8_t pin_;
};



class DestructorToken
{
public:
    DestructorToken(uint8_t pin) : pin_(pin) { }
    ~DestructorToken()
    {
        PinToggle(pin_);
    }
    uint8_t pin_;
};


template <uint8_t BUF_SIZE>
class SerialBufReader
{
public:
    SerialBufReader(HardwareSerial &s)
    : s_(s)
    {
        // Nothing to do
    }
    
    char *GetBuf(uint8_t *bufSizeReturned = NULL)
    {
        char *retVal = NULL;
        
        uint8_t bytesAvailable = s_.available();

        if (bytesAvailable)
        {
            uint8_t bytesToRead = bytesAvailable;

            if (bytesToRead > BUF_SIZE)
            {
                bytesToRead = BUF_SIZE;
            }

            s_.readBytes(buf_, bytesToRead);
            
            uint8_t bytesInBuffer = bytesToRead;

            // terminate with NULL
            buf_[bytesInBuffer] = '\0';
            
            // Strip newline
            if (buf_[bytesInBuffer - 1] == '\n')
            {
                buf_[bytesInBuffer - 1] = '\0';
                
                --bytesInBuffer;
            }
            
            // return values
            if (bytesInBuffer)
            {
                retVal = buf_;
                
                if (bufSizeReturned)
                {
                    *bufSizeReturned = bytesInBuffer;
                }
            }
        }

        return retVal;
    }
    
private:
    char buf_[BUF_SIZE + 1];

    HardwareSerial &s_;
};


class SerialPrinter
{
public:
    SerialPrinter(HardwareSerial &s)
    : s_(s)
    {
        // Nothing to do
    }
    
    template <typename STRT, typename VALT>
    void Print(STRT str, VALT val)
    {
        s_.print(str);
        s_.print(": ");
        s_.println(val);
    }

private:
    HardwareSerial &s_;
};



#endif // __UTL_H__









