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
public:
    
    static const uint8_t C_IDLE  = 20;
    static const uint8_t C_TIMED = 20;
    static const uint8_t C_INTER = 0;
    
    
private:
public:
    
    static const uint8_t ROW_COUNT = 14;
    static const uint8_t COL_COUNT = 4;
    
    static const uint8_t CAP_I2C_ADDR = 0x28;
    
    static const uint32_t DEFAULT_CAP_SENSE_POLL_PERIOD = 1500;
    
    static const uint8_t IRIS_PIN_LIST_LEN = ROW_COUNT;
    const uint8_t IRIS_PIN_LIST[IRIS_PIN_LIST_LEN] = { 4, 0, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52 };
    
    
    static const uint8_t BORDER_PIN_LIST_LEN = 5;
    const uint8_t BORDER_UPPER_RIGHT_PIN_LIST[BORDER_PIN_LIST_LEN] = {  1,  9, 25, 41, 10 };
    const uint8_t BORDER_LOWER_RIGHT_PIN_LIST[BORDER_PIN_LIST_LEN] = { 42, 11, 27, 43, 51 };
    const uint8_t BORDER_LOWER_LEFT_PIN_LIST[BORDER_PIN_LIST_LEN]  = { 55, 47, 31, 15, 46 };
    const uint8_t BORDER_UPPER_LEFT_PIN_LIST[BORDER_PIN_LIST_LEN]  = { 14, 45, 29, 13,  5 };
    
    
    
public:
    AppEvilEye(AppEvilEyeConfig &cfg)
    : cfg_(cfg)
    , cap_(CAP_I2C_ADDR)
    , mux_(        
        { 14, 15, 16, 17, 18, 19, 9, 10, 23, 24, 25, 26, 4, 5 },
        { 6, 11, 12, 13 }
      )
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
        
        // Set up capacitive touch sense
        SetupCapSense();
        
        // Start up LED renderer
        mux_.Start();
        
        // Say hi
        StartupAnimation();
        
        // Handle async events
        Log(P("Running"));
        PAL.Delay(1000);

        evm_.MainLoop();
    }
    
    
