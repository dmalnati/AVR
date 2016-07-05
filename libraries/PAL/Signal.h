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



// Assume all signal-related things are using an 8-bit timer in FastPWM mode.

/*
class SignalSource
{
public:
    SignalSource() {}
    virtual ~SignalSource() {}
    
    // Phase Offset in the range of 0 - 359
    // Phase Step   in the range of 0 - 359
    
    // SetPhaseOffset is an absolute movement of the cursor.  Good for reset.
    // SetPhaseStep dictates where the next sample comes from offset from the
    //              prior sample.
    
    virtual void    SetPhaseOffset(uint16_t phaseOffset) = 0;
    virtual void    SetPhaseStep(uint16_t phaseStep)     = 0;
    virtual uint8_t NextSample()                         = 0;
};
*/


#include <avr/pgmspace.h>


extern const uint8_t SINE_TABLE[] PROGMEM;


class SignalSourceSineWave
{
    static const uint16_t SAMPLE_COUNT = 512;
    
    static const uint16_t DEFAULT_PHASE_OFFSET = 0;
    static const uint16_t DEFAULT_PHASE_STEP   = 1;
    
    constexpr static const double SCALING_RATIO = (double)SAMPLE_COUNT / 360.0;
    
public:
    SignalSourceSineWave()
    {
        Reset(DEFAULT_PHASE_STEP, DEFAULT_PHASE_OFFSET);
    }
    
    // Usage:
    // - Reset()
    // - GetSample(), GetNextSampleReady(), ...
    // - ChangePhaseStep(), GetSample(), GetNextSampleReady(), ...
    //
    // Doing Reset(), ChangePhaseStep() won't work!
    
    
    inline void Reset(uint8_t phaseStep, uint16_t phaseOffset = 0)
    {
        // Set up so that when GetNextSampleReady is complete, the value of
        // idxCurrent is equal to the phaseOffset specified and the sample
        // there is ready to be read.
        idxStep_    = phaseStep * SCALING_RATIO;
        idxCurrent_ = (phaseOffset * SCALING_RATIO);
        idxCurrent_ = (idxCurrent_ - idxStep_) % SAMPLE_COUNT;
        
        // Acquire sample
        GetNextSampleReady();
    }
    
    inline void ChangePhaseStep(uint16_t phaseStep)
    {
        // Rewind a step
        idxCurrent_ = (idxCurrent_ - idxStep_) % SAMPLE_COUNT;
        
        // Prepare new step size
        idxStep_ = phaseStep * SCALING_RATIO;
        
        // Acquire sample
        GetNextSampleReady();
    }
    
    inline uint8_t GetSample()
    {
        return sample_;
    }
    
    inline void GetNextSampleReady()
    {
        idxCurrent_ = (idxCurrent_ + idxStep_) % SAMPLE_COUNT;
        
        uint16_t pgmByteLocation = (uint16_t)SINE_TABLE + idxCurrent_;
        
        sample_ = pgm_read_byte_near(pgmByteLocation);
    }
    
private:

    uint8_t sample_;
    
    uint16_t idxCurrent_;
    uint16_t idxStep_;
};











template <typename SignalSource>
class SignalDAC
{
    static const uint32_t DEFAULT_PERIOD_USEC = 1000000;
    
public:
    SignalDAC(SignalSource *ss)
    : ss_(ss)
    , timerChannelA_(timer_.GetTimerChannelA())
    , timerChannelB_(timer_.GetTimerChannelB())
    {
        // Set up default period configuration and apply
        SetPeriod(DEFAULT_PERIOD_USEC);
    }
    
    ~SignalDAC()
    {
        Stop();
    }
    
    
    
    // Opaque to the caller
    struct SignalDACPeriodConfig
    {
        // Actually useful config
        Timer1::FastPWMTimerConfig fastPWMTimerConfig;
        
        // Useful for debugging
        uint32_t periodUsecRequested;
    };
    
