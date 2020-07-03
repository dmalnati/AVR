#ifndef __RGB_LED_EFFECTS_CONTROLLER_H__
#define __RGB_LED_EFFECTS_CONTROLLER_H__


#include "Evm.h"
#include "SignalSourceSineWave.h"
#include "SignalOscillator.h"
#include "RgbLedPwmController.h"


class RgbLedEffectsController
{
protected:

    static const     uint16_t SAMPLING_FREQUENCY_HZ = 100;
    static const     uint32_t SAMPLING_INTERVAL_MS  = 1000 / SAMPLING_FREQUENCY_HZ;

    static const     uint32_t DEFAULT_SIGNAL_PERIOD_RED_MS      = 10000;
    static constexpr double   DEFAULT_SIGNAL_FREQUENCY_RED_HZ   = (double)1000 / DEFAULT_SIGNAL_PERIOD_RED_MS;
    static const     uint32_t DEFAULT_SIGNAL_PERIOD_GREEN_MS    = 5000;
    static constexpr double   DEFAULT_SIGNAL_FREQUENCY_GREEN_HZ = (double)1000 / DEFAULT_SIGNAL_PERIOD_GREEN_MS;
    static const     uint32_t DEFAULT_SIGNAL_PERIOD_BLUE_MS     = 8000;
    static constexpr double   DEFAULT_SIGNAL_FREQUENCY_BLUE_HZ  = (double)1000 / DEFAULT_SIGNAL_PERIOD_BLUE_MS;
    
    static const uint8_t DEFAULT_PHASE_OFFSET_BRADS = 0;

    struct ColorState
    {
        uint8_t val = 0;
    };

    struct RgbColorState
    {
        ColorState red;
        ColorState green;
        ColorState blue;
    };


public:
    RgbLedEffectsController()
    : soRed_(SignalSourceSineWave::GetSample)
    , soGreen_(SignalSourceSineWave::GetSample)
    , soBlue_(SignalSourceSineWave::GetSample)
    , running_(0)
    {
        soRed_.SetSampleRate(SAMPLING_FREQUENCY_HZ);
        soGreen_.SetSampleRate(SAMPLING_FREQUENCY_HZ);
        soBlue_.SetSampleRate(SAMPLING_FREQUENCY_HZ);

        SetFrequencyRed(DEFAULT_SIGNAL_FREQUENCY_RED_HZ);
        SetFrequencyGreen(DEFAULT_SIGNAL_FREQUENCY_GREEN_HZ);
        SetFrequencyBlue(DEFAULT_SIGNAL_FREQUENCY_BLUE_HZ);

        SetPhaseOffsetAll(DEFAULT_PHASE_OFFSET_BRADS);

        Stop();
    }

    ~RgbLedEffectsController()
    {
        Stop();
    }

    void SetFrequencyAll(double frequency)
    {
        SetFrequencyRed(frequency);
        SetFrequencyGreen(frequency);
        SetFrequencyBlue(frequency);
    }

    void SetFrequencyRed(double frequency)
    {
        soRed_.SetFrequency(frequency);
    }

    void SetFrequencyGreen(double frequency)
    {
        soGreen_.SetFrequency(frequency);
    }

    void SetFrequencyBlue(double frequency)
    {
        soBlue_.SetFrequency(frequency);
    }

    void SetPhaseOffsetAll(uint8_t phaseOffsetBrads)
    {
        SetPhaseOffsetRed(phaseOffsetBrads);
        SetPhaseOffsetGreen(phaseOffsetBrads);
        SetPhaseOffsetBlue(phaseOffsetBrads);
    }

    void SetPhaseOffsetRed(uint8_t phaseOffsetBrads)
    {
        soRed_.SetPhaseOffset(phaseOffsetBrads);
    }

    void SetPhaseOffsetGreen(uint8_t phaseOffsetBrads)
    {
        soGreen_.SetPhaseOffset(phaseOffsetBrads);
    }

    void SetPhaseOffsetBlue(uint8_t phaseOffsetBrads)
    {
        soBlue_.SetPhaseOffset(phaseOffsetBrads);
    }

    void Start()
    {
        if (!running_)
        {
            Stop();

            StartPwm();
        }

        StartSampler();

        running_ = 1;
    }

    void Pause()
    {
        StopSampler();
    }

    void Stop()
    {
        StopPwm();
        ResetOscillators();
        StopSampler();

        running_ = 0;
    }



protected:

    void StartPwm()
    {
        pwmController_.Init();
        pwmController_.Start();
    }

    void StopPwm()
    {
        pwmController_.Stop();
        pwmController_.SetRed(0);
        pwmController_.SetGreen(0);
        pwmController_.SetBlue(0);
    }

    void StartSampler()
    {
        ted_.SetCallback([this](){
            OnTimeout();
        });

        ted_.RegisterForTimedEventIntervalRigid(SAMPLING_INTERVAL_MS, 0);
    }

    void StopSampler()
    {
        ted_.DeRegisterForTimedEvent();
    }

    void ResetOscillators()
    {
        soRed_.Reset();
        soGreen_.Reset();
        soBlue_.Reset();
    }

    void GetNextState()
    {
        colorState_.red.val   = soRed_.GetNextSampleAbs();
        colorState_.green.val = soGreen_.GetNextSampleAbs();
        colorState_.blue.val  = soBlue_.GetNextSampleAbs();
    }

    RgbColorState colorState_;

private:

    void OnTimeout()
    {
        GetNextState();
        ApplyNextState();
    }

    void ApplyNextState()
    {
        pwmController_.SetRed(colorState_.red.val);
        pwmController_.SetGreen(colorState_.green.val);
        pwmController_.SetBlue(colorState_.blue.val);
    }


private:

    TimedEventHandlerDelegate ted_;

    RgbLedPwmController pwmController_;

    SignalOscillator soRed_;
    SignalOscillator soGreen_;
    SignalOscillator soBlue_;

    uint8_t running_;
};







class RgbLedEffectsControllerDebugger
: protected RgbLedEffectsController
{
public:
    RgbLedEffectsControllerDebugger()
    {
        // Nothing to do
    }

    void PrintDurationMs(uint32_t durationMs)
    {
        ResetOscillators();

        Log(P("ms,     R,   G,   B"));

        uint32_t msNow = 0;
        while (msNow <= durationMs)
        {
            GetNextState();

            printf(
                "%6lu, %3u, %3u, %3u\n",
                msNow,
                colorState_.red.val,
                colorState_.green.val,
                colorState_.blue.val
            );

            msNow += SAMPLING_INTERVAL_MS;
        }
    }

    RgbLedEffectsController &GetController()
    {
        return (RgbLedEffectsController &)*this;
    }

private:
};












#endif  // __RGB_LED_EFFECTS_CONTROLLER_H__