private:

    void StartupAnimation()
    {
        DoAnimationRampUp(200);
    }

    void SetupShell()
    {
        shell_.RegisterCommand("start", [this](char *){
            Log(P("Starting"));
            mux_.Start();
        });

        shell_.RegisterCommand("stop", [this](char *){
            Log(P("Stopping"));
            mux_.Stop();
        });
        
        shell_.RegisterCommand("on", [this](char *){
            Log(P("On"));
            LedSetAllOn();
        });
        
        shell_.RegisterCommand("off", [this](char *){
            Log(P("Off"));
            LedSetAllOff();
        });
        
        shell_.RegisterCommand("p", [this](char *cmdStr){
            Str str(cmdStr);

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
        });


        shell_.RegisterCommand("us", [this](char *cmdStr){
            Str str(cmdStr);

            if (str.TokenCount(' ') == 2)
            {
                uint32_t us = atol(str.TokenAtIdx(1, ' '));

                Log(P("us = "), us);
                mux_.SetRowIntervalUs(us);
            }
        });
        
        shell_.RegisterCommand("rampup", [this](char *cmdStr){
            Str str(cmdStr);

            if (str.TokenCount(' ') == 2)
            {
                uint32_t ms = atol(str.TokenAtIdx(1, ' '));

                Log(P("Ramp Up at "), ms, P(" ms"));
                
                DoAnimationRampUp(ms);
            }
        });
        
        shell_.RegisterCommand("rampdn", [this](char *cmdStr){
            Str str(cmdStr);

            if (str.TokenCount(' ') == 2)
            {
                uint32_t ms = atol(str.TokenAtIdx(1, ' '));

                Log(P("Ramp Down at "), ms, P(" ms"));
                
                DoAnimationRampDown(ms);
            }
        });
        
        shell_.RegisterCommand("iris", [this](char *cmdStr){
            Str str(cmdStr);

            if (str.TokenCount(' ') == 4)
            {
                uint32_t totalDurationMs  = atol(str.TokenAtIdx(1, ' '));
                uint8_t  forwardOrReverse = atoi(str.TokenAtIdx(2, ' '));
                uint8_t  onOff            = atoi(str.TokenAtIdx(3, ' '));

                Log(P("IrisRamp "), totalDurationMs, ", ", forwardOrReverse, ", ", onOff);
                
                DoIrisRamp(totalDurationMs, forwardOrReverse, onOff);
            }
        });
        
        shell_.RegisterCommand("border", [this](char *cmdStr){
            Str str(cmdStr);

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
                else
                {
                    Log(P("    Invalid side"));
                }
            }
        });

        shell_.RegisterCommand("sense", [this](char *){
            Log(P("Sense"));
            OnSense();
        });
        

        
        shell_.Start();
    }
    
    void SetupCapSense()
    {
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
    }
    
    
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
    
    void DoAnimationRampUp(uint32_t intervalBetweenSpeedsMs)
    {
        LedSetAllOn();
        
        fnChain_.Reset();
        
        uint32_t delayMs = 0;
        uint32_t rowIntervalUsList[] = { 50000, 40000, 30000, 20000, 10000, 5000, 2500, 1800, 500 };
        for (auto rowIntervalUs : rowIntervalUsList)
        {
            fnChain_.Append([=](){
                mux_.SetRowIntervalUs(rowIntervalUs);
            }, delayMs);
            
            delayMs = intervalBetweenSpeedsMs;
        }
        
        fnChain_.SetCallbackOnComplete([this](){ LedSetAllOff(); }, intervalBetweenSpeedsMs);

        fnChain_.Start();
    }
    
    void DoAnimationRampDown(uint32_t intervalBetweenSpeedsMs)
    {
        LedSetAllOn();
        
        fnChain_.Reset();
        
        uint32_t delayMs = 0;
        uint32_t rowIntervalUsList[] = { 500, 1800, 2500, 5000, 10000, 20000, 30000, 40000, 50000 };
        for (auto rowIntervalUs : rowIntervalUsList)
        {
            fnChain_.Append([=](){
                mux_.SetRowIntervalUs(rowIntervalUs);
            }, delayMs);
            
            delayMs = intervalBetweenSpeedsMs;
        }
        
        fnChain_.SetCallbackOnComplete([this](){ LedSetAllOff(); }, intervalBetweenSpeedsMs);

        fnChain_.Start();
    }
    
    
    
    
    
    void PathRamp(const uint8_t  *pinList,
                  uint8_t         pinListLen,
                  uint32_t        totalDurationMs,
                  uint8_t         forwardOrReverse,
                  uint8_t         onOff)
    {
        uint32_t intervalMs = (totalDurationMs / pinListLen);
        uint32_t delayMs    = 0;
        
        fnChain_.Reset();
        
        if (forwardOrReverse)
        {
            for (uint8_t i = pinListLen; i > 0; --i)
            {
                uint8_t pinIris = pinList[i - 1];
                
                fnChain_.Append([=](){
                    mux_.SetLedState(pinIris, onOff);
                }, delayMs);
                
                delayMs = intervalMs;
            }
        }
        else
        {
            for (uint8_t i = 0; i < pinListLen; ++i)
            {
                uint8_t pinIris = pinList[i];
                
                fnChain_.Append([=](){
                    mux_.SetLedState(pinIris, onOff);
                }, delayMs);
                
                delayMs = intervalMs;
            }
        }
        
        fnChain_.Start();
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
    
    

/*

Animations

Maybe run multiple types in parallel.
    eg maybe you have:
    - 3 different iris animations (w/ variable speeds)
    - 5 different sclera animations
    
    then you random determine which to run, and their params

    
    
Sclera animations:
- up/down/left/right wipe
- inner-to-outer and outer-to-inner wipe
- sparkle
- perimeter 
    



*/
    
    
    void OnSense()
    {
        Log(P("OnSense"));
        DoAnimationRampUp(100);
    }
    
    
    

    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

    AppEvilEyeConfig &cfg_;
    
    SensorCapacitiveTouchCAP1188 cap_;
    
    LedMatrixTimeMultiplexer<ROW_COUNT, COL_COUNT> mux_;
    
    static const uint8_t FN_CHAIN_LEN = 20;
    FunctionChainAsync<FN_CHAIN_LEN> fnChain_;

    
    
    static const uint8_t CONSOLE_CMD_COUNT = 20;
    SerialAsyncConsoleEnhanced<CONSOLE_CMD_COUNT> shell_;

    
};

















#endif  // __APP_EVIL_EYE_H__