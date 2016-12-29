#ifndef __TIMER_2_H__
#define __TIMER_2_H__


#include "Timer.h"


class Timer2
{
public:
    static const uint8_t PIN_CHANNEL_A = 17;
    static const uint8_t PIN_CHANNEL_B =  5;
    
    Timer2()
    {
        // Nothing to do
    }
    
    ~Timer2()
    {
        // Nothing to do
    }
    
    static uint16_t GetTimerPrescalerValue()
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
    
    static uint16_t ConvertTimerPrescalerToValue(TimerPrescaler p)
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
    
    static TimerPrescaler GetTimerPrescalerFromRegister()
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
    
    static TimerPrescaler GetTimerPrescaler()
    {
        return timerPrescaler_;
    }
    
    static void SetTimerPrescaler(TimerPrescaler p)
    {
        timerPrescaler_ = p;
    }
    
    static void StopTimer()
    {
        // Intentionally directly set the register, leaving in place the
        // cached internal value of the prescaler.
        // That way StartTimer can be called later without needing to
        // re-set the prescaler value.
        TCCR2B &= 0xF8;
    }
    
    static void StartTimer()
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
    
    static void SetTimerModeFastPWM8Bit()
    {
        SetTimerMode(TimerMode::FAST_PWM);
    }
    
    static void SetTimerMode(TimerMode m)
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

            uint8_t tccr2a;
            uint8_t tccr2b;
            
            tccr2a = (uint8_t)
                // Mask out the two bits
                (TCCR2A & (uint8_t)~(_BV(WGM21) | _BV(WGM20))) |
                // Plug in the two new bits
                (wgm21bit << WGM21 | wgm20bit << WGM20);
            
            tccr2b = (uint8_t)((uint8_t)(TCCR2B & (uint8_t)~_BV(WGM22)) |
                               (uint8_t)(wgm22bit << WGM22));
                
            TCCR2A = tccr2a;
            TCCR2B = tccr2b;
        }
    }
    
    constexpr
    static TimerChannel *GetTimerChannelA()
    {
        return &channelA_;
    }
    
    constexpr
    static TimerChannel *GetTimerChannelB()
    {
        return &channelB_;
    }
    
    constexpr
    static TimerInterrupt *GetTimerOverflowHandler()
    {
        return &ovfHandler_;
    }
    
    static uint8_t GetTimerValue()
    {
        return TCNT2;
    }
    
    static void SetTimerValue(uint8_t value)
    {
        TCNT2 = value;
    }
    
    
    // Should be private but public for the sake of the ISR
    static function<void()> cbFnA_;
    static function<void()> cbFnB_;
    static function<void()> cbFnOvf_;
    
    static TimerInterrupt::CbFnRaw cbFnRawA_;
    static TimerInterrupt::CbFnRaw cbFnRawB_;
    static TimerInterrupt::CbFnRaw cbFnRawOvf_;

    static void OnFnRawADefault()   { cbFnA_();   }
    static void OnFnRawBDefault()   { cbFnB_();   }
    static void OnFnRawOvfDefault() { cbFnOvf_(); }
    
    
private:

    static TimerPrescaler     timerPrescaler_;

    static TimerChannel8Bit   channelA_;
    static TimerChannel8Bit   channelB_;
    static TimerInterrupt     ovfHandler_;
};



#endif  // __TIMER_2_H__




