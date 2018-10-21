#ifndef __APP_EVIL_EYE_H__
#define __APP_EVIL_EYE_H__


#include "Log.h"
#include "Evm.h"
#include "FunctionChain.h"
#include "SerialInput.h"
#include "SensorCapacitiveTouchCAP1188.h"
#include "LedMatrixTimeMultiplexer.h"


struct AppEvilEyeConfig
{
};


class AppEvilEye
{
private:
    
    static const uint8_t CAP_I2C_ADDR = 0x28;
    
    static const uint32_t DEFAULT_CAP_SENSE_POLL_PERIOD = 500;
    
    static const uint32_t MUX_INTERVAL_US = 500;
    
    
public:
    AppEvilEye(AppEvilEyeConfig &cfg)
    : cfg_(cfg)
    , cap_(CAP_I2C_ADDR)
    , mux_(ROW_PIN_LIST, COL_PIN_LIST)
    {
        // Nothing to do
    }
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // Entry point
    //
    ///////////////////////////////////////////////////////////////////////////

    void Run()
    {
        // Init serial and announce startup
        LogStart(9600);
        Log(P("Starting"));
        
        // Setup interactive controls
        SetupShell();
        
        // Start up LED renderer
        mux_.SetRowIntervalUs(MUX_INTERVAL_US);
        
        // Set up for animation
        EnterFullPowerAnimationState();
        
        // Say hi
        StartupAnimation([this](){
            EnterLowPowerSensingState();
        });
        
        // Handle async events
        Log(P("Running"));
        
        evm_.MainLoopLowPower();
    }
    
private:

    ///////////////////////////////////////////////////////////////////////////
    //
    // Operating States
    //
    ///////////////////////////////////////////////////////////////////////////

    void EnterFullPowerAnimationState()
    {
        Log(P("EnterFullPowerAnimationState"));
        
        evm_.LowPowerDisable();
        
        mux_.Start();
        
        //ReportEvmEvents();
        
        CapSenseDisable();
    }
    
    void EnterLowPowerSensingState()
    {
        Log(P("EnterLowPowerSensingState"));
        
        mux_.Stop();
        
        CapSenseEnable();
        
        //ReportEvmEvents();
        
        evm_.LowPowerEnable();
    }
    
    void ReportEvmEvents()
    {
        Log(P("Idle: "), evm_.GetIdleEventCount());
        Log(P("Timed: "), evm_.GetTimedEventCount());
        Log(P("Interrupt: "), evm_.GetInterruptEventCount());
    }
    
    void CapSenseEnable()
    {
        CapSenseDisable();
        
        cap_.PowerStandby();
        cap_.StandbyConfigure(
            SensorCapacitiveTouchCAP1188::StandbyAvgOrSum::AVG,
            SensorCapacitiveTouchCAP1188::StandbySamplesPerMeasurement::COUNT_1,
            SensorCapacitiveTouchCAP1188::StandbySampleTime::US_2560,
            SensorCapacitiveTouchCAP1188::StandbyCycleTime::MS_140
        );
        cap_.SetStandbyInputEnable(0b10000000);
        cap_.SetStandbySensitivity(7);
        
        cap_.SetCallback([this](uint8_t bitmapTouched){
            if (bitmapTouched)
            {
                OnSense();
            }
        });
        cap_.SetPollPeriodMs(DEFAULT_CAP_SENSE_POLL_PERIOD);
        cap_.Start();
        
        // Seen that there is a stray touched reading at startup, not sure why
        cap_.GetTouched();
    }
    
    void CapSenseDisable()
    {
        cap_.SetStandbyInputEnable(0b00000000);
        cap_.Stop();
    }
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // Init functions
    //
    ///////////////////////////////////////////////////////////////////////////

    void SetupShell()
    {
        shell_.RegisterErrorHandler([this](char *cmdStr){
            Str str(cmdStr);
            
            if (str.TokenCount(' ') >= 1)
            {
                const char *cmd = str.TokenAtIdx(0, ' ');
                
                if (!strcmp(cmd, "start"))
                {
                    Log(P("Starting"));
                    mux_.Start();
                }
                else if (!strcmp(cmd, "stop"))
                {
                    Log(P("Stopping"));
                    mux_.Stop();
                }
                else if (!strcmp(cmd, "on"))
                {
                    Log(P("On"));
                    LedSetAllOn();
                }
                else if (!strcmp(cmd, "off"))
                {
                    Log(P("Off"));
                    LedSetAllOff();
                }
                else if (!strcmp(cmd, "p"))
                {
                    if (str.TokenCount(' ') == 3)
                    {
                        uint8_t pin   = atoi(str.TokenAtIdx(1, ' '));
                        uint8_t onOff = atoi(str.TokenAtIdx(2, ' '));

                        Log(P("Pin "), pin, P(" = "), onOff);
                        
                        mux_.SetLedState(pin, onOff);
                    }
                    else if (str.TokenCount(' ') == 4)
                    {
                        uint8_t row   = atoi(str.TokenAtIdx(1, ' '));
                        uint8_t col   = atoi(str.TokenAtIdx(2, ' '));
                        uint8_t onOff = atoi(str.TokenAtIdx(3, ' '));

                        Log(P("Pin "), row, ',', col, P(" = "), onOff);
                        
                        mux_.SetLedState(row, col, onOff);
                    }
                }
                else if (!strcmp(cmd, "us"))
                {
                    if (str.TokenCount(' ') == 2)
                    {
                        uint32_t us = atol(str.TokenAtIdx(1, ' '));

                        Log(P("us = "), us);
                        mux_.SetRowIntervalUs(us);
                    }
                }
                else if (!strcmp(cmd, "rampup"))
                {
                    if (str.TokenCount(' ') == 2)
                    {
                        uint32_t ms = atol(str.TokenAtIdx(1, ' '));

                        Log(P("Ramp Up at "), ms, P(" ms"));
                        
                        DoAnimationRampUp(ms);
                    }
                }
                else if (!strcmp(cmd, "rampdn"))
                {
                    if (str.TokenCount(' ') == 2)
                    {
                        uint32_t ms = atol(str.TokenAtIdx(1, ' '));

                        Log(P("Ramp Down at "), ms, P(" ms"));
                        
                        DoAnimationRampDown(ms);
                    }
                }
                else if (!strcmp(cmd, "iris"))
                {
                    if (str.TokenCount(' ') == 4)
                    {
                        uint32_t totalDurationMs  = atol(str.TokenAtIdx(1, ' '));
                        uint8_t  forwardOrReverse = atoi(str.TokenAtIdx(2, ' '));
                        uint8_t  onOff            = atoi(str.TokenAtIdx(3, ' '));

                        Log(P("IrisRamp "), totalDurationMs, ", ", forwardOrReverse, ", ", onOff);
                        
                        DoIrisRamp(totalDurationMs, forwardOrReverse, onOff);
                    }
                }
                else if (!strcmp(cmd, "border"))
                {
                    if (str.TokenCount(' ') == 5)
                    {
                        const char *strSide          = str.TokenAtIdx(1, ' ');
                        uint32_t    totalDurationMs  = atol(str.TokenAtIdx(2, ' '));
                        uint8_t     forwardOrReverse = atoi(str.TokenAtIdx(3, ' '));
                        uint8_t     onOff            = atoi(str.TokenAtIdx(4, ' '));

                        Log(P("Border "), strSide, P(": "), totalDurationMs, ", ", forwardOrReverse, ", ", onOff);
                        
                        if (!strcmp(strSide, "ur"))
                        {
                            DoBorderUpperRightRamp(totalDurationMs, forwardOrReverse, onOff);
                        }
                        else if (!strcmp(strSide, "lr"))
                        {
                            DoBorderLowerRightRamp(totalDurationMs, forwardOrReverse, onOff);
                        }
                        else if (!strcmp(strSide, "ll"))
                        {
                            DoBorderLowerLeftRamp(totalDurationMs, forwardOrReverse, onOff);
                        }
                        else if (!strcmp(strSide, "ul"))
                        {
                            DoBorderUpperLeftRamp(totalDurationMs, forwardOrReverse, onOff);
                        }
                        else if (!strcmp(strSide, "all"))
                        {
                            DoBorderRamp(totalDurationMs, forwardOrReverse, onOff);
                        }
                        else
                        {
                            Log(P("    Invalid side"));
                        }
                    }
                }
                else if (!strcmp(cmd, "sense"))
                {
                    Log(P("Sense"));
                    OnSense();
                }
                else if (!strcmp(cmd, ""))
                {
                }
            }
        });

        // Uncomment to use, but not compatible with low-power intended use
        // otherwise
        //shell_.Start();
    }
    
    
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // Event Handlers
    //
    ///////////////////////////////////////////////////////////////////////////
    
    void OnSense()
    {
        Log(P("OnSense"));

        EnterFullPowerAnimationState();
        
        uint32_t durationIrisBlinkMs   = 2000;
        uint32_t durationBorderTotalMs = 1500;
        uint32_t durationAllOn         = 1500;
        uint32_t durationFadeOutMs     = 3500;
        
        uint32_t durationBorderMs = durationBorderTotalMs / 3;
        
        mux_.SetRowIntervalUs(MUX_INTERVAL_US);
        
        LedSetAllOff();
        
        
        fnChainCtl_.Reset();
        
        // Iris blink
        fnChainCtl_.Append([=](){
            IrisBlink(3, durationIrisBlinkMs);
        });
        
        // Border in, out, in
        fnChainCtl_.Append([=](){
            IrisOn();
            
            DoBorderRamp(durationBorderMs, 0, 1);
        }, durationIrisBlinkMs);
        fnChainCtl_.Append([=](){
            DoBorderRamp(durationBorderMs, 0, 0);
        }, durationBorderMs);
        fnChainCtl_.Append([=](){
            DoBorderRamp(durationBorderMs, 1, 1);
        }, durationBorderMs);
        
        // Full shine
        fnChainCtl_.Append([=](){
            LedSetAllOn();
        }, durationBorderMs);
        
        // Fade out
        fnChainCtl_.Append([=](){
            DoAnimationRampDown(durationFadeOutMs);
        }, durationAllOn);
        
        // Cleanup on complete
        fnChainCtl_.SetCallbackOnComplete([this](){
            LedSetAllOff();
            
            fnChainCtl_.Stop();
            fnChainAnimation_.Stop();
            
            OnSenseComplete();
        }, durationFadeOutMs);
        
        fnChainCtl_.Start();
    }
    
    void OnSenseComplete()
    {
        Log(P("OnSenseComplete"));
        
        EnterLowPowerSensingState();
    }
    
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // Animation Utilities
    //
    ///////////////////////////////////////////////////////////////////////////

    void LedSetAll(uint8_t onOff)
    {
        for (uint8_t row = 0; row < ROW_COUNT; ++row)
        {
            for (uint8_t col = 0; col < COL_COUNT; ++col)
            {
                mux_.SetLedState(row, col, onOff);
            }
        }
    }
    
    void LedSetAllOn()
    {
        LedSetAll(1);
    }
    
    void LedSetAllOff()
    {
        LedSetAll(0);
    }
    
    
    void LedSetAllList(const uint8_t *pinList, uint8_t pinListLen, uint8_t onOff)
    {
        for (uint8_t i = 0; i < pinListLen; ++i)
        {
            mux_.SetLedState(pinList[i], onOff);
        }
    }
    
    void LedSetListOn(const uint8_t *pinList, uint8_t pinListLen)
    {
        LedSetAllList(pinList, pinListLen, 1);
    }
    
    void LedSetListOff(const uint8_t *pinList, uint8_t pinListLen)
    {
        LedSetAllList(pinList, pinListLen, 0);
    }
    
    void LedBlinkList(const uint8_t *pinList, uint8_t pinListLen, uint32_t blinkCount, uint32_t blinkDurationTotalMs)
    {
        uint32_t intervalMs = blinkDurationTotalMs / (blinkCount * 2);
        uint32_t delayMs    = 0;
        
        fnChainAnimation_.Reset();
        
        for (uint8_t i = 0; i < blinkCount; ++i)
        {
            fnChainAnimation_.Append([=](){
                LedSetListOn(pinList, pinListLen);
            }, delayMs);
            
            delayMs = intervalMs;
            
            fnChainAnimation_.Append([=](){
                LedSetListOff(pinList, pinListLen);
            }, delayMs);
        }
        
        fnChainAnimation_.Start();
    }
    
    
    void DoAnimationRampUp(uint32_t         durationTotalMs,
                           function<void()> cbFn             = function<void()>{})
    {
        static const uint32_t rowIntervalUsList[]  = { 50000, 40000, 30000, 20000, 10000, 5000, 2500, 1800, 500 };
        static const uint8_t  rowIntervalUsListLen = sizeof(rowIntervalUsList) / sizeof(uint32_t);
        
        uint32_t intervalBetweenSpeedsMs = durationTotalMs / rowIntervalUsListLen;
        
        LedSetAllOn();
        
        fnChainAnimation_.Reset();
        
        uint32_t delayMs = 0;
        for (auto rowIntervalUs : rowIntervalUsList)
        {
            fnChainAnimation_.Append([=](){
                mux_.SetRowIntervalUs(rowIntervalUs);
            }, delayMs);
            
            delayMs = intervalBetweenSpeedsMs;
        }
        
        fnChainAnimation_.Append([=](){ LedSetAllOff(); }, intervalBetweenSpeedsMs);
        
        fnChainAnimation_.SetCallbackOnComplete(cbFn);

        fnChainAnimation_.Start();
    }
    
    void DoAnimationRampDown(uint32_t durationTotalMs)
    {
        static const uint32_t rowIntervalUsList[] = { 500, 1800, 2500, 5000, 10000, 20000, 30000, 40000, 50000 };
        static const uint8_t  rowIntervalUsListLen = sizeof(rowIntervalUsList) / sizeof(uint32_t);
        
        uint32_t intervalBetweenSpeedsMs = durationTotalMs / rowIntervalUsListLen;
        
        LedSetAllOn();
        
        fnChainAnimation_.Reset();
        
        uint32_t delayMs = 0;
        for (auto rowIntervalUs : rowIntervalUsList)
        {
            fnChainAnimation_.Append([=](){
                mux_.SetRowIntervalUs(rowIntervalUs);
            }, delayMs);
            
            delayMs = intervalBetweenSpeedsMs;
        }
        
        fnChainAnimation_.SetCallbackOnComplete([this](){ LedSetAllOff(); }, intervalBetweenSpeedsMs);

        fnChainAnimation_.Start();
    }
    
    void PathRampMulti(const uint8_t **pinListList,
                       uint8_t         pinListListLen,
                       uint8_t         pinListLen,
                       uint32_t        totalDurationMs,
                       uint8_t         forwardOrReverse,
                       uint8_t         onOff)
    {
        uint32_t intervalMs = (totalDurationMs / pinListLen);
        uint32_t delayMs    = 0;
        
        fnChainAnimation_.Reset();
        
        if (forwardOrReverse)
        {
            for (uint8_t i = pinListLen; i > 0; --i)
            {
                fnChainAnimation_.Append([=](){
                    for (uint8_t j = 0; j < pinListListLen; ++j)
                    {
                        uint8_t pin = pinListList[j][i - 1];
                        mux_.SetLedState(pin, onOff);
                    }

                }, delayMs);
                
                delayMs = intervalMs;
            }
        }
        else
        {
            for (uint8_t i = 0; i < pinListLen; ++i)
            {
                fnChainAnimation_.Append([=](){
                    for (uint8_t j = 0; j < pinListListLen; ++j)
                    {
                        uint8_t pin = pinListList[j][i];
                        mux_.SetLedState(pin, onOff);
                    }
                }, delayMs);
                
                delayMs = intervalMs;
            }
        }
        
        fnChainAnimation_.Start();
    }
    
    void PathRamp(const uint8_t  *pinList,
                  uint8_t         pinListLen,
                  uint32_t        totalDurationMs,
                  uint8_t         forwardOrReverse,
                  uint8_t         onOff)
    {
        PathRampMulti(&pinList, 1, pinListLen, totalDurationMs, forwardOrReverse, onOff);
    }

    
    ///////////////////////////////////////////////////////////////////////////
    //
    // Animation Higher Level
    //
    ///////////////////////////////////////////////////////////////////////////

    void StartupAnimation(function<void()> cbFn)
    {
        DoAnimationRampUp(2500, cbFn);
    }
    
    
    void DoIrisRamp(uint32_t totalDurationMs, uint8_t forwardOrReverse, uint8_t onOff)
    {
        PathRamp(IRIS_PIN_LIST, IRIS_PIN_LIST_LEN, totalDurationMs, forwardOrReverse, onOff);
    }
    
    void DoIrisRampLeftOn(uint32_t totalDurationMs)
    {
        DoIrisRamp(totalDurationMs, 1, 1);
    }
    
    void DoIrisRampLeftOff(uint32_t totalDurationMs)
    {
        DoIrisRamp(totalDurationMs, 1, 0);
    }
    
    void DoIrisRampRightOn(uint32_t totalDurationMs)
    {
        DoIrisRamp(totalDurationMs, 0, 1);
    }
    
    void DoIrisRampRightOff(uint32_t totalDurationMs)
    {
        DoIrisRamp(totalDurationMs, 0, 0);
    }
    
