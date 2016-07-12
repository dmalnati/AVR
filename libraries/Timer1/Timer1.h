#ifndef __TIMER_1_H__
#define __TIMER_1_H__


#include "Timer.h"


class Timer1
{
    static const uint8_t PIN_CHANNEL_A = 15;
    static const uint8_t PIN_CHANNEL_B = 16;
    
public:
    Timer1()
    : timerPrescaler_(GetTimerPrescalerFromRegister())
    , channelA_  (&cbFnA_,   &TIMSK1, OCIE1A, &TIFR1, OCF1A, &OCR1A, &TCCR1A, COM1A1, COM1A0, PIN_CHANNEL_A)
    , channelB_  (&cbFnB_,   &TIMSK1, OCIE1B, &TIFR1, OCF1B, &OCR1B, &TCCR1A, COM1B1, COM1B0, PIN_CHANNEL_B)
    , ovfHandler_(&cbFnOvf_, &TIMSK1, TOIE1,  &TIFR1, TOV1)
    {
        // Nothing to do
    }
    
    ~Timer1()
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
        DIV_BY_64,
        DIV_BY_256,
        DIV_BY_1024,
        EXTERNAL_CLOCK_FALLING_EDGE,
        EXTERNAL_CLOCK_RISING_EDGE
    };
    
    uint16_t ConvertTimerPrescalerToValue(TimerPrescaler p)
    {
        uint16_t retVal = 0;
        
        switch (p)
        {
            case TimerPrescaler::DIV_BY_1:    retVal =    1; break;
            case TimerPrescaler::DIV_BY_8:    retVal =    8; break;
            case TimerPrescaler::DIV_BY_64:   retVal =   64; break;
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
            uint8_t cs12bit = TCCR1B & _BV(CS12) ? 1 : 0;
            uint8_t cs11bit = TCCR1B & _BV(CS11) ? 1 : 0;
            uint8_t cs10bit = TCCR1B & _BV(CS10) ? 1 : 0;
            
            retVal = (TimerPrescaler)(uint8_t)(cs12bit << CS12 |
                                               cs11bit << CS11 |
                                               cs10bit << CS10);
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
        TCCR1B &= 0xF8;
    }
    
    void StartTimer()
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            uint8_t cs12bit = (uint8_t)timerPrescaler_ & 0x04 ? 1 : 0;
            uint8_t cs11bit = (uint8_t)timerPrescaler_ & 0x02 ? 1 : 0;
            uint8_t cs10bit = (uint8_t)timerPrescaler_ & 0x01 ? 1 : 0;
            
            TCCR1B = (uint8_t)((uint8_t) (TCCR1B & 0xF8) | 
                               (uint8_t)((cs12bit << CS12) |
                                         (cs11bit << CS11) |
                                         (cs10bit << CS10)));
        }
    }
    
    enum class TimerMode : uint8_t
    {
        NORMAL = 0,
        PHASE_CORRECT_PWM_8_BIT,
        PHASE_CORRECT_PWM_9_BIT,
        PHASE_CORRECT_PWM_10_BIT,
        CTC_TOP_OCRNA,
        FAST_PWM_8_BIT,
        FAST_PWM_9_BIT,
        FAST_PWM_10_BIT,
        PHASE_AND_FREQUENCY_CORRECT_PWM_TOP_ICRN,
        PHASE_AND_FREQUENCY_CORRECT_PWM_TOP_OCRNA,
        PHASE_CORRECT_PWM_TOP_ICRN,
        PHASE_CORRECT_PWM_TOP_OCRNA,
        CTC_TOP_ICRN,
        RESERVED,
        FAST_PWM_TOP_ICRN,
        FAST_PWM_TOP_OCRNA
    };
    
    void SetTimerMode(TimerMode m)
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            // Get integer value of TimerMode and extract bits from it
            uint8_t wgm13bit = (uint8_t)m & 0x08 ? 1 : 0;
            uint8_t wgm12bit = (uint8_t)m & 0x04 ? 1 : 0;
            uint8_t wgm11bit = (uint8_t)m & 0x02 ? 1 : 0;
            uint8_t wgm10bit = (uint8_t)m & 0x01 ? 1 : 0;
            
            //         TCCR1A    TCCR1B
            //        76543210  76543210
            // WGM13               x      
            // WGM12                x   
            // WGM11        x            
            // WGM10         x          
            
            TCCR1B = (uint8_t)((uint8_t)(TCCR1B & (uint8_t)~_BV(WGM13)) |
                               (uint8_t)(wgm13bit << WGM13));
            TCCR1B = (uint8_t)((uint8_t)(TCCR1B & (uint8_t)~_BV(WGM12)) |
                               (uint8_t)(wgm12bit << WGM12));
            TCCR1A = (uint8_t)((uint8_t)(TCCR1A & (uint8_t)~_BV(WGM11)) |
                               (uint8_t)(wgm11bit << WGM11));
            TCCR1A = (uint8_t)((uint8_t)(TCCR1A & (uint8_t)~_BV(WGM10)) |
                               (uint8_t)(wgm10bit << WGM10));
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
    
    uint16_t GetTimerValue()
    {
        uint16_t retVal;
        
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            retVal = TCNT1;
        }

        return retVal;
    }
    
    void SetTimerValue(uint16_t value)
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            TCNT1 = value;
        }
    }
    
    
    // Should be private but public for the sake of the ISR
    static function<void()> cbFnA_;
    static function<void()> cbFnB_;
    static function<void()> cbFnOvf_;
    
private:

    TimerPrescaler      timerPrescaler_;

    TimerChannel16Bit   channelA_;
    TimerChannel16Bit   channelB_;
    TimerInterrupt      ovfHandler_;
};



#endif  // __TIMER_1_H__




