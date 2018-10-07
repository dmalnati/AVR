#ifndef __APP_GATE_CONTROLLER_H__
#define __APP_GATE_CONTROLLER_H__


#include "PAL.h"
#include "Log.h"
#include "SerialInput.h"
#include "Evm.h"
#include "PinInput.h"
#include "RFSI4463PRO.h"


struct AppGateControllerConfig
{
    uint16_t myGateCombo;
    uint32_t myGateFreq;
    
    uint8_t pinSlaveSelect;
    uint8_t pinShutdown;
    uint8_t pinOOK;
    
    uint8_t pinLed1;
    uint8_t pinLed2;
    uint8_t pinLed3;
    uint8_t pinLed4;
};


class AppGateController
{
    static const uint8_t C_IDLE  = 10;
    static const uint8_t C_TIMED = 10;
    static const uint8_t C_INTER = 10;

    
    // Defaults reflect my gate
    struct SendConfig
    {
        uint8_t codeSendCount       = 5;
        uint32_t delayBetweenCodeMs = 2;
        
        uint32_t bitDurationUs             = 1960;
        uint32_t bitHighDurationWhenHighUs = 1450;
        uint32_t bitHighDurationWhenLowUs  =  485;
    };
    
    
public:

    AppGateController(AppGateControllerConfig &cfg)
    : cfg_(cfg)
    , radio_(cfg_.pinSlaveSelect, cfg_.pinShutdown)
    {
        for (auto pin : (uint8_t[]){
            cfg_.pinOOK, 
            cfg_.pinLed1,
            cfg_.pinLed2,
            cfg_.pinLed3,
            cfg_.pinLed4
        })
        {
            PAL.PinMode(pin, OUTPUT);
            PAL.DigitalWrite(pin, LOW);
        }
    }
    
    void Run()
    {
        LogStart(9600);
        Log("Starting");
        
        // Start up Radio
        RadioStart();
        
        // Set up interactive controls
        SetupShellCommands();
        
        // Handle events
        Log("Running");
        evm_.MainLoop();
    }


private:

    ///////////////////////////////////////////////////////////////////////////
    //
    // Shell Commands
    //
    ///////////////////////////////////////////////////////////////////////////

    void SetupShellCommands()
    {
        shell_.RegisterCommand("codeSendCount", [this](char *cmdStr){
            Str str(cmdStr);
      
            if (str.TokenCount(' ') == 2)
            {
                sc_.codeSendCount = atol(str.TokenAtIdx(1, ' '));

                PrintConfig(sc_);
            }
        });
        
        shell_.RegisterCommand("delayBetweenCodeMs", [this](char *cmdStr){
            Str str(cmdStr);
      
            if (str.TokenCount(' ') == 2)
            {
                sc_.delayBetweenCodeMs = atol(str.TokenAtIdx(1, ' '));

                PrintConfig(sc_);
            }
        });
        
        shell_.RegisterCommand("brute", [this](char *){
            uint16_t low  = 0;
            uint16_t high = 0b0000001111111111;
            
            //SendCombinationRange(low, high);
            SendCombinationRangeAsync(low, high);
        });
        
        shell_.RegisterCommand("open", [this](char *){
            Open();
        });
        
        shell_.RegisterCommand("jam", [this](char *){
            Jam();
        });
        
        shell_.RegisterCommand("forceopen", [this](char *){
            //ForceOpen();
        });
        
        shell_.RegisterCommand("rfstart", [this](char *){
            RadioStart();
        });
        
        shell_.RegisterCommand("rfstop", [this](char *){
            RadioStop();
        });
        
        shell_.RegisterCommand("freq", [this](char *cmdStr){
            Str str(cmdStr);
      
            if (str.TokenCount(' ') == 2)
            {
                uint32_t freq = atol(str.TokenAtIdx(1, ' '));

                Log("Freq = ", freq);
                radio_.SetFrequency(freq);
            }
        });
        
        shell_.Start();
        
        PrintConfig(sc_);
    }
    
    void PrintConfig(SendConfig &sc)
    {
        Log("SendConfig");
        Log("  codeSendCount      = ", sc.codeSendCount);
        Log("  delayBetweenCodeMs = ", sc.delayBetweenCodeMs);
    }
    
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // Radio Control
    //
    ///////////////////////////////////////////////////////////////////////////
    
    void RadioStart()
    {
        radio_.Init();
        radio_.SetFrequency(cfg_.myGateFreq);
        radio_.SetModulationType(RFSI4463PRO::ModulationType::MT_OOK);
        radio_.Start();
    }
    
    void RadioStop()
    {
        radio_.Stop();
    }


    ///////////////////////////////////////////////////////////////////////////
    //
    // Basic Code Transmission Control - Sync
    //
    ///////////////////////////////////////////////////////////////////////////

    void SendCombinationRange(uint16_t low, uint16_t high)
    {
        for (uint16_t cur = low; cur <= high; ++cur)
        {
            SendCombination(cur, sc_.codeSendCount);
        }
    }
    
    void SendCombination(uint16_t combo10bit, uint16_t count)
    {
        for (uint8_t i = 0; i < count; ++i)
        {
            ClockOut10Bits(combo10bit);
            
            PAL.Delay(sc_.delayBetweenCodeMs);
        }
    }
    
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // Status LED Logic
    //
    ///////////////////////////////////////////////////////////////////////////
    
    void SetLEDs(uint8_t pct)
    {
        struct
        {
            uint8_t pctComplete;
            uint8_t pin;
        } pctHandlerList[] = {
            {  25, cfg_.pinLed1 },
            {  50, cfg_.pinLed2 },
            {  75, cfg_.pinLed3 },
            { 100, cfg_.pinLed4 },
        };
        
        for (auto pctHandler : pctHandlerList)
        {
            if (pct >= pctHandler.pctComplete)
            {
                PAL.DigitalWrite(pctHandler.pin, LOW);
            }
            else
            {
                PAL.DigitalWrite(pctHandler.pin, HIGH);
            }
        }
    }
    
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // Basic Code Transmission Control - Async
    //
    ///////////////////////////////////////////////////////////////////////////
    
    
    TimedEventHandlerDelegate ledTed_;
    
    void SendCombinationRangeAsync(uint16_t low, uint16_t high)
    {
        // Maintain state in some awful hack to avoid enlarging the function type
        struct State
        {
            uint16_t currentVal;
            uint16_t high;
            function<void()> cbFn;
        };
        
        static State state_;
        
        state_.currentVal = low;
        state_.high       = high;
        state_.cbFn = [this](){
            if (state_.currentVal <= state_.high)
            {
                //Log("SCRA Cont");
                SendCombinationAsync(state_.currentVal,
                                     sc_.codeSendCount,
                                     &state_.cbFn);
                
                ++state_.currentVal;
            }
            else
            {
                //Log("SCRA Stop");
                ledTed_();
                ledTed_.DeRegisterForTimedEvent();
            }
        };
        
        
        // Get LEDs set up
        ledTed_.SetCallback([=](){
            uint16_t used  = state_.currentVal - low;
            uint16_t range = high - low + 1;
            
            uint8_t pct = (double)used / (double)range * 100.0;
            
            SetLEDs(pct);
        });
        ledTed_();
        ledTed_.RegisterForTimedEventInterval(100, 0);
        
        // Begin
        state_.cbFn();
    }

    void SendCombinationAsync(uint16_t combo10bit,
                              uint16_t count,
                              function<void()> *cbFn)
    {
        static uint16_t countRemaining;
        
        countRemaining = count;
        
        sendCombinationTed_.SetCallback([=]() {
            if (countRemaining)
            {
                ClockOut10Bits(combo10bit);
                //Log("CO1B: ", combo10bit);
                
                --countRemaining;
            }
            else
            {
                //Log("SCA:DeRegister");
                sendCombinationTed_.DeRegisterForTimedEvent();
                
                (*cbFn)();
            }
        });
        
        sendCombinationTed_.RegisterForTimedEventInterval(sc_.delayBetweenCodeMs, 0);
    }

    
    ///////////////////////////////////////////////////////////////////////////
    //
    // Bit Representation
    //
    ///////////////////////////////////////////////////////////////////////////
    
    void ClockOut10Bits(uint16_t combo10bit)
    {
        uint16_t bitShiftBuf = combo10bit;
        uint8_t  bitSentCount = 0;
        
        uint32_t bitLowDurationWhenHighUs = sc_.bitDurationUs -
                                            sc_.bitHighDurationWhenHighUs;

        uint32_t bitLowDurationWhenLowUs = sc_.bitDurationUs -
                                           sc_.bitHighDurationWhenLowUs;
                                          
        while (bitSentCount < 10)
        {
            uint8_t onOrOff = bitShiftBuf & 0x0001;
            bitShiftBuf >>= 1;

            if (onOrOff)
            {
                PAL.DigitalWrite(cfg_.pinOOK, HIGH);
                PAL.DelayMicroseconds(sc_.bitHighDurationWhenHighUs);
                PAL.DigitalWrite(cfg_.pinOOK, LOW);
                PAL.DelayMicroseconds(bitLowDurationWhenHighUs);
            }
            else
            {
                PAL.DigitalWrite(cfg_.pinOOK, HIGH);
                PAL.DelayMicroseconds(sc_.bitHighDurationWhenLowUs);
                PAL.DigitalWrite(cfg_.pinOOK, LOW);
                PAL.DelayMicroseconds(bitLowDurationWhenLowUs);
            }

            ++bitSentCount;
        }
    }
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // High-Level Features
    //
    ///////////////////////////////////////////////////////////////////////////
    
    void Open()
    {
        SendCombinationRangeAsync(cfg_.myGateCombo, cfg_.myGateCombo);
    }

    void ForceOpen()
    {
    }
    
    void Jam()
    {
        PAL.DigitalToggle(cfg_.pinOOK);
        PAL.DelayMicroseconds(sc_.bitHighDurationWhenLowUs);
    }



private:

    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

    AppGateControllerConfig &cfg_;
    
    RFSI4463PRO radio_;
    
    SendConfig sc_;
    
    TimedEventHandlerDelegate sendCombinationTed_;

    SerialAsyncConsoleEnhanced<20> shell_;


};



































#endif  // __APP_GATE_CONTROLLER_H__