    uint8_t GetPeriodConfig(uint32_t periodUsec, SignalDACPeriodConfig *cfg)
    {
        uint8_t retVal = 0;
        
        // Keep track of requested period
        cfg->periodUsecRequested = periodUsec;
        
        uint32_t periodPerStepUsec = periodUsec / ss_->GetStepCount();
        
        Serial.print("GetPeriodConfig for ");
        Serial.println(periodUsec);
        Serial.print("Step count: ");
        Serial.println(ss_->GetStepCount());
        Serial.print("Period per-step: ");
        Serial.println(periodPerStepUsec);
        
        // Attempt to get timer configuration for this period
        if (timer_.DetermineFastPWMTimerParams(periodPerStepUsec,
                                               &cfg->fastPWMTimerConfig))
        {
            retVal = 1;
            
            
            Timer1::FastPWMTimerConfig &c = cfg->fastPWMTimerConfig;
            Serial.println("It worked!");
            Serial.print("timerPrescalerValue: ");
            Serial.println(c.timerPrescalerValue);
            Serial.print("tickDurationUsec: ");
            Serial.println(c.tickDurationUsec);
            Serial.print("periodUsecCalculated: ");
            Serial.println(c.periodUsecCalculated);
            Serial.print("channelAValue: ");
            Serial.println(c.channelAValue);
        }

        return retVal;
    }
    
    
    
    // Refers to the period of time where the signal will have completed
    // one entire cycle and will start again.
    //
    // Does not change the location of the signal, simply adjusts the rate of
    // movement through it.
    void SetPeriod(SignalDACPeriodConfig *cfgDac)
    {

    
        // Period refers to entire signal having completed a cycle.
        // We need to slice that period into as many chunks as we have steps.
        
        // Approx each step in 2200Hz cycle = 454us, with steps:
        // 0 25 50 75 100 75 50 25 (8 steps)
        // Each step is ~57us
        // But our callback on step takes ~12us
        // So maybe we just factor that in?  Call 57s really 45 in the math?
        
        
        
        // make atomic?
        // de-register for interrupts during this?  Or clear them?
        // stop the timer during this?  probably a bit expensive so maybe not
        // a good idea?  How to keep it doing the right thing but not mess up
        // state?
        
        
        // Take out the timer configuration
        Timer1::FastPWMTimerConfig &cfg = cfgDac->fastPWMTimerConfig;

        // Set up timer to actually drive at the rate configured.
        timer_.ApplyFastPWMTimerParams(&cfg);
            
        // Set channel B to represent Duty cycle
        timerChannelB_->SetValue(ss_->NextStep());
            
        // Attach interrupt to channel A to know when a given period ends
        Pin pinSignalA(27);
        timerChannelA_->SetInterruptHandler([=, &PAL](){
            this->DebugSetDutyCycle(ss_->NextStep());
            this->timer_.SetTimerValue(0);
            PAL.DigitalToggle(pinSignalA);
        });
        timerChannelA_->RegisterForInterrupt();
    }
    
    
    // Convenience function for times when performance isn't an issue
    // when setting the period.
    uint8_t SetPeriod(uint32_t periodUsec)
    {
        uint8_t retVal = 0;
        
        SignalDACPeriodConfig cfg;
        
        if (GetPeriodConfig(periodUsec, &cfg))
        {
            retVal = 1;
            
            SetPeriod(&cfg);
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
        //uint16_t value = (double)pct / 100.0 * timerChannelA_->GetValue();
        
        uint16_t value = pct << 3;
        
        timerChannelB_->SetValue(value);
        
        /*
        Serial.println("Changing Duty Cycle: ");
        Serial.print("pct: ");
        Serial.println(pct);
        Serial.print("value: ");
        Serial.println(value);
        */
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
    SignalSource *ss_;

    Timer1        timer_;
    TimerChannel *timerChannelA_;
    TimerChannel *timerChannelB_;
};









#endif  // __SIGNAL_H__







