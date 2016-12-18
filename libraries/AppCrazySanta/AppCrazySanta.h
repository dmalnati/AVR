#ifndef __APP_CRAZY_SANTA_H__
#define __APP_CRAZY_SANTA_H__


#include "Evm.h"
#include "ServoController.h"
#include "LedFader.h"



/*

head
- 90+ turns left
- less turns right


left arm
- down - higher


left arm
- down - lower


 */

 
#if 0
/*
 * Internally operate as if
 * - upward movement goes closer to 180 and
 * - downard movement goes closer to 0
 *
 */
class SantaArm
{
public:
    enum struct Orientation : uint8_t
    {
        NORMAL = 0,
        REVERSE
    };
    
public:
    SantaArm(uint8_t     pin,
             Orientation orientation,
             uint8_t     top = 180,
             uint8_t     bottom = 0)
    : orientation_(orientation)
    , top_(top)
    , bottom_(bottom)
    , degCurrent_(0)
    , sc_(pin)
    {
        LogicalMoveTo(0);
    }
    
    void MoveToTop(uint32_t durationMs)
    {
        
    }
    
    void MoveToBottom(uint32_t durationMs)
    {
        
    }
    
private:
    void LogicalMoveTo(uint8_t deg)
    {
        if (orientation_ == Orientation::NORMAL)
        {
            uint8_t degUsed = deg;
            
            if (deg >= top_)
            {
                degUsed = top_;
            }
            else if (deg <= bottom_)
            {
                degUsed = bottom_;
            }
            
            degCurrent_ = degUsed;
            
            sc_.MoveTo(degCurrent_);
        }
        else // orientation_ == Orientation::REVERSE
        {
            uint8_t degUsed = top_ - deg;
            
            if (deg >= top_)
            {
                degUsed = top_;
            }
            else if (deg <= bottom_)
            {
                degUsed = bottom_;
            }
            
            degCurrent_ = degUsed;

            
            
            
            sc_.MoveTo(top_ - degCurrent_)
        }
    }

    Orientation orientation_;
    
    uint8_t top_;
    uint8_t bottom_;
    
    uint8_t degCurrent_;
    
    ServoController sc_;
};
#endif










#include "PinInput.h"
#include "TimedEventHandler.h"


struct AppCrazySantaConfig
{
    uint8_t pinServoLeftArm;
    uint8_t pinServoRightArm;
    uint8_t pinServoHead;
    
    uint8_t pinLedLeftEye;
    uint8_t pinLedRightEye;
};

class AppCrazySanta
{
private:
    static const uint16_t FADE_DURATION_MS = 1000;
    
    using LedFaderClass = LEDFader<1,0>;
    
public:
    static const uint8_t C_IDLE  = ServoController::C_IDLE  * 3 +
                                   LedFaderClass::C_IDLE * 2;
    static const uint8_t C_TIMED = ServoController::C_TIMED * 3 +
                                   LedFaderClass::C_TIMED * 2;
    static const uint8_t C_INTER = ServoController::C_INTER * 3 +
                                   LedFaderClass::C_INTER * 2;
    
public:
    AppCrazySanta(AppCrazySantaConfig &cfg)
    : trigger_(9)
    , scLeftArm_(cfg.pinServoLeftArm)
    , scRightArm_(cfg.pinServoRightArm)
    , scHead_(cfg.pinServoHead)
    {
        ledFaderLeftEye_.AddLED(cfg.pinLedLeftEye);
        ledFaderRightEye_.AddLED(cfg.pinLedRightEye);
    }
    
    void Run()
    {
        trigger_.SetCallback([this](uint8_t){ StartCrazy(); });
        trigger_.Enable();
        
        StartupAnimation();
        
        evm_.MainLoop();
    }

private:

    void StartupAnimation()
    {
        ledFaderLeftEye_.FadeOnce(FADE_DURATION_MS);
        ledFaderRightEye_.FadeOnce(FADE_DURATION_MS);
        
        evm_.HoldStackDangerously(FADE_DURATION_MS);
    }

    void StartCrazy()
    {
        // stop motion sensing
        
        // startup animation (raise one arm, the other, swivel head?)
        //
        // select one of several fits
        //
        // Maybe there are different programs for:
        // - arms
        //   - both up and down at the same time
        //   - up and down opposite one another
        // - head
        // - eyes
        //   - on
        //   - fade together
        //   - fade alternating one another
        //   - fade at different speeds from one another
        //   - one on, the other fading
        //
        // And when there is a fit triggered, one of each of those programs is
        // selected, independently, and run in concert with one another.
        //
        
        trigger_.Disable();
        
        ledFaderLeftEye_.FadeForever(FADE_DURATION_MS);
        ledFaderRightEye_.FadeForever(FADE_DURATION_MS);

        
        for (uint8_t i = 0; i < 2; ++i)
        {
            #if 0
            TimedEventHandlerDelegate ted1_;
            TimedEventHandlerDelegate ted2_;
            
            ted1_.SetCallback([this](){ scLeftArm_.MoveTo(20);  });
            ted2_.SetCallback([this](){ scRightArm_.MoveTo(20); });
            
            // +7ms left arm
            ted1_.RegisterForTimedEvent(7);
            
            // +14ms right arm
            ted1_.RegisterForTimedEvent(14);
            #endif
            
            // immediate move
            scLeftArm_.MoveTo(20);
            scRightArm_.MoveTo(20);
            scHead_.MoveTo(45);
            
            
            
            evm_.HoldStackDangerously(500);
            
            
            scHead_.MoveTo(135);
            scLeftArm_.MoveTo(120);
            scRightArm_.MoveTo(120);
            evm_.HoldStackDangerously(500);
        
        }
        scLeftArm_.Stop();
        scRightArm_.Stop();
        scHead_.Stop();
        
        ledFaderLeftEye_.Stop();
        ledFaderRightEye_.Stop();
        
        trigger_.Enable();
    }
    
    void StopCrazy()
    {
        // arms down
        // head centered
        // power off servos
        
        // start motion sensing
    }
    
    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;
    
    PinInput trigger_;
    
    ServoController scLeftArm_;
    ServoController scRightArm_;
    ServoController scHead_;
    
    LedFaderClass ledFaderLeftEye_;
    LedFaderClass ledFaderRightEye_;
};



#endif  // __APP_CRAZY_SANTA_H__