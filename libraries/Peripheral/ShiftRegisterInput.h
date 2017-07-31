#ifndef __SHIFT_REGISTER_INPUT_H__
#define __SHIFT_REGISTER_INPUT_H__


#include "Function.h"
#include "TimedEventHandler.h"
#include "ShiftRegister.h"


template <uint8_t REGISTER_COUNT>
class ShiftRegisterInput
{
    static const uint8_t BUF_SIZE               = REGISTER_COUNT ? REGISTER_COUNT : 1;
    static const uint8_t CB_FN_LIST_LEN         = BUF_SIZE * 8;
    static const uint8_t DEFAULT_POLL_PERIOD_MS = 20;
    
    using CbFn = function<void(uint8_t logicLevel)>;
    
    struct Callback
    {
        uint8_t mode;
        uint8_t activeLevel;
        CbFn    cbFn;
    };
    
public:

    ShiftRegisterInput(ShiftRegister &sr,
                       uint32_t       pollPeriodMs = DEFAULT_POLL_PERIOD_MS,
                       CbFn           cbFnDefault  = [](uint8_t){})
    : sr_(sr)
    , pollPeriodMs_(pollPeriodMs)
    {
        memset(buf,     0, BUF_SIZE);
        memset(bufLast, 0, BUF_SIZE);
        
        for (uint16_t i = 0; i < CB_FN_LIST_LEN; ++i)
        {
            SetCallback(i, cbFnDefault, LEVEL_RISING_AND_FALLING, HIGH);
        }
    }
    
    void SetCallback(uint8_t                             pinLogical,
                     function<void(uint8_t logicLevel)>  cbFn,
                     uint8_t                             mode        = LEVEL_RISING,
                     uint8_t                             activeLevel = LOW)
    {
        uint8_t cbListIdx = ((pinLogical / 8) * 8) + (pinLogical % 8);
        
        Callback c;
        
        c.cbFn        = cbFn;
        c.mode        = mode;
        c.activeLevel = activeLevel;
        
        cbList_[cbListIdx] = c;
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

            if (bitmap != bitmapLast)
            {
                for (uint8_t j = 0; j < 8; ++j)
                {
                    uint8_t bitVal     = !!(bitmap     & (1 << (7 - j)));
                    uint8_t bitValLast = !!(bitmapLast & (1 << (7 - j)));
                    
                    if (bitVal != bitValLast || forceRefresh)
                    {
                        uint8_t cbListIdx = (i * 8) + j;
                        
                        OnPinChange(cbListIdx, bitVal);
                    }
                }
            }
        }
        
        memcpy(bufLast, buf, BUF_SIZE);
    }
    
    void OnPinChange(uint8_t cbListIdx, uint8_t bitVal)
    {
        Callback c = cbList_[cbListIdx];
        
        uint8_t invokeCallback = 0;
        uint8_t bitValLogical  = c.activeLevel == HIGH ? bitVal : !bitVal;
        
        if (c.mode == LEVEL_UNDEFINED || c.mode == LEVEL_RISING)
        {
            invokeCallback = bitValLogical;
        }
        else if (c.mode == LEVEL_FALLING)
        {
            invokeCallback = !bitValLogical;
        }
        else
        {
            invokeCallback = 1;
        }
        
        if (invokeCallback)
        {
            c.cbFn(bitValLogical);
        }
    }
    
    
    ShiftRegister &sr_;

    TimedEventHandlerDelegate ted_;
    uint32_t                  pollPeriodMs_;
    
    uint8_t buf[BUF_SIZE];
    uint8_t bufLast[BUF_SIZE];
    
    Callback cbList_[CB_FN_LIST_LEN];
};



#endif  // __SHIFT_REGISTER_INPUT_H__




