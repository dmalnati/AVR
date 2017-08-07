#ifndef __SHIFT_REGISTER_INPUT_SINGLE_CALLBACK_H__
#define __SHIFT_REGISTER_INPUT_SINGLE_CALLBACK_H__


#include "Function.h"
#include "TimedEventHandler.h"
#include "ShiftRegisterIn.h"


template <uint8_t REGISTER_COUNT>
class ShiftRegisterInputSingleCallback
{
    static const uint8_t BUF_SIZE               = REGISTER_COUNT ? REGISTER_COUNT : 1;
    static const uint8_t CB_FN_LIST_LEN         = BUF_SIZE * 8;
    static const uint8_t DEFAULT_POLL_PERIOD_MS = 20;
    
    using CbFn = function<void(uint8_t pinLogical, uint8_t logicLevel)>;
    
public:

    ShiftRegisterInputSingleCallback(ShiftRegisterIn &sr,
                                     uint32_t         pollPeriodMs = DEFAULT_POLL_PERIOD_MS)
    : sr_(sr)
    , pollPeriodMs_(pollPeriodMs)
    {
        memset(buf,     0, BUF_SIZE);
        memset(bufLast, 0, BUF_SIZE);
    }
    
    void SetCallback(CbFn cbFn)
    {
        cbFn_ = cbFn;
    }
    
    void Init(uint8_t forceRefresh = 0)
    {
        ted_.SetCallback([this](){
            OnPoll();
        });
        
        ted_.RegisterForTimedEventInterval(pollPeriodMs_);
        
        // Get initial state
        sr_.ShiftIn(bufLast, BUF_SIZE);
        
        DoScan(forceRefresh);
    }
    
    uint8_t DigitalReadCached(uint8_t pinLogical)
    {
        uint8_t retVal = 0;
        
        if (pinLogical <= ((REGISTER_COUNT * 8) - 1))
        {
            uint8_t bufIdx = pinLogical / 8;
            uint8_t bit    = pinLogical % 8;
            
            retVal = !!(buf[bufIdx] & _BV(bit));
        }
        
        return retVal;
    }


private:

    void OnPoll()
    {
        DoScan();
    }
    
    void DoScan(uint8_t forceRefresh = 0)
    {
        sr_.ShiftIn(buf, BUF_SIZE);
        
        for (uint16_t i = 0; i < BUF_SIZE; ++i)
        {
            uint8_t bitmapLast = bufLast[i];
            uint8_t bitmap     = buf[i];

            if (bitmap != bitmapLast || forceRefresh)
            {
                for (int8_t j = 7; j >= 0; --j)
                {
                    uint8_t bitVal     = !!(bitmap     & (1 << (7 - j)));
                    uint8_t bitValLast = !!(bitmapLast & (1 << (7 - j)));
                    
                    if (bitVal != bitValLast || forceRefresh)
                    {
                        uint8_t cbListIdx = (i * 8) + (7 - j);
                        
                        cbFn_(cbListIdx, bitVal);
                    }
                }
            }
        }
        
        memcpy(bufLast, buf, BUF_SIZE);
    }
    
    
    ShiftRegisterIn &sr_;

    TimedEventHandlerDelegate ted_;
    uint32_t                  pollPeriodMs_;
    
    uint8_t buf[BUF_SIZE];
    uint8_t bufLast[BUF_SIZE];
    
    CbFn cbFn_;
};



#endif  // __SHIFT_REGISTER_INPUT_SINGLE_CALLBACK_H__




