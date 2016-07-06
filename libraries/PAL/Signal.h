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


// Todo
// -- swap out modulo (though maybe compiler recognizes 512)
//    (http://stackoverflow.com/questions/48053/is-there-an-alternative-to-using-modulus-in-c-c)
//   - can do if-then
//   - or mask
// -- put in any atomics before speed testing



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
    
    
    inline void Reset(uint16_t phaseStep, uint16_t phaseOffset = 0)
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






// Opaque to the caller
struct SignalDACFrequencyConfig
{
    // Actually useful configuration
    uint16_t phaseStep;

    // Useful for debugging
    uint16_t frequencyRequested;
};

#include <util/atomic.h>

template <typename SignalSource>
class SignalDAC
{
public:
    SignalDAC(SignalSource *ss)
    : ss_(ss)
    , timerChannelA_(timer_.GetTimerChannelA())
    , timerChannelB_(timer_.GetTimerChannelB())
    {
        // Nothing to do
    }
    
    ~SignalDAC()
    {
        Stop();
    }
    
    
    // Configure frequency of signal
    uint8_t GetFrequencyConfig(uint16_t                  frequency,
                               SignalDACFrequencyConfig *cfg)
    {
        uint8_t retVal = 1;
        
        // frequency = 1200;
        
        // We know we're using FastPWM on an 8-bit timer.
        // Let's say we hand-configure it for simplicity in logic below.
        // (subject to change)
        // Figure out frequency of the timer overflow.
        
        double cpuFrequency = PAL.GetCpuFreq();
        double timerPrescaler = 1;
        double timerTickCount = 256;
        double timerFrequency = (double)cpuFrequency /
                                  (double)timerPrescaler / 
                                  (double)timerTickCount;

        // timerFrequency = 31,250Hz


        double timerTickDurationUs = 1.0 / (double)timerFrequency * 
                                     1000000.0 / (timerTickCount * timerPrescaler);

        // timerTickDurationUs = 0.125

        double timerOverflowPeriodUs =
            (double)(timerTickCount * timerPrescaler) /
            (double) cpuFrequency *
            1000000.0;

        // timerOverflowPeriodUs = 32
            
        double frequencyPeriodUs = 1.0 / (double)frequency * 1000000.0;
        
        // frequencyPeriodUs = 833
        
        double timerOverflowsPerFrequencyPeriod =
            (double)frequencyPeriodUs / (double)timerOverflowPeriodUs;
        
        // timerOverflowsPerFrequencyPeriod = 26
        
        
        double phaseStep = 360.0 / (double)timerOverflowsPerFrequencyPeriod;
        
        // phaseStep = 13
        
        
        
        // fill out configuration
        cfg->phaseStep          = ceil(phaseStep);
        cfg->frequencyRequested = frequency;
        
        Serial.print("frequencyRequested: ");
        Serial.println(cfg->frequencyRequested);
        Serial.print("phaseStepDouble: ");
        Serial.println(phaseStep);
        Serial.print("phaseStep: ");
        Serial.println(cfg->phaseStep);
        
        return retVal;
    }
    
    
    // Expected to happen at beginning of transmission
    void SetInitialFrequency(SignalDACFrequencyConfig *cfg)
    {
        ss_->Reset(cfg->phaseStep);
    }
    
    // Expected to happen during run
    inline void ChangeFrequency(SignalDACFrequencyConfig *cfg)
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            ss_->ChangePhaseStep(cfg->phaseStep);
        }
    }

    void Start()
    {
        // Stop any ongoing timer
        Stop();
        
        
        
        // Hand set-up the timer
        timer_.SetTimerPrescaler(Timer1::TimerPrescaler::DIV_BY_1);
        timer_.SetTimerMode(Timer1::TimerMode::FAST_PWM_8_BIT);
        timer_.SetTimerValue(0);

        
        timerChannelB_->SetFastPWMModeBehavior(
            TimerChannel::FastPWMModeBehavior::CLEAR
        );
        
        
        // Set channel B to first sample, and prepare subsequent sample
        timerChannelB_->SetValue(ss_->GetSample());
        ss_->GetNextSampleReady();
        
        
        
        
        
        // Attach interrupt to channel A to know when a given period ends
        // so that you can transition to next sequence
        // TODO -- use overflow interrupt instead of this
        timerChannelA_->SetValue(255);
        Pin pinSignalA(27);
        timerChannelA_->SetInterruptHandler([=, &PAL](){
            
            PAL.DigitalToggle(pinSignalA);
            
            timerChannelB_->SetValue(ss_->GetSample());
            ss_->GetNextSampleReady();
            
        });
        timerChannelA_->RegisterForInterrupt();
        
        
        
        
        // Begin timer counting
        timer_.StartTimer();
    }
    
    void Stop()
    {
        timer_.StopTimer();
    }
    

private:
    SignalSource *ss_;

    Timer1        timer_;
    TimerChannel *timerChannelA_;
    TimerChannel *timerChannelB_;
};









#endif  // __SIGNAL_H__







