#ifndef __TIMER_H__
#define __TIMER_H__


#include <inttypes.h>
#include <avr/interrupt.h>


/*


Clocked Tone Switcher


- Output bitrate
- bit representation is a tone
  - the tone has to be a sine wave which carries on from where it was
    previously


It draws data, not fed data(?)
    maybe through callback function<>
    
    


*/



#include "PAL.h"
#include "Function.h"


class TimerChannel
{
public:
    TimerChannel(function<void()>  *cbFn,
                 volatile uint16_t *ocr,
                 volatile uint8_t  *comreg,
                          uint8_t   com1bitLoc,
                          uint8_t   com0bitLoc,
                 volatile uint8_t  *timsk,
                          uint8_t   ociebitLoc,
                 volatile uint8_t  *tifr,
                          uint8_t   ocfbitLoc,
                          uint8_t   pin)
    : cbFn_(cbFn)
    , ocr_(ocr)
    , comreg_(comreg)
    , com1bitLoc_(com1bitLoc)
    , com0bitLoc_(com0bitLoc)
    , timsk_(timsk)
    , ociebitLoc_(ociebitLoc)
    , tifr_(tifr)
    , ocfbitLoc_(ocfbitLoc)
    , pin_(pin)
    {
        // What state to be in?
        
        // Which parts should be atomic?
        
        // Set pin to output mode
        PAL.PinMode(pin_, OUTPUT);
        OutputLow();
    }
    
    ~TimerChannel() {}
    
    inline uint16_t GetValue()
    {
        return *ocr_;
    }
    
    inline void SetValue(uint16_t value)
    {
        *ocr_ = value;
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
    
    //////////////////////////////////////////////////////////////////////////
    //
    // Interrupt control
    //
    //////////////////////////////////////////////////////////////////////////
    
    void SetInterruptHandler(function<void()> cbFn)
    {
        *cbFn_ = cbFn;
    }
    
    void UnSetInterruptHandler()
    {
        *cbFn_ = [](){};
    }
    
    void RegisterForInterrupt()
    {
        // Clear flag
        *tifr_ &= (uint8_t)~_BV(ocfbitLoc_);
        
        // Register for interrupts
        *timsk_ |= _BV(ociebitLoc_);
    }
    
    void DeRegisterForInterrupt()
    {
        // Disable interrupts
        *timsk_ &= (uint8_t)~_BV(ociebitLoc_);
        
        // Clear flag
        *tifr_ &= (uint8_t)~_BV(ocfbitLoc_);
    }


private:
    void SetConfigurationBits(uint8_t cfg)
    {
        uint8_t com1bit = (uint8_t)cfg & 0x02 ? 1 : 0;
        uint8_t com0bit = (uint8_t)cfg & 0x01 ? 1 : 0;
        
        uint8_t mask = (uint8_t)~(_BV(com1bitLoc_) | _BV(com0bitLoc_));
        
        *comreg_ = (uint8_t)((uint8_t) (*comreg_ & mask) | 
                             (uint8_t)((com1bit << com1bitLoc_) |
                                       (com0bit << com0bitLoc_)));
    }
    
    
    function<void()> *cbFn_;

    volatile uint16_t *ocr_;
    volatile uint8_t  *comreg_;
             uint8_t   com1bitLoc_;
             uint8_t   com0bitLoc_;
    volatile uint8_t  *timsk_;
             uint8_t ociebitLoc_;
    volatile uint8_t  *tifr_;
             uint8_t   ocfbitLoc_;
             
    uint8_t pin_;
};



















class Timer1
{
    static const uint8_t PIN_CHANNEL_A = 15;
    static const uint8_t PIN_CHANNEL_B = 16;
    
public:
    Timer1()
    : timerPrescaler_(GetTimerPrescalerFromRegister())
    , channelA_(&cbFnA_, &OCR1A, &TCCR1A, COM1A1, COM1A0, &TIMSK1, OCIE1A, &TIFR1, OCF1A, PIN_CHANNEL_A)
    , channelB_(&cbFnB_, &OCR1B, &TCCR1A, COM1B1, COM1B0, &TIMSK1, OCIE1B, &TIFR1, OCF1B, PIN_CHANNEL_B)
    {
        // Care if more than one instance created of mostly-static class?
        
        // What state should this object be in at creation?
            // power down, configure, set up to not run and all regs cleared
            // and then power up again?
        
        // which parts should be atomic?
    }
    
    ~Timer1()
    {
        // ???
    }
    
    void PowerUpTimer()
    {
        // Turn off power saving
        PRR &= (uint8_t)~_BV(PRTIM1);
    }
    
    void PowerDownTimer()
    {
        // Turn on power saving
        PRR |= _BV(PRTIM1);
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
            case TimerPrescaler::DIV_BY_1:    retVal = 1;    break;
            case TimerPrescaler::DIV_BY_8:    retVal = 8;    break;
            case TimerPrescaler::DIV_BY_64:   retVal = 64;   break;
            case TimerPrescaler::DIV_BY_256:  retVal = 256;  break;
            case TimerPrescaler::DIV_BY_1024: retVal = 1024; break;
            
            default: break;
        }
        
        return retVal;
    }
    
    TimerPrescaler GetTimerPrescalerFromRegister()
    {
        uint8_t cs12bit = TCCR1B & _BV(CS12) ? 1 : 0;
        uint8_t cs11bit = TCCR1B & _BV(CS11) ? 1 : 0;
        uint8_t cs10bit = TCCR1B & _BV(CS10) ? 1 : 0;
        
        return (TimerPrescaler)(uint8_t)(cs12bit << CS12 |
                                         cs11bit << CS11 |
                                         cs10bit << CS10);
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
        uint8_t cs12bit = (uint8_t)timerPrescaler_ & 0x04 ? 1 : 0;
        uint8_t cs11bit = (uint8_t)timerPrescaler_ & 0x02 ? 1 : 0;
        uint8_t cs10bit = (uint8_t)timerPrescaler_ & 0x01 ? 1 : 0;
        
        TCCR1B = (uint8_t)((uint8_t) (TCCR1B & 0xF8) | 
                           (uint8_t)((cs12bit << CS12) |
                                     (cs11bit << CS11) |
                                     (cs10bit << CS10)));
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
    
    TimerChannel *GetTimerChannelA()
    {
        return &channelA_;
    }
    
    TimerChannel *GetTimerChannelB()
    {
        return &channelB_;
    }
    
    uint16_t GetTimerValue()
    {
        return TCNT1;
    }
    
    void SetTimerValue(uint16_t value)
    {
        TCNT1 = value;
    }
    
    
    
    struct FastPWMTimerConfig
    {
        // Actual configuration params
        TimerPrescaler                     timerPrescaler;
        TimerMode                          timerMode;
        uint16_t                           channelAValue;
        TimerChannel::FastPWMModeBehavior  channelAFastPWMModeBehavior;
        TimerChannel::FastPWMModeBehavior  channelBFastPWMModeBehavior;
        
        // Useful for debugging and logging
        uint32_t  periodUsecCalculated;
        uint16_t  timerPrescalerValue;
        double    tickDurationUsec;
    };
    
    // Get back configuration which will drive the timer at the specified
    // period.
    // It involves using channel A as the TOP.
    // Up to the user to decide to use B as duty cycle, set interrupt handlers,
    // etc.
    //
    // Return 1 if possible, 0 if not
    uint8_t DetermineFastPWMTimerParams(uint32_t            periodUsecRequired,
                                        FastPWMTimerConfig *cfg)
    {
        // How to calculate:
        // - Iterate prescalers until you find a period >= required
        // - Calculate duration of each tick
        // - Set TOP = period / tickDuration
        // - Duty cycle is then = pct * TOP (user has to do this)
        
        const uint32_t MAX_VALUES = 65536;
        
        TimerPrescaler prescalerList[] = {
            TimerPrescaler::DIV_BY_1,
            TimerPrescaler::DIV_BY_8,
            TimerPrescaler::DIV_BY_64,
            TimerPrescaler::DIV_BY_256,
            TimerPrescaler::DIV_BY_1024
        };
        uint8_t prescalerListLen = sizeof(prescalerList) /
                                   sizeof(TimerPrescaler);
        
        uint8_t  minimumPrescalerFound = 0;
        for (uint8_t i = 0;
             i < prescalerListLen && !minimumPrescalerFound;
             ++i)
        {
            TimerPrescaler prescaler      = prescalerList[i];
            uint16_t       prescalerValue = ConvertTimerPrescalerToValue(prescaler);
            
            uint32_t cpuFreq          = PAL.GetCpuFreq();
            double   ticksPerSec      = (double)cpuFreq /
                                        (double)prescalerValue;
            double   tickDurationUsec = 1000000.0 / ticksPerSec;
            uint32_t periodUsecMax    = (double)MAX_VALUES *
                                        tickDurationUsec;
            
            if (periodUsecMax >= periodUsecRequired)
            {
                minimumPrescalerFound = 1;
                
                // Fill out return config
                cfg->timerPrescaler      = prescaler;
                cfg->timerMode           = TimerMode::FAST_PWM_TOP_OCRNA;
                cfg->channelAValue       = ((double)periodUsecRequired /
                                            tickDurationUsec) - 1;
                // this is just for debug, should be NONE
                cfg->channelAFastPWMModeBehavior =
                    TimerChannel::FastPWMModeBehavior::SPECIAL_TOP_VALUE;
                cfg->channelBFastPWMModeBehavior =
                    TimerChannel::FastPWMModeBehavior::CLEAR;
                
                cfg->periodUsecCalculated = (cfg->channelAValue + 1) * 
                                             tickDurationUsec;
                cfg->timerPrescalerValue  = prescalerValue;
                cfg->tickDurationUsec     = tickDurationUsec;
            }
        }
        
        return minimumPrescalerFound;
    }
    
    // Caller still needs to set B channel value to represent duty cycle.
    void ApplyFastPWMTimerParams(FastPWMTimerConfig *cfg)
    {
        SetTimerPrescaler(cfg->timerPrescaler);
        SetTimerMode(cfg->timerMode);
        
        channelA_.SetValue(cfg->channelAValue);
        channelA_.SetFastPWMModeBehavior(cfg->channelAFastPWMModeBehavior);
        
        channelB_.SetFastPWMModeBehavior(cfg->channelBFastPWMModeBehavior);
    }
    
    
    

    
    
    
    static uint16_t GetOCRA()
    {
        return OCR1A;
    }
    
    static uint16_t GetOCRB()
    {
        return OCR1B;
    }
    
    static uint8_t GetTCCRA()
    {
        return TCCR1A;
    }
    
    static uint8_t GetTCCRB()
    {
        return TCCR1B;
    }
    
    
    static uint8_t GetWaveformGenerationMode()
    {
        //         TCCR1A    TCCR1B
        //        76543210  76543210
        // WGM13               x      
        // WGM12                x   
        // WGM11        x            
        // WGM10         x          
        
        uint8_t retVal = (uint8_t)(          (uint8_t)(TCCR1A & 0x02) |
                                   (uint8_t)((uint8_t)(TCCR1B & 0x18) >> 1));
                                   
        return retVal;
    }
    
    // Should be private but public for the sake of the ISR
    static function<void()> cbFnA_;
    static function<void()> cbFnB_;
    
private:

    TimerPrescaler timerPrescaler_;

    TimerChannel channelA_;
    TimerChannel channelB_;
};



#endif  // __TIMER_H__














