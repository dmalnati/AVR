#ifndef __APP_CRAZY_SANTA_H__
#define __APP_CRAZY_SANTA_H__


#include "Evm.h"
#include "ServoController.h"



/*

head
- 90+ turns left
- less turns right


left arm
- down - higher


left arm
- down - lower


 */

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
    }
    
public:
    SantaArm(uint8_t     pin,
             Orientation orientation,
             uint8_t     top = 180,
             uint8_t     bottom = 0)
    : sc_(pin)
    , orientation_(orientation)
    , top_(top)
    , bottom_(bottom)
    , degCurrent_(0)
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




struct AppCrazySantaConfig
{
    uint8_t pinServoLeftArm;
    uint8_t pinServoRightArm;
    uint8_t pinServoHead;
};

class AppCrazySanta
{
public:
    static const uint8_t C_IDLE  = ServoController::C_IDLE  * 3;
    static const uint8_t C_TIMED = ServoController::C_TIMED * 3;
    static const uint8_t C_INTER = ServoController::C_INTER * 3;
    
public:
    AppCrazySanta(AppCrazySantaConfig &cfg)
    : scLeftArm_(cfg.pinServoLeftArm)
    , scRightArm_(cfg.pinServoRightArm)
    , scHead_(cfg.pinServoHead)
    {
    }
    
    void Run()
    {
        evm_.MainLoop();
    }

private:

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
        
        
        
        
    }
    
    void StopCrazy()
    {
        // arms down
        // head centered
        // power off servos
        
        // start motion sensing
    }
    
    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;
    
    ServoController scLeftArm_;
    ServoController scRightArm_;
    ServoController scHead_;
};



#endif  // __APP_CRAZY_SANTA_H__