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
    TimerChannel(volatile uint16_t *ocr,
                 volatile uint8_t  *comreg,
                          uint8_t   com1bitLoc,
                          uint8_t   com0bitLoc,
                 volatile uint8_t  *timsk,
                          uint8_t   ociebitLoc,
                 volatile uint8_t  *tifr,
                          uint8_t   ocfbitLoc,
                          uint8_t   pin)
    : ocr_(ocr)
    , comreg_(comreg)
    , com1bitLoc_(com1bitLoc)
    , com0bitLoc_(com0bitLoc)
    , timsk_(timsk)
    , ociebitLoc_(ociebitLoc)
    , tifr_(tifr)
    , ocfbitLoc_(ocfbitLoc)
    {
        // What state to be in?
        
        // Which parts should be atomic?
        
        // Set pin to output mode
        PAL.PinMode(pin, OUTPUT);
        PAL.DigitalWrite(pin, LOW);
    }
    
    ~TimerChannel() {}
    
    uint16_t GetValue()
    {
        return *ocr_;
    }
    
    void SetValue(uint16_t value)
    {
        *ocr_ = value;
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
    
    void SetInterruptHandler(function<void()> cbFn)
    {
        cbFn_ = cbFn;
    }
    
    void UnSetInterruptHandler()
    {
        cbFn_ = [](){};
    }
    
    // Should be private but public for the sake of the ISR
    void OnISR()
    {
        cbFn_();
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
                                       
        Serial.println("Setting configuration");
        Serial.print("cfg: 0x");
        Serial.println(cfg, HEX);
        Serial.print("com1bit: ");
        Serial.println(com1bit);
        Serial.print("com0bit: ");
        Serial.println(com0bit);
        Serial.print("mask: 0x");
        Serial.println(mask, HEX);
    }
    

    volatile uint16_t *ocr_;
    volatile uint8_t  *comreg_;
             uint8_t   com1bitLoc_;
             uint8_t   com0bitLoc_;
    volatile uint8_t  *timsk_;
             uint8_t ociebitLoc_;
    volatile uint8_t  *tifr_;
             uint8_t   ocfbitLoc_;
             
    function<void()> cbFn_;
};



class Timer1
{
    static const uint8_t PIN_CHANNEL_A = 15;
    static const uint8_t PIN_CHANNEL_B = 16;
    
public:
    Timer1()
    : channelA_(&OCR1A, &TCCR1A, COM1A1, COM1A0, &TIMSK1, OCIE1A, &TIFR1, OCF1A, PIN_CHANNEL_A)
    , channelB_(&OCR1B, &TCCR1A, COM1B1, COM1B0, &TIMSK1, OCIE1B, &TIFR1, OCF1B, PIN_CHANNEL_B)
    {
        // Set up static members
        channelAPtr_ = &channelA_;
        channelBPtr_ = &channelB_;

        
        // Care if more than one instance created of mostly-static class?
        
        // What state should this object be in at creation?
        
        // which parts should be atomic?
    }
    
    ~Timer1()
    {
        // ???
    }
    
    static void PowerUpTimer()
    {
        // Turn off power saving
        PRR &= (uint8_t)~_BV(PRTIM1);
    }
    
    static void PowerDownTimer()
    {
        // Turn on power saving
        PRR |= _BV(PRTIM1);
    }
    
    static void StopTimer()
    {
        SetTimerPrescaler(TimerPrescaler::DISABLE_TIMER);
    }
    
    static uint16_t GetTimerPrescalerValue()
    {
        uint16_t retVal = 0;
        
        uint8_t cs12 = TCCR1B & _BV(CS12);
        uint8_t cs11 = TCCR1B & _BV(CS11);
        uint8_t cs10 = TCCR1B & _BV(CS10);
        
        // Unfortunately the scaling is not as simple as the CPU prescaler, so
        // bit shifting isn't sufficient.
        //
        // Any other logic is more convoluted than just enumerating the few
        // actual combinations.
        //
        // Look only for actual prescale values, everything else call zero
        if      (!cs12 && !cs11 &&  cs10)  retVal =    1;
        else if (!cs12 &&  cs11 && !cs10)  retVal =    8;
        else if (!cs12 &&  cs11 &&  cs10)  retVal =   64;
        else if ( cs12 && !cs11 && !cs10)  retVal =  256;
        else if ( cs12 && !cs11 &&  cs10)  retVal = 1024;
        
        return retVal;
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
    
    static void SetTimerPrescaler(TimerPrescaler p)
    {
        uint8_t cs12bit = (uint8_t)p & 0x04 ? 1 : 0;
        uint8_t cs11bit = (uint8_t)p & 0x02 ? 1 : 0;
        uint8_t cs10bit = (uint8_t)p & 0x01 ? 1 : 0;
        
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
    
    static void SetTimerMode(TimerMode m)
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
    
    static uint16_t GetTimerValue()
    {
        return TCNT1;
    }
    
    static void SetTimerValue(uint16_t value)
    {
        TCNT1 = value;
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
    static TimerChannel *channelAPtr_;
    static TimerChannel *channelBPtr_;
    
    
private:

    TimerChannel channelA_;
    TimerChannel channelB_;
};



#endif  // __TIMER_H__














