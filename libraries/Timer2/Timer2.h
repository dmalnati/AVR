#ifndef __TIMER_2_H__
#define __TIMER_2_H__


#include "Timer.h"


class Timer2
{
    static const uint8_t PIN_CHANNEL_A = 17;
    static const uint8_t PIN_CHANNEL_B =  5;
    
public:
    Timer2()
    : timerPrescaler_(GetTimerPrescalerFromRegister())
    , channelA_  (&cbFnA_,   &TIMSK2, OCIE2A, &TIFR2, OCF2A, &OCR2A, &TCCR2A, COM2A1, COM2A0, PIN_CHANNEL_A)
    , channelB_  (&cbFnB_,   &TIMSK2, OCIE2B, &TIFR2, OCF2B, &OCR2B, &TCCR2A, COM2B1, COM2B0, PIN_CHANNEL_B)
    , ovfHandler_(&cbFnOvf_, &TIMSK2, TOIE2,  &TIFR2, TOV2)
    {
        // Nothing to do
    }
    
    ~Timer2()
    {
        // Nothing to do
    }
    
    uint16_t GetTimerPrescalerValue()
    {
        return ConvertTimerPrescalerToValue(timerPrescaler_);
    }
    
    enum class TimerPrescaler : uint8_t
    {
        DISABLE_TIMER = 0,
        DIV_BY_1,
        DIV_BY_8,
        DIV_BY_32,
        DIV_BY_64,
        DIV_BY_128,
        DIV_BY_256,
        DIV_BY_1024
    };
    
    uint16_t ConvertTimerPrescalerToValue(TimerPrescaler p)
    {
        uint16_t retVal = 0;
        
        switch (p)
        {
            case TimerPrescaler::DIV_BY_1:    retVal =    1; break;
            case TimerPrescaler::DIV_BY_8:    retVal =    8; break;
            case TimerPrescaler::DIV_BY_32:   retVal =   32; break;
            case TimerPrescaler::DIV_BY_64:   retVal =   64; break;
            case TimerPrescaler::DIV_BY_128:  retVal =  128; break;
            case TimerPrescaler::DIV_BY_256:  retVal =  256; break;
            case TimerPrescaler::DIV_BY_1024: retVal = 1024; break;
            
            default: break;
        }
        
        return retVal;
    }
    
    TimerPrescaler GetTimerPrescalerFromRegister()
    {
        TimerPrescaler retVal;
        
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            uint8_t cs22bit = TCCR2B & _BV(CS22) ? 1 : 0;
            uint8_t cs21bit = TCCR2B & _BV(CS21) ? 1 : 0;
            uint8_t cs20bit = TCCR2B & _BV(CS20) ? 1 : 0;
            
            retVal = (TimerPrescaler)(uint8_t)(cs22bit << CS22 |
                                               cs21bit << CS21 |
                                               cs20bit << CS20);
        }
        
        return retVal;
    }
    
    TimerPrescaler GetTimerPrescaler()
    {
        return timerPrescaler_;
    }
    
    void SetTimerPrescaler(TimerPrescaler p)
    {
        timerPrescaler_ = p;
    }
    
    void StopTimer()
    {
        // Intentionally directly set the register, leaving in place the
        // cached internal value of the prescaler.
        // That way StartTimer can be called later without needing to
        // re-set the prescaler value.
        TCCR2B &= 0xF8;
    }
    
    void StartTimer()
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            uint8_t cs22bit = (uint8_t)timerPrescaler_ & 0x04 ? 1 : 0;
            uint8_t cs21bit = (uint8_t)timerPrescaler_ & 0x02 ? 1 : 0;
            uint8_t cs20bit = (uint8_t)timerPrescaler_ & 0x01 ? 1 : 0;
            
            TCCR2B = (uint8_t)((uint8_t) (TCCR2B & 0xF8) | 
                               (uint8_t)((cs22bit << CS22) |
                                         (cs21bit << CS21) |
                                         (cs20bit << CS20)));
        }
    }
    
    enum class TimerMode : uint8_t
    {
        NORMAL = 0,
        PHASE_CORRECT_PWM,
        CTC_TOP_OCRNA,
        FAST_PWM,
        RESERVED_1,
        PHASE_CORRECT_PWM_TOP_OCRNA,
        RESERVED_2,
        FAST_PWM_TOP_OCRNA
    };
    
    void SetTimerMode(TimerMode m)
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            // Get integer value of TimerMode and extract bits from it
            uint8_t wgm22bit = (uint8_t)m & 0x04 ? 1 : 0;
            uint8_t wgm21bit = (uint8_t)m & 0x02 ? 1 : 0;
            uint8_t wgm20bit = (uint8_t)m & 0x01 ? 1 : 0;
            
            //         TCCR2A    TCCR2B
            //        76543210  76543210
            // WGM22                x   
            // WGM21        x            
            // WGM20         x          
            
            TCCR2B = (uint8_t)((uint8_t)(TCCR2B & (uint8_t)~_BV(WGM22)) |
                               (uint8_t)(wgm22bit << WGM22));
            TCCR2A = (uint8_t)((uint8_t)(TCCR2A & (uint8_t)~_BV(WGM21)) |
                               (uint8_t)(wgm21bit << WGM21));
            TCCR2A = (uint8_t)((uint8_t)(TCCR2A & (uint8_t)~_BV(WGM20)) |
                               (uint8_t)(wgm20bit << WGM20));
        }
    }
    
    TimerChannel *GetTimerChannelA()
    {
        return &channelA_;
    }
    
    TimerChannel *GetTimerChannelB()
    {
        return &channelB_;
    }
    
    TimerInterrupt *GetTimerOverflowHandler()
    {
        return &ovfHandler_;
    }
    
    uint8_t GetTimerValue()
    {
        return TCNT2;
    }
    
    void SetTimerValue(uint8_t value)
    {
        TCNT2 = value;
    }
    
    
    // Should be private but public for the sake of the ISR
    static function<void()> cbFnA_;
    static function<void()> cbFnB_;
    static function<void()> cbFnOvf_;
    
private:

    TimerPrescaler     timerPrescaler_;

    TimerChannel8Bit   channelA_;
    TimerChannel8Bit   channelB_;
    TimerInterrupt     ovfHandler_;
};



#endif  // __TIMER_2_H__




