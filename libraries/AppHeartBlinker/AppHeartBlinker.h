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
    
    void Run()
    {
        while (1)
        {
            uint32_t pattern = GetRandomInRange(1, 5);
            
            switch (pattern)
            {
                case 1: PatternLoopRight();           break;
                case 2: PatternLoopLeft();            break;
                case 3: PatternVerticalTopToBottom(); break;
                case 4: PatternVerticalBottomToTop(); break;
                case 5: PatternPulseAll();            break;
                
                default: PatternPulseAll(); break;
            }
        }
    }
    

private:
    // Calculate sizing values for Evm
    static const uint8_t LED_COUNT = 16;
    
    static const uint8_t C_IDLE  = 
        LED_COUNT * LEDFader<1,1>::C_IDLE  +
                    LEDFader<LED_COUNT, 1>::C_IDLE;
    static const uint8_t C_TIMED =
        LED_COUNT * LEDFader<1,1>::C_TIMED +
                    LEDFader<LED_COUNT, 1>::C_TIMED;
    static const uint8_t C_INTER =
        LED_COUNT * LEDFader<1,1>::C_INTER +
                    LEDFader<LED_COUNT, 1>::C_INTER;
    
    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;
    
private:
    static const uint32_t DEFAULT_FADER_DURATION_RANGE_LOW_MS  = 500;
    static const uint32_t DEFAULT_FADER_DURATION_RANGE_HIGH_MS = 750;
    static const uint32_t DEFAULT_LAP_DURATION_RANGE_LOW_MS    = 1000;
    static const uint32_t DEFAULT_LAP_DURATION_RANGE_HIGH_MS   = 3000;
    
    void Setup()
    {
        // Fill list of LEDFaders one-by-one
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
        // Add to list of LedFaders
        LEDFader<1,1> *ledFader = ledFaderList_.PushNew();
        
        if (ledFader)
        {
            ledFader->AddLED(pin);
        }
        
        // Add to super LedFader also
        ledFaderAll_.AddLED(pin);
    }

    uint32_t GetRandomInRange(uint32_t rangeLow, uint32_t rangeHigh)
    {
        uint32_t retVal = 0;
        
        srand(PAL.Millis());
        
        retVal = rangeLow + (rand() % (rangeHigh - rangeLow + 1));
        
        return retVal;
    }
    
    uint32_t GetRandomLapDuration()
    {
        return GetRandomInRange(DEFAULT_LAP_DURATION_RANGE_LOW_MS,
                                DEFAULT_LAP_DURATION_RANGE_HIGH_MS);
    }
    
    uint32_t GetRandomFadeDuration()
    {
        return GetRandomInRange(DEFAULT_FADER_DURATION_RANGE_LOW_MS,
                                DEFAULT_FADER_DURATION_RANGE_HIGH_MS);
    }
    
    void StopAllFaders()
    {
        for (uint8_t i = 0; i < ledFaderList_.Size(); ++i)
        {
            ledFaderList_[i].Stop();
        }
        
        ledFaderAll_.Stop();
    }

    
    ///////////////////////////////////////////////////////////////////////////
    //
    // Loop Right / Left
    //
    ///////////////////////////////////////////////////////////////////////////

    void PatternLoopRight()
    {
        const uint32_t fadeDurationMs = GetRandomFadeDuration();
        const uint32_t lapDurationMs  = GetRandomLapDuration();
        
        StopAllFaders();
        
        // Calculate delay between subsequent fade begin
        uint32_t durationBeforeNextStart =
            (float)(lapDurationMs - fadeDurationMs) /
            (float)(LED_COUNT + 1);
        
        // Start on LED 1
        ledFaderList_[0].FadeOnce(fadeDurationMs);
        evm_.HoldStackDangerously(durationBeforeNextStart);
        
        for (uint8_t i = 1; i < LED_COUNT; ++i)
        {
            ledFaderList_[i].FadeOnce(fadeDurationMs);
            evm_.HoldStackDangerously(durationBeforeNextStart);
        }

        // End on LED 1
        ledFaderList_[0].FadeOnce(fadeDurationMs);
        evm_.HoldStackDangerously(fadeDurationMs);
        
        StopAllFaders();
    }
    
    void PatternLoopLeft()
    {
        const uint32_t fadeDurationMs = GetRandomFadeDuration();
        const uint32_t lapDurationMs  = GetRandomLapDuration();
        
        StopAllFaders();
        
        // Calculate delay between subsequent fade begin
        uint32_t durationBeforeNextStart =
            (float)(lapDurationMs - fadeDurationMs) /
            (float)(LED_COUNT + 1);
        
        // Start on LED 1
        ledFaderList_[0].FadeOnce(fadeDurationMs);
        evm_.HoldStackDangerously(durationBeforeNextStart);
        
        for (uint8_t i = LED_COUNT - 1; i > 0 ; --i)
        {
            ledFaderList_[i].FadeOnce(fadeDurationMs);
            evm_.HoldStackDangerously(durationBeforeNextStart);
        }

        // End on LED 1
        ledFaderList_[0].FadeOnce(fadeDurationMs);
        evm_.HoldStackDangerously(fadeDurationMs);
        
        StopAllFaders();
    }

    
    ///////////////////////////////////////////////////////////////////////////
    //
    // Vertical TopToBottom / BottomToTop
    //
    ///////////////////////////////////////////////////////////////////////////
    
    void PatternVerticalTopToBottom()
    {
        const uint32_t fadeDurationMs = GetRandomFadeDuration();
        const uint32_t lapDurationMs  = GetRandomLapDuration();
        
        StopAllFaders();
        
        // Calculate delay between subsequent fade begin
        uint32_t durationBeforeNextStart =
            (float)(lapDurationMs - fadeDurationMs) /
            ((float)(LED_COUNT / 2) + 1);
        
        // Start on the top middle
        ledFaderList_[0].FadeOnce(fadeDurationMs);
        evm_.HoldStackDangerously(durationBeforeNextStart);
        
        // Symmetrically work downward
        for (uint8_t i = 1, j = LED_COUNT - 1; i < (LED_COUNT / 2); ++i, --j)
        {
            ledFaderList_[i].FadeOnce(fadeDurationMs);
            ledFaderList_[j].FadeOnce(fadeDurationMs);
            evm_.HoldStackDangerously(durationBeforeNextStart);
        }
        
        // End on the bottom middle
        ledFaderList_[LED_COUNT / 2].FadeOnce(fadeDurationMs);
        evm_.HoldStackDangerously(fadeDurationMs);
        
        StopAllFaders();
    }
    
    void PatternVerticalBottomToTop()
    {
        const uint32_t fadeDurationMs = GetRandomFadeDuration();
        const uint32_t lapDurationMs  = GetRandomLapDuration();
        
        StopAllFaders();
        
        // Calculate delay between subsequent fade begin
        uint32_t durationBeforeNextStart =
            (float)(lapDurationMs - fadeDurationMs) /
            ((float)(LED_COUNT / 2) + 1);
        
        // Start on the bottom middle
        ledFaderList_[LED_COUNT / 2].FadeOnce(fadeDurationMs);
        evm_.HoldStackDangerously(durationBeforeNextStart);
        
        // Symmetrically work upward
        for (uint8_t i = (LED_COUNT / 2) - 1, j = (LED_COUNT / 2) + 1;
             i > 0;
             --i, ++j)
        {
            ledFaderList_[i].FadeOnce(fadeDurationMs);
            ledFaderList_[j].FadeOnce(fadeDurationMs);
            evm_.HoldStackDangerously(durationBeforeNextStart);
        }
        
        // End on the top middle
        ledFaderList_[0].FadeOnce(fadeDurationMs);
        evm_.HoldStackDangerously(fadeDurationMs);
        
        StopAllFaders();
    }
    
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // Pulse All
    //
    ///////////////////////////////////////////////////////////////////////////
    
    void PatternPulseAll()
    {
        const uint32_t fadeDurationMs = GetRandomFadeDuration();
        
        StopAllFaders();
        
        ledFaderAll_.FadeOnce(fadeDurationMs);
        evm_.HoldStackDangerously(fadeDurationMs);
        
        StopAllFaders();
    }

    
    
    AppHeartBlinkerConfig &cfg_;
    
    LEDFader<LED_COUNT, 1>                ledFaderAll_;
    ListInPlace<LEDFader<1,1>, LED_COUNT> ledFaderList_;
};


#endif  // __APP_HEART_BLINKER_H__