    void IrisBlink(uint8_t blinkCount, uint32_t durationTotalMs)
    {
        LedBlinkList(IRIS_PIN_LIST, IRIS_PIN_LIST_LEN, blinkCount, durationTotalMs);
    }
    
    void IrisOn()
    {
        LedSetAllList(IRIS_PIN_LIST, IRIS_PIN_LIST_LEN, 1);
    }
    

    
    
    void DoBorderUpperRightRamp(uint32_t totalDurationMs, uint8_t forwardOrReverse, uint8_t onOff)
    {
        PathRamp(BORDER_UPPER_RIGHT_PIN_LIST, BORDER_PIN_LIST_LEN, totalDurationMs, forwardOrReverse, onOff);
    }
    
    void DoBorderLowerRightRamp(uint32_t totalDurationMs, uint8_t forwardOrReverse, uint8_t onOff)
    {
        PathRamp(BORDER_LOWER_RIGHT_PIN_LIST, BORDER_PIN_LIST_LEN, totalDurationMs, forwardOrReverse, onOff);
    }
    
    void DoBorderLowerLeftRamp(uint32_t totalDurationMs, uint8_t forwardOrReverse, uint8_t onOff)
    {
        PathRamp(BORDER_LOWER_LEFT_PIN_LIST, BORDER_PIN_LIST_LEN, totalDurationMs, forwardOrReverse, onOff);
    }
    
    void DoBorderUpperLeftRamp(uint32_t totalDurationMs, uint8_t forwardOrReverse, uint8_t onOff)
    {
        PathRamp(BORDER_UPPER_LEFT_PIN_LIST, BORDER_PIN_LIST_LEN, totalDurationMs, forwardOrReverse, onOff);
    }
    
    void DoBorderRamp(uint32_t totalDurationMs, uint8_t forwardOrReverse, uint8_t onOff)
    {
        PathRampMulti(BORDER_PIN_LIST_LIST,
                      BORDER_PIN_LIST_LIST_LEN,
                      BORDER_PIN_LIST_LEN,
                      totalDurationMs,
                      forwardOrReverse,
                      onOff);
    }
    
    
    
    
    
private:

    static const uint8_t C_IDLE  = 1;
    static const uint8_t C_TIMED = 3;
    static const uint8_t C_INTER = 0;
    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

    AppEvilEyeConfig &cfg_;
    
    SensorCapacitiveTouchCAP1188 cap_;
    
    
    static const uint8_t ROW_COUNT = 14;
    static const uint8_t COL_COUNT = 4;
    LedMatrixTimeMultiplexer<ROW_COUNT, COL_COUNT> mux_;
    
    static const uint8_t FN_CHAIN_ANIMATION_LEN = 14;
    FunctionChainAsync<FN_CHAIN_ANIMATION_LEN> fnChainAnimation_;

    static const uint8_t FN_CHAIN_CTL_LEN = 6;
    FunctionChainAsync<FN_CHAIN_CTL_LEN> fnChainCtl_;

    static const uint8_t CONSOLE_CMD_COUNT = 0;
    SerialAsyncConsoleEnhanced<CONSOLE_CMD_COUNT> shell_;

    
    static const uint8_t ROW_PIN_LIST[ROW_COUNT];
    static const uint8_t COL_PIN_LIST[COL_COUNT];
    
    static const uint8_t IRIS_PIN_LIST_LEN = ROW_COUNT;
    static const uint8_t IRIS_PIN_LIST[IRIS_PIN_LIST_LEN];
    
    static const uint8_t BORDER_PIN_LIST_LEN = 5;
    static const uint8_t BORDER_UPPER_RIGHT_PIN_LIST[BORDER_PIN_LIST_LEN];
    static const uint8_t BORDER_LOWER_RIGHT_PIN_LIST[BORDER_PIN_LIST_LEN];
    static const uint8_t BORDER_LOWER_LEFT_PIN_LIST[BORDER_PIN_LIST_LEN];
    static const uint8_t BORDER_UPPER_LEFT_PIN_LIST[BORDER_PIN_LIST_LEN];
    
    static const uint8_t BORDER_PIN_LIST_LIST_LEN = 4;
    static const uint8_t *BORDER_PIN_LIST_LIST[BORDER_PIN_LIST_LIST_LEN];
};

















#endif  // __APP_EVIL_EYE_H__