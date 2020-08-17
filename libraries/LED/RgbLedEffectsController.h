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

    static const     uint32_t DEFAULT_SIGNAL_PERIOD_RED_MS   = 10000;
    static const     uint32_t DEFAULT_SIGNAL_PERIOD_GREEN_MS =  5000;
    static const     uint32_t DEFAULT_SIGNAL_PERIOD_BLUE_MS  =  8000;
    
    static const uint8_t DEFAULT_PHASE_OFFSET_BRADS = 0;

public:

    struct ColorState
    {
        uint8_t                     val              = 0;
        uint32_t                    periodMs         = 0;
        uint8_t                     phaseOffsetBrads = 0;
        Q88::INTERNAL_STORAGE_TYPE  rotation         = 0;
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
    , pctRangeMultiplier_(1.0)
    , running_(0)
    {
        soRed_.SetSampleRate(SAMPLING_FREQUENCY_HZ);
        soGreen_.SetSampleRate(SAMPLING_FREQUENCY_HZ);
        soBlue_.SetSampleRate(SAMPLING_FREQUENCY_HZ);

        SetPeriodRed(DEFAULT_SIGNAL_PERIOD_RED_MS);
        SetPeriodGreen(DEFAULT_SIGNAL_PERIOD_GREEN_MS);
        SetPeriodBlue(DEFAULT_SIGNAL_PERIOD_BLUE_MS);

        SetPhaseOffsetAll(DEFAULT_PHASE_OFFSET_BRADS);

        Stop();
    }

    ~RgbLedEffectsController()
    {
        Stop();
    }

    void SetPeriodAll(uint32_t periodMs)
    {
        SetPeriodRed(periodMs);
        SetPeriodGreen(periodMs);
        SetPeriodBlue(periodMs);
    }

    void SetPeriodRed(uint32_t periodMs)
    {
        colorState_.red.periodMs = periodMs;
        SetFrequencyRed(PeriodToFrequency(colorState_.red.periodMs));
    }

    void SetPeriodGreen(uint32_t periodMs)
    {
        colorState_.green.periodMs = periodMs;
        SetFrequencyGreen(PeriodToFrequency(colorState_.green.periodMs));
    }

    void SetPeriodBlue(uint32_t periodMs)
    {
        colorState_.blue.periodMs = periodMs;
        SetFrequencyBlue(PeriodToFrequency(colorState_.blue.periodMs));
    }

    void SetPhaseOffsetAll(uint8_t phaseOffsetBrads)
    {
        SetPhaseOffsetRed(phaseOffsetBrads);
        SetPhaseOffsetGreen(phaseOffsetBrads);
        SetPhaseOffsetBlue(phaseOffsetBrads);
    }

    void SetPhaseOffsetRed(uint8_t phaseOffsetBrads)
    {
        colorState_.red.phaseOffsetBrads = phaseOffsetBrads;
        soRed_.SetPhaseOffset(colorState_.red.phaseOffsetBrads);
    }

    void SetPhaseOffsetGreen(uint8_t phaseOffsetBrads)
    {
        colorState_.green.phaseOffsetBrads = phaseOffsetBrads;
        soGreen_.SetPhaseOffset(colorState_.green.phaseOffsetBrads);
    }

    void SetPhaseOffsetBlue(uint8_t phaseOffsetBrads)
    {
        colorState_.blue.phaseOffsetBrads = phaseOffsetBrads;
        soBlue_.SetPhaseOffset(colorState_.blue.phaseOffsetBrads);
    }

    void SetRangePct(uint8_t pctRange)
    {
        pctRangeMultiplier_ = (double)pctRange / 100.0;
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

    const RgbColorState GetState()
    {
        return colorState_;
    }

    void SetState(RgbColorState rgbColorState)
    {
        // Expect full-range values, adjust using multiplier here

        // Red
        pwmController_.SetRed(rgbColorState.red.val * pctRangeMultiplier_);
        SetPeriodRed(rgbColorState.red.periodMs);
        SetPhaseOffsetRed(rgbColorState.red.phaseOffsetBrads);
        soRed_.ReplaceRotationState(rgbColorState.red.rotation);

        // Green
        pwmController_.SetGreen(rgbColorState.green.val * pctRangeMultiplier_);
        SetPeriodGreen(rgbColorState.green.periodMs);
        SetPhaseOffsetGreen(rgbColorState.green.phaseOffsetBrads);
        soGreen_.ReplaceRotationState(rgbColorState.green.rotation);

        // Blue
        pwmController_.SetBlue(rgbColorState.blue.val * pctRangeMultiplier_);
        SetPeriodBlue(rgbColorState.blue.periodMs);
        SetPhaseOffsetBlue(rgbColorState.blue.phaseOffsetBrads);
        soBlue_.ReplaceRotationState(rgbColorState.blue.rotation);
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
        colorState_.red.val        = soRed_.GetNextSampleAbs();
        colorState_.red.rotation   = soRed_.GetRotationState();

        colorState_.green.val      = soGreen_.GetNextSampleAbs();
        colorState_.green.rotation = soGreen_.GetRotationState();

        colorState_.blue.val       = soBlue_.GetNextSampleAbs();
        colorState_.blue.rotation  = soBlue_.GetRotationState();
    }

    RgbColorState colorState_;

private:

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

    double PeriodToFrequency(uint32_t periodMs)
    {
        return (double)1000 / periodMs;
    }

    void OnTimeout()
    {
        GetNextState();
        ApplyNextState();
    }

    void ApplyNextState()
    {
        // Expect full-range values, adjust using multiplier here
        
        pwmController_.SetRed(colorState_.red.val * pctRangeMultiplier_);
        pwmController_.SetGreen(colorState_.green.val * pctRangeMultiplier_);
        pwmController_.SetBlue(colorState_.blue.val * pctRangeMultiplier_);
    }


private:

    TimedEventHandlerDelegate ted_;

    RgbLedPwmController pwmController_;

    SignalOscillator soRed_;
    SignalOscillator soGreen_;
    SignalOscillator soBlue_;

    double pctRangeMultiplier_;

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

