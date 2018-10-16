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
    static const uint8_t C_INTER = 20;
    
    
private:
    
    static const uint8_t ROW_COUNT = 14;
    static const uint8_t COL_COUNT = 4;
    
    static const uint8_t CAP_I2C_ADDR = 0x28;

    
public:
    AppEvilEye(AppEvilEyeConfig &cfg)
    : cfg_(cfg)
    , cap_(CAP_I2C_ADDR)
    , mux_(        
        { 15, 14, 16, 17, 18, 19, 9, 10, 23, 24, 25, 26, 4, 5 },
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
        
        // Start up LEDs
        mux_.Start();
        
        
        // Handle async events
        Log(P("Running"));
        PAL.Delay(1000);

        evm_.MainLoop();
    }
    
    
private:

    void SetupShell()
    {
        shell_.RegisterCommand("start", [this](char *){
            Log("Starting");
            mux_.Start();
        });

        shell_.RegisterCommand("stop", [this](char *){
            Log("Stopping");
            mux_.Stop();
        });
        
        shell_.RegisterCommand("on", [this](char *){
            Log("On");
            LedSetAllOn();
        });
        
        shell_.RegisterCommand("off", [this](char *){
            Log("Off");
            LedSetAllOff();
        });

        shell_.RegisterCommand("us", [this](char *cmdStr){
            Str str(cmdStr);

            if (str.TokenCount(' ') == 2)
            {
                uint32_t us = atol(str.TokenAtIdx(1, ' '));

                Log("us = ", us);
                mux_.SetRowIntervalUs(us);
            }
        });
        
        shell_.RegisterCommand("rampup", [this](char *cmdStr){
            Str str(cmdStr);

            if (str.TokenCount(' ') == 2)
            {
                uint32_t ms = atol(str.TokenAtIdx(1, ' '));

                Log("Ramp Up at ", ms, " ms");
                
                DoAnimationRampUp(ms);
            }
        });
        
        shell_.RegisterCommand("rampdn", [this](char *cmdStr){
            Str str(cmdStr);

            if (str.TokenCount(' ') == 2)
            {
                uint32_t ms = atol(str.TokenAtIdx(1, ' '));

                Log("Ramp Down at ", ms, " ms");
                
                DoAnimationRampDown(ms);
            }
        });
        
        
        shell_.Start();
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
        
        uint8_t delayMs = 0;
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
        
        uint8_t delayMs = 0;
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