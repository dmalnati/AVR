#ifndef __SIGNAL_DAC_H__
#define __SIGNAL_DAC_H__


#include <util/atomic.h>

#include "Timer2.h"


template <typename SignalSource>
class SignalDAC
{
public:
    SignalDAC()
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
        typename SignalSource::PhaseConfig cfgPhase;
        
        // Useful for debugging
        uint16_t frequencyRequested;
    };
    
    static
    void GetFrequencyConfig(uint16_t         frequency,
                            FrequencyConfig *cfg)
    {
        // Method:
        // 
        // User enters a frequency: eg 1200Hz
        // 
        // This code intends to:
        // - determine duration of single signal loop at frequency
        // - determine duration of single timer loop
        //   - assume 8-bit FastPWM timer (256 ticks), no prescaler
        // - calculate number of timer loops per signal loop
        //   - calculate percent of signal each timer loop represents
        // - get configuration for signal source
        
        
        // - determine duration of single signal loop at frequency
        double durationSingleWaveAtFrequencyUs = 1000000.0 / (double)frequency;
        
        // - determine duration of single timer loop
        //   - assume 8-bit FastPWM timer (256 ticks), no prescaler
        double cpuFrequency          = PAL.GetCpuFreq();
        double timerPrescaler        = 1;
        double timerTickCount        = 256;
        double durationSingleTimerUs = 
            1000000.0 / (cpuFrequency / timerPrescaler / timerTickCount);
        
        // - calculate number of timer loops per signal loop
        //   - calculate percent of signal each timer loop represents
        double timerLoopsPerSingleWaveAtFrequency =
            durationSingleWaveAtFrequencyUs / durationSingleTimerUs;
        double phasePct = 1.0 / timerLoopsPerSingleWaveAtFrequency * 100.0;
        
        // - get configuration for signal source
        ss_.GetPhaseConfig(phasePct, &cfg->cfgPhase);
        cfg->frequencyRequested = frequency;
        
        Serial.print("frequencyRequested: ");
        Serial.println(cfg->frequencyRequested);
        Serial.print("durationSingleWaveAtFrequencyUs: ");
        Serial.println(durationSingleWaveAtFrequencyUs);
        Serial.print("durationSingleTimerUs: ");
        Serial.println(durationSingleTimerUs);
        Serial.print("timerLoopsPerSingleWaveAtFrequency: ");
        Serial.println(timerLoopsPerSingleWaveAtFrequency);
        Serial.print("phasePct: ");
        Serial.println(phasePct);
        Serial.print("idxStep: ");
        Serial.println(cfg->cfgPhase.idxStep);
    }
    
    // Expected to happen at beginning of transmission
    static
    void SetInitialFrequency(FrequencyConfig *cfg)
    {
        ss_.Reset(&cfg->cfgPhase);
    }
    
    static
    void Start()
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            // Hand set-up the timer
            timer_.SetTimerPrescaler(Timer2::TimerPrescaler::DIV_BY_1);
            //timer_.SetTimerMode(Timer2::TimerMode::FAST_PWM_TOP_OCRNA);
            timer_.SetTimerMode(Timer2::TimerMode::FAST_PWM);
            timer_.SetTimerValue(0);

            // Set channel B to first sample, and prepare subsequent sample
            timerChannelB_->SetValue(ss_.GetSample());
            ss_.GetNextSampleReady();
            
            // Attach interrupt to overflow to know when a given period ends
            // so that you can transition to next sequence
            timerChannelOvf_->SetInterruptHandlerRaw(OnInterrupt);
            timerChannelOvf_->RegisterForInterrupt();
            
            // Begin timer counting
            timer_.StartTimer();
            
            // Begin comparing B to the timer value in order to operate the
            // output level
            timerChannelB_->SetFastPWMModeBehavior(
                TimerChannel::FastPWMModeBehavior::CLEAR
            );
        }
    }
    
    static
    void OnInterrupt()
    {
        timerChannelB_->SetValue(ss_.GetSample());
        
        ss_.GetNextSampleReady();
    }
    
    // Expected to happen during run
    static
    inline void ChangeFrequency(FrequencyConfig *cfg)
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            ChangeFrequencyNonAtomic(cfg);
        }
    }
    
    static
    inline void ChangeFrequencyNonAtomic(FrequencyConfig *cfg)
    {
        ss_.ChangePhaseStep(&cfg->cfgPhase);
        
        // debug -- test duration increase (+5us)
        // maybe this doesn't happen when pin toggling not turned on?
        //ss_.GetNextSampleReady();
    }
    
    static
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
    static SignalSource ss_;

    static Timer2                    timer_;
    constexpr static TimerChannel   *timerChannelA_   = Timer2::GetTimerChannelA();
    constexpr static TimerChannel   *timerChannelB_   = Timer2::GetTimerChannelB();
    constexpr static TimerInterrupt *timerChannelOvf_ = Timer2::GetTimerOverflowHandler();
};

template <typename SignalSource>
SignalSource SignalDAC<SignalSource>::ss_;


#endif  // __SIGNAL_DAC_H__
















