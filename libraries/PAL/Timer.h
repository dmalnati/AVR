#ifndef __TIMER_H__
#define __TIMER_H__


#include <inttypes.h>
#include <avr/interrupt.h>

#include "PAL.h"
#include "Function.h"


class TimerInterrupt
{
public:
    TimerInterrupt(function<void()>  *cbFn,
                   volatile uint8_t  *timsk,
                            uint8_t   ociebitLoc,
                   volatile uint8_t  *tifr,
                            uint8_t   ocfbitLoc)
    : cbFn_(cbFn)
    , timsk_(timsk)
    , ociebitLoc_(ociebitLoc)
    , tifr_(tifr)
    , ocfbitLoc_(ocfbitLoc)
    {
        // Nothing to do
    }
    
    virtual ~TimerInterrupt()
    {
        // Nothing to do
    }
    
    //////////////////////////////////////////////////////////////////////////
    //
    // Interrupt control
    //
    //////////////////////////////////////////////////////////////////////////
    
    void SetInterruptHandler(function<void()> cbFn)
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            *cbFn_ = cbFn;
        }
    }
    
    void UnSetInterruptHandler()
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            *cbFn_ = [](){};
        }
    }
    
    void RegisterForInterrupt()
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            // Clear flag
            *tifr_ &= (uint8_t)~_BV(ocfbitLoc_);
            
            // Register for interrupts
            *timsk_ |= _BV(ociebitLoc_);
        }
    }
    
    void DeRegisterForInterrupt()
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            // Disable interrupts
            *timsk_ &= (uint8_t)~_BV(ociebitLoc_);
            
            // Clear flag
            *tifr_ &= (uint8_t)~_BV(ocfbitLoc_);
        }
    }


private:

    function<void()>  *cbFn_;

    volatile uint8_t  *timsk_;
             uint8_t   ociebitLoc_;
    volatile uint8_t  *tifr_;
             uint8_t   ocfbitLoc_;
};



class TimerChannel
: public TimerInterrupt
{
public:
    TimerChannel(function<void()>  *cbFn,
                 volatile uint8_t  *timsk,
                          uint8_t   ociebitLoc,
                 volatile uint8_t  *tifr,
                          uint8_t   ocfbitLoc,
                 volatile uint8_t  *comreg,
                          uint8_t   com1bitLoc,
                          uint8_t   com0bitLoc,
                          uint8_t   pin)
    : TimerInterrupt(cbFn, timsk, ociebitLoc, tifr, ocfbitLoc)
    , comreg_(comreg)
    , com1bitLoc_(com1bitLoc)
    , com0bitLoc_(com0bitLoc)
    , pin_(pin, LOW)
    {
        // What state to be in?
        
        // Which parts should be atomic?
    }
    
    virtual ~TimerChannel() {}
    
    inline uint16_t GetValue()
    {
        uint16_t retVal = 0;
        
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            retVal = GetValueInternal();
        }
        
        return retVal;
    }
    
    inline void SetValue(uint16_t value)
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            SetValueInternal(value);
        }
    }
    
    //////////////////////////////////////////////////////////////////////////
    //
    // Output pin control modes
    //
    //////////////////////////////////////////////////////////////////////////
    
    void OutputHigh()
    {
        PAL.DigitalWrite(pin_, HIGH);
    }
    
    void OutputLow()
    {
        PAL.DigitalWrite(pin_, LOW);
    }
    
    
    enum class CTCModeBehavior : uint8_t
    {
        NONE = 0,
        TOGGLE,
        CLEAR,
        SET
    };
    
    void SetCTCModeBehavior(CTCModeBehavior b)
    {
        SetConfigurationBits((uint8_t)b);
    }
    
    enum class FastPWMModeBehavior : uint8_t
    {
        NONE = 0,
        SPECIAL_TOP_VALUE,
        CLEAR,
        SET
    };
    
    void SetFastPWMModeBehavior(FastPWMModeBehavior b)
    {
        SetConfigurationBits((uint8_t)b);
    }
    
    enum class PhaseCorrectPWMModeBehavior : uint8_t
    {
        NONE = 0,
        SPECIAL_TOP_VALUE,
        CLEAR,
        SET
    };
    
    void SetPhaseCorrectPWMModeBehavior(PhaseCorrectPWMModeBehavior b)
    {
        SetConfigurationBits((uint8_t)b);
    }

protected:
    virtual uint16_t GetValueInternal()             = 0;
    virtual void     SetValueInternal(uint16_t val) = 0;

private:
    void SetConfigurationBits(uint8_t cfg)
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            uint8_t com1bit = (uint8_t)cfg & 0x02 ? 1 : 0;
            uint8_t com0bit = (uint8_t)cfg & 0x01 ? 1 : 0;
            
            uint8_t mask = (uint8_t)~(_BV(com1bitLoc_) | _BV(com0bitLoc_));
            
            *comreg_ = (uint8_t)((uint8_t) (*comreg_ & mask) | 
                                 (uint8_t)((com1bit << com1bitLoc_) |
                                           (com0bit << com0bitLoc_)));
        }
    }
    

    volatile uint8_t  *comreg_;
             uint8_t   com1bitLoc_;
             uint8_t   com0bitLoc_;
             
    Pin pin_;
};



class TimerChannel8Bit
: public TimerChannel
{
public:
    TimerChannel8Bit(function<void()>  *cbFn,
                     volatile uint8_t  *timsk,
                              uint8_t   ociebitLoc,
                     volatile uint8_t  *tifr,
                              uint8_t   ocfbitLoc,
                     volatile uint8_t  *ocr,
                     volatile uint8_t  *comreg,
                              uint8_t   com1bitLoc,
                              uint8_t   com0bitLoc,
                              uint8_t   pin)
    : TimerChannel(cbFn,
                   timsk,
                   ociebitLoc,
                   tifr,
                   ocfbitLoc,
                   comreg,
                   com1bitLoc,
                   com0bitLoc,
                   pin)
    , ocr_(ocr)
    {
        // Nothing to do
    }
    
    virtual ~TimerChannel8Bit() {}

private:
    inline virtual uint16_t GetValueInternal()
    {
        return (uint16_t)*ocr_;
    }
    
    inline virtual void SetValueInternal(uint16_t value)
    {
        *ocr_ = (uint8_t)value;
    }
    
    volatile uint8_t *ocr_;
};




class TimerChannel16Bit
: public TimerChannel
{
public:
    TimerChannel16Bit(function<void()>  *cbFn,
                      volatile uint8_t  *timsk,
                               uint8_t   ociebitLoc,
                      volatile uint8_t  *tifr,
                               uint8_t   ocfbitLoc,
                      volatile uint16_t *ocr,
                      volatile uint8_t  *comreg,
                               uint8_t   com1bitLoc,
                               uint8_t   com0bitLoc,
                               uint8_t   pin)
    : TimerChannel(cbFn,
                   timsk,
                   ociebitLoc,
                   tifr,
                   ocfbitLoc,
                   comreg,
                   com1bitLoc,
                   com0bitLoc,
                   pin)
    , ocr_(ocr)
    {
        // Nothing to do
    }
    
    virtual ~TimerChannel16Bit() {}

private:
    inline virtual uint16_t GetValueInternal()
    {
        return *ocr_;
    }
    
    inline virtual void SetValueInternal(uint16_t value)
    {
        *ocr_ = value;
    }
    
    volatile uint16_t *ocr_;
};






#endif  // __TIMER_H__














