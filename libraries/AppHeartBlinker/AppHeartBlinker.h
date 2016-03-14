#ifndef __APP_HEART_BLINKER_H__
#define __APP_HEART_BLINKER_H__

#include "Evm.h"
#include "LedFader.h"

struct AppHeartBlinkerConfig
{
    uint8_t pinLED1;
    uint8_t pinLED2;
    uint8_t pinLED3;
    uint8_t pinLED4;
    uint8_t pinLED5;
    uint8_t pinLED6;
    uint8_t pinLED7;
    uint8_t pinLED8;
    uint8_t pinLED9;
    uint8_t pinLED10;
    uint8_t pinLED11;
    uint8_t pinLED12;
    uint8_t pinLED13;
    uint8_t pinLED14;
    uint8_t pinLED15;
    uint8_t pinLED16;
};

class AppHeartBlinker
{
public:
    AppHeartBlinker(AppHeartBlinkerConfig &cfg)
    : cfg_(cfg)
    {
        Setup();
    }
    
    ~AppHeartBlinker() { }
    
    
    uint8_t GetNextPattern(uint8_t rangeLow, uint8_t rangeHigh)
    {
        uint8_t retVal = 0;
        
        srand(PAL.Millis());
        
        retVal = rangeLow + (rand() % rangeHigh);
        
        return retVal;
    }

    void Run()
    {
        while (1)
        {
            uint8_t pattern = GetNextPattern(1, 1);
            
            switch (pattern)
            {
                case 1: Pattern1(); break;
                
                default: Pattern1(); break;
            }
        }
    }
    
    // Pattern:
    // - clockwise led trail
    // - takes x seconds to complete one lap
    // - meaning the final LED should start at duration-fadeTime
    // 
    void Pattern1()
    {
        const uint32_t LAP_DURATION_MS = 2000;
        
        StopAllFaders();
        
        // Calculate delay between subsequent fade begin
        uint8_t durationBeforeNextStart =
            (LAP_DURATION_MS - DEFAULT_FADER_DURATION_MS) / ledFaderList_.Size();
        
        uint8_t i = 0;
        while (i < ledFaderList_.Size())
        {
            ledFaderList_[i].FadeOnce(DEFAULT_FADER_DURATION_MS);
            
            if (i == (ledFaderList_.Size() - 1))
            {
                evm_.HoldStackDangerously(DEFAULT_FADER_DURATION_MS);
            }
            else
            {
                evm_.HoldStackDangerously(durationBeforeNextStart);
            }
            
            ++i;
        }
        
        StopAllFaders();
    }
    
    void StopAllFaders()
    {
        for (uint8_t i = 0; i < ledFaderList_.Size(); ++i)
        {
            ledFaderList_[i].Stop();
        }
    }

private:
    // Calculate sizing values for Evm
    static const uint8_t C_IDLE  = LEDFader<1,1>::C_IDLE  * 16;
    static const uint8_t C_TIMED = LEDFader<1,1>::C_TIMED * 16;
    static const uint8_t C_INTER = LEDFader<1,1>::C_INTER * 16;
    
    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;
    
private:
    static const uint32_t DEFAULT_FADER_DURATION_MS = 600;
    static const uint8_t  LED_COUNT                 = 16;
    
    void Setup()
    {
        AddLedPin(cfg_.pinLED1);
        AddLedPin(cfg_.pinLED2);
        AddLedPin(cfg_.pinLED3);
        AddLedPin(cfg_.pinLED4);
        AddLedPin(cfg_.pinLED5);
        AddLedPin(cfg_.pinLED6);
        AddLedPin(cfg_.pinLED7);
        AddLedPin(cfg_.pinLED8);
        AddLedPin(cfg_.pinLED9);
        AddLedPin(cfg_.pinLED10);
        AddLedPin(cfg_.pinLED11);
        AddLedPin(cfg_.pinLED12);
        AddLedPin(cfg_.pinLED13);
        AddLedPin(cfg_.pinLED14);
        AddLedPin(cfg_.pinLED15);
        AddLedPin(cfg_.pinLED16);
    }
    
    void AddLedPin(uint8_t pin)
    {
        LEDFader<1,1> *ledFader = ledFaderList_.PushNew();
        
        if (ledFader)
        {
            ledFader->AddLED(pin);
        }
    }


    AppHeartBlinkerConfig &cfg_;
    
    ListInPlace<LEDFader<1,1>, LED_COUNT> ledFaderList_;
};


#endif  // __APP_HEART_BLINKER_H__











