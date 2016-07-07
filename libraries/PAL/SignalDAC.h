#ifndef __SIGNAL_DAC_H__
#define __SIGNAL_DAC_H__


#include <util/atomic.h>

#include "Timer.h"


// Opaque to the caller
struct SignalDACFrequencyConfig
{
    // Actually useful configuration
    uint16_t phaseStep;

    // Useful for debugging
    uint16_t frequencyRequested;
};


template <typename SignalSource>
class SignalDAC
{
public:
    SignalDAC(SignalSource *ss)
    : ss_(ss)
    , timerChannelA_(timer_.GetTimerChannelA())
    , timerChannelB_(timer_.GetTimerChannelB())
    , wrapCounter_(&wrapCounterDummy_)
    , pinDebug_(4)
    {
        // Nothing to do
        
        // Debug
        PAL.PinMode(pinDebug_, OUTPUT);
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
    
    void Start()
    {
        // Stop any ongoing timer
        //Stop();
        
        
        
        // Hand set-up the timer
        timer_.SetTimerValue(0);
        timer_.SetTimerPrescaler(Timer1::TimerPrescaler::DIV_BY_1);
        timer_.SetTimerMode(Timer1::TimerMode::FAST_PWM_8_BIT);

        
        
        // Set channel B to first sample, and prepare subsequent sample
        timerChannelB_->SetValue(ss_->GetSample());
        ss_->GetNextSampleReady();
        
        
        
        
        
        // Attach interrupt to channel A to know when a given period ends
        // so that you can transition to next sequence
        // TODO -- use overflow interrupt instead of this
        timerChannelA_->SetValue(255);
        timerChannelA_->SetInterruptHandler([this](){
            
            timerChannelB_->SetValue(ss_->GetSample());
            ss_->GetNextSampleReady();
            
            ++(*wrapCounter_);
            
        });
        timerChannelA_->RegisterForInterrupt();
        
        
        
        
        
        // Begin timer counting
        timer_.StartTimer();
        
        
        // Begin comparing B to the timer value in order to operate the output
        // level
        timerChannelB_->SetFastPWMModeBehavior(
            TimerChannel::FastPWMModeBehavior::CLEAR
        );
    }
    
    // Expected to happen during run
    inline void ChangeFrequency(SignalDACFrequencyConfig  *cfg,
                                volatile register uint8_t *wrapCounter = NULL)
    {
        PAL.DigitalToggle(pinDebug_);
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            ss_->ChangePhaseStep(cfg->phaseStep);
            
            wrapCounter_ = wrapCounter ? wrapCounter : &wrapCounterDummy_;
            *wrapCounter_ = 0;
        }
        PAL.DigitalToggle(pinDebug_);
    }
    
    void Stop()
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            // Stop the timer counting
            timer_.StopTimer();

            // Don't let any potentially queued interrupts fire
            timerChannelA_->DeRegisterForInterrupt();

            // Make sure B channel is no longer evaluated in terms of the timer
            // for setting output pin level
            timerChannelB_->SetFastPWMModeBehavior(
                TimerChannel::FastPWMModeBehavior::NONE
            );
            
            // In case the output pin was high when this function got called
            timerChannelB_->OutputLow();
        }
    }
    

private:
    SignalSource *ss_;

    Timer1        timer_;
    TimerChannel *timerChannelA_;
    TimerChannel *timerChannelB_;
    
    volatile uint8_t *wrapCounter_;
    volatile uint8_t  wrapCounterDummy_;
    
    Pin pinDebug_;
};




#endif  // __SIGNAL_DAC_H__
















