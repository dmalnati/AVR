#ifndef __TEST_HARNESS_328P_H__
#define __TEST_HARNESS_328P_H__


#include <PAL.h>
#include <Evm.h>
#include <LedFader.h>


template<uint8_t C_IDLE = 20, uint8_t C_TIMED = 20, uint8_t C_INTER = 20>
class TestHarness328P
{
private:
    using ThisClass    = TestHarness328P<C_IDLE, C_INTER, C_TIMED>;
    using PinToFn      = InterruptEventHandlerDelegate<ThisClass>;
    using RstOnTimeout = TimedEventHandlerDelegate<PlatformAbstractionLayer>;
    
public:
    static const uint32_t DEFAULT_FADER_DURATION_MS         =  600;
    static const uint32_t DEFAULT_HOLD_TO_RESET_DURATION_MS = 1000;

    TestHarness328P() : fadeDuration_(DEFAULT_FADER_DURATION_MS) { }
    virtual ~TestHarness328P() { }
    
    void SetFadeDuration(uint32_t fadeDuration) { fadeDuration_ = fadeDuration; }
    
    void Start()
    {
        Init();
        
        StartBlueFaderOnce();
        StartRedFaderOnce();
        StartGreenFaderOnce();
        
        evm_.MainLoop();
    }
    
    void Init()
    {
        // Set up LED Faders
        uint8_t pinBlueLED  = 12;
        uint8_t pinRedLED   = 13;
        uint8_t pinGreenLED = 14;
        
        faderBlue_. AddLED(pinBlueLED);
        faderRed_.  AddLED(pinRedLED);
        faderGreen_.AddLED(pinGreenLED);
        
        // Set up button handlers
        uint8_t pinButton1 = 17;
        uint8_t pinButton2 = 16;
        uint8_t pinButton3 = 15;
        
        ptfButton1_.SetCallback(this, &ThisClass::OnButton1Private);
        ptfButton2_.SetCallback(this, &ThisClass::OnButton2Private);
        ptfButton3_.SetCallback(this, &ThisClass::OnButton3Private);
        
        ptfButton1_.RegisterForInterruptEvent(pinButton1,
                                              LEVEL_RISING_AND_FALLING);
        ptfButton2_.RegisterForInterruptEvent(pinButton2);
        ptfButton3_.RegisterForInterruptEvent(pinButton3);

        // Set up ResetOnTimeout handler
        rot_.SetCallback(&PAL, &PlatformAbstractionLayer::SoftReset);
    }
    
    virtual void OnButton1() { StartGreenFaderOnce(); }
    virtual void OnButton2() { StartRedFaderOnce();   }
    virtual void OnButton3() { StartBlueFaderOnce();  }
    
    void    StartBlueFaderForever()  { faderBlue_.FadeForever(fadeDuration_);  }
    void    StartBlueFaderOnce()     { faderBlue_.FadeOnce(fadeDuration_);     }
    void    StopBlueFader()          { faderBlue_.Stop();                      }
    uint8_t IsActiveBlueFader()      { return faderBlue_.IsActive();           }

    void    StartRedFaderForever()   { faderRed_.FadeForever(fadeDuration_);   }
    void    StartRedFaderOnce()      { faderRed_.FadeOnce(fadeDuration_);      }
    void    StopRedFader()           { faderRed_.Stop();                       }
    uint8_t IsActiveRedFader()       { return faderRed_.IsActive();            }

    void    StartGreenFaderForever() { faderGreen_.FadeForever(fadeDuration_); }
    void    StartGreenFaderOnce()    { faderGreen_.FadeOnce(fadeDuration_);    }
    void    StopGreemFader()         { faderGreen_.Stop();                     }
    uint8_t IsActiveGreenFader()     { return faderGreen_.IsActive();          }
    
private:
    void OnButton1Private(uint8_t logicLevel)
    {
        if (logicLevel == 1)
        {
            // Queue timer to reset if this is in fact a reset
            rot_.RegisterForTimedEvent(DEFAULT_HOLD_TO_RESET_DURATION_MS);
            
            OnButton1();
        }
        else
        {
            // I guess it wasn't a reset.  Cancel timeout.
            rot_.DeRegisterForTimedEvent();
        }
    }
    
    void OnButton2Private(uint8_t) { OnButton2(); }
    void OnButton3Private(uint8_t) { OnButton3(); }
    
private:
    static const uint8_t C_IDLE_INTERNAL  = C_IDLE;
    static const uint8_t C_INTER_INTERNAL = C_INTER + 3;    // 3 buttons
    static const uint8_t C_TIMED_INTERNAL = C_TIMED + 1;    // 1 ROT

    Evm::Instance<C_IDLE_INTERNAL,
                  C_INTER_INTERNAL,
                  C_TIMED_INTERNAL> evm_;
    
    uint32_t fadeDuration_;
    
    LEDFader<1,1> faderBlue_;
    LEDFader<1,1> faderRed_;
    LEDFader<1,1> faderGreen_;
    
    PinToFn ptfButton1_;
    PinToFn ptfButton2_;
    PinToFn ptfButton3_;
    
    RstOnTimeout rot_;
};


#endif // __TEST_HARNESS_328P_H__












