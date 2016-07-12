#ifndef __SIGNAL_DAC_H__
#define __SIGNAL_DAC_H__


#include <util/atomic.h>

#include "Timer2.h"


template <typename SignalSource>
class SignalDAC
{
public:
    SignalDAC(SignalSource *ss)
    : ss_(ss)
    , timerChannelA_(timer_.GetTimerChannelA())
    , timerChannelB_(timer_.GetTimerChannelB())
    , pinDebugA_(9, LOW)
    {
        // Nothing to do
    }
    
    ~SignalDAC()
    {
        Stop();
    }
    
    struct FrequencyConfig
    {
        // Actually useful
        typename SignalSource::PhaseConfig cfgPhaseStep;
        
        // Useful for debugging
        uint16_t frequencyRequested;
    };
    
    // Configure frequency of signal
    uint8_t GetFrequencyConfig(uint16_t         frequency,
                               FrequencyConfig *cfg)
    {
        uint8_t retVal = 1;
        
        // frequency = 1200;
        
        // We know we're using FastPWM on an 8-bit timer.
        // Let's say we hand-configure it for simplicity in logic below.
        // (subject to change)
        // Figure out frequency of the timer overflow.
        
        double cpuFrequency = PAL.GetCpuFreq();
        double timerPrescaler = 1;
        //double timerTickCount = 256;
        double timerTickCount = 247;
        //double timerFrequency = (double)cpuFrequency /
        //                          (double)timerPrescaler / 
        //                          (double)timerTickCount;

        // timerFrequency = 31,250Hz


        // unused variable, but working through calculations
        //double timerTickDurationUs = 1.0 / (double)timerFrequency * 
        //                             1000000.0 / (timerTickCount * timerPrescaler);

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
        //cfg->phaseStep          = ceil(phaseStep);
        ss_->GetPhaseConfig(ceil(phaseStep), &cfg->cfgPhaseStep);
        cfg->frequencyRequested = frequency;
        
        Serial.print("frequencyRequested: ");
        Serial.println(cfg->frequencyRequested);
        Serial.print("phaseStepDouble: ");
        Serial.println(phaseStep);
        Serial.print("phaseStep: ");
        Serial.println(ceil(phaseStep));
        
        return retVal;
    }
    
    // Expected to happen at beginning of transmission
    void SetInitialFrequency(FrequencyConfig *cfg)
    {
        ss_->Reset(&cfg->cfgPhaseStep);
    }
    
    void Start()
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            // Hand set-up the timer
            timer_.SetTimerPrescaler(Timer2::TimerPrescaler::DIV_BY_1);
            timer_.SetTimerMode(Timer2::TimerMode::FAST_PWM_TOP_OCRNA);
            timer_.SetTimerValue(0);

            
            // Set channel B to first sample, and prepare subsequent sample
            timerChannelB_->SetValue(ss_->GetSample());
            ss_->GetNextSampleReady();
            
            
            // Attach interrupt to channel A to know when a given period ends
            // so that you can transition to next sequence
            timerChannelA_->SetInterruptHandler([this](){
                timerChannelB_->SetValue(ss_->GetSample());
                
                ss_->GetNextSampleReady();
            });
            timerChannelA_->SetValue(247);
            timerChannelA_->RegisterForInterrupt();
            
            
            // Begin timer counting
            timer_.StartTimer();
            
            
            // Begin comparing B to the timer value in order to operate the
            // output level
            timerChannelB_->SetFastPWMModeBehavior(
                TimerChannel::FastPWMModeBehavior::CLEAR
            );
        }
    }

    
    // Expected to happen during run
    inline void ChangeFrequency(FrequencyConfig *cfg)
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            ss_->ChangePhaseStep(&cfg->cfgPhaseStep);
        }
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

    Timer2          timer_;
    TimerChannel   *timerChannelA_;
    TimerChannel   *timerChannelB_;
    
    
    Pin  pinDebugA_;
};




#endif  // __SIGNAL_DAC_H__
















