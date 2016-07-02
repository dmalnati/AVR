#ifndef __SIGNAL_H__
#define __SIGNAL_H__


#include "Timer.h"


/*

Want to generate a sin wave.


Need to control:
- period of each complete sin wave (frequency)
- steps


Each step is a given duty cycle of a much higher resolution pwm.

Each period is divided by the number of steps taken to go completely
from bottom to top to bottom.

Need to be able to switch period without losing place in sin wave.


Initial application is to create Bell202 tones, which are:
- 1200Hz (833us period sin)
- 2200Hz (454us period sin)


Use like:
- Construct
- SetPeriod
- Start
- ChangePeriod n times
- Stop
- Destroy

If you Start after Stop, the sin wave starts over at the last-configured
period.


Implementation phases:
- 1. figure out step counts that might work
- 2. just get pwm working
- 3. output pwm and step coarsely
- 4. get timing between steps working


*/



class Signal
{
    static const uint16_t DEFAULT_PERIOD_USEC = 1000;
public:
    Signal()
    : Signal(DEFAULT_PERIOD_USEC)
    {
        // Nothing to do
    }
    
    Signal(uint16_t periodUsec)
    : periodUsec_(periodUsec)
    , timerChannelA_(timer_.GetTimerChannelA())
    , timerChannelB_(timer_.GetTimerChannelB())
    {
        // Nothing to do
    }
    
    ~Signal()
    {
        Stop();
    }
    
    uint8_t SetPeriod(uint16_t periodUsec)
    {
        uint8_t retVal = 0;
        
        Timer1::FastPWMTimerConfig cfg;
        
        Serial.print("SetPeriod to ");
        Serial.println(periodUsec);
        if (timer_.DetermineFastPWMTimerParams(periodUsec, &cfg))
        {
            Serial.println("It worked!");
            Serial.print("timerPrescalerValue: ");
            Serial.println(cfg.timerPrescalerValue);
            Serial.print("tickDurationUsec: ");
            Serial.println(cfg.tickDurationUsec);
            Serial.print("periodUsecCalculated: ");
            Serial.println(cfg.periodUsecCalculated);
            Serial.print("channelAValue: ");
            Serial.println(cfg.channelAValue);
            
            
            
            retVal = 1;
            
            timer_.SetTimerPrescaler(cfg.timerPrescaler);
            timer_.SetTimerMode(cfg.timerMode);
            
            timerChannelA_->SetValue(cfg.channelAValue);
            timerChannelA_->SetFastPWMModeBehavior(
                cfg.channelAFastPWMModeBehavior);
                
            timerChannelB_->SetValue(0);
            timerChannelB_->SetFastPWMModeBehavior(
                cfg.channelBFastPWMModeBehavior);
        }
        
        return retVal;
    }

    void Start()
    {
        timer_.SetTimerValue(0);
        timer_.StartTimer();
    }
    
    void DebugSetDutyCycle(uint8_t pct)
    {
        uint16_t value = (double)pct / 100.0 * timerChannelA_->GetValue();
        
        timerChannelB_->SetValue(value);
    }
    
    void ChangePeriod()
    {
        
    }
    
    void Stop()
    {
        timer_.StopTimer();
    }
    
    
    
    
    
    #if 0
    void SetPeriodThoughts(uint16_t periodUsec)
    {
        
        
        // We know this is 0, ..., 100
        uint8_t signalSourceDiscretePointsCount = 21;
        
        // In a single period, we're only going to touch on 0 and 100 once, so
        // subtract those two numbers from the second count of discrete points.
        uint8_t stepsPerPeriod =
            signalSourceDiscretePointsCount +
            signalSourceDiscretePointsCount -
            2;
        
        // This will equal 40 in our case
        
        // What does that mean for our frequencies?  How long is each step
        // in a given period?
        // - 1200Hz (833us period sin)
        //   - so 833 / 40 = 20.8 usec
        // - 2200Hz (454us period sin)
        //   - so 454 / 40 = 11.3 usec
        
        // This isn't going to work at that many samples.  Have to step
        // bigger steps.
        
        // What if it was just 4 steps (0, 35, 65, 100)
        // Then total up+down = 6
        // 1200Hz = 833us; 833 / 6 = 138us
        // 2200Hz = 454us; 454 / 6 =  75us
        
        // So, a single step in that case can be as low as 75us
        //
        // During that time, we have to achieve an actual PWM.
        // Let's say that the 35% which is 4+7=11 pulse durations is the longest.
        //
        // (75us / 11 steps = ~7us / pulse)
        //
        // If we wanted a given duty cycle:
        // - set just set that proportion of the
        
        // probably can know max sample count, and work downward based on
        // known timings (empirically found)
        
        // 35%
        uint8_t onOffCount[][2] = { { 4, 7 } };
        
        uint8_t onCount  = onOffCount[0][0];
        uint8_t offCount = onOffCount[0][1];
    }    
    #endif
    
    
    
    
    
    
    
    
    
    

private:
    uint16_t periodUsec_;
    
    Timer1        timer_;
    TimerChannel *timerChannelA_;
    TimerChannel *timerChannelB_;
};









#endif  // __SIGNAL_H__







