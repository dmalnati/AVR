#ifndef __STEPPER_CONTROLLER_H__
#define __STEPPER_CONTROLLER_H__


#include "PAL.h"
#include "Function.h"
#include "IdleTimeHiResTimedEventHandler.h"


/*
 * This represents one of the two H-Bridges in the L293D chip.
 *
 * In this chip, per-H-Bridge, there is an Enable pin which affects whether
 * current flow is going to be affected by the two driver inputs (S1, S2).
 *
 * S1 and S2 are used to control the flow of current and should be used together
 * to allow for current flow or not.
 *
 * For example:
 * When S1 is enabled, current flows from it.
 * When S1 is disabled, current flows into it.
 *
 * Therefore, to get the functionality desired, S1 and S2 should always be
 * opposite of one another to allow for current flow in a given direction.
 *
 *     enabled ->
 * ------ S1 -----
 *               |
 * ------ S2 -----
 *    <- disabled
 *
 */
class HBridgeL293D
{
public:
    HBridgeL293D(uint8_t pinEnable, uint8_t pinS1, uint8_t pinS2)
    : pinEnable_(pinEnable, LOW)
    , pinS1_(pinS1)
    , pinS2_(pinS2)
    {
        Disable();
        SetFlowDirectionTopToBottom();
    }

    void Enable()  { PAL.DigitalWrite(pinEnable_, HIGH);  }
    void Disable() { PAL.DigitalWrite(pinEnable_, LOW);   }

    void SetFlowDirectionTopToBottom()
    {
        PAL.DigitalWrite(pinS1_, HIGH);
        PAL.DigitalWrite(pinS2_, LOW);
    }
    
    void SetFlowDirectionBottomToTop()
    {
        PAL.DigitalWrite(pinS1_, LOW);
        PAL.DigitalWrite(pinS2_, HIGH);
    }
    
    void SetFlowDisabled()
    {
        PAL.DigitalWrite(pinS1_, LOW);
        PAL.DigitalWrite(pinS2_, LOW);
    }

private:
    Pin pinEnable_;
    Pin pinS1_;
    Pin pinS2_;
};



//
// https://www.youtube.com/watch?v=Qc8zcst2blU
//
class StepperControllerBipolar
{
public:
    StepperControllerBipolar(uint8_t pinEnable,
                             uint8_t pinPhase1S1,
                             uint8_t pinPhase1S2,
                             uint8_t pinPhase2S1,
                             uint8_t pinPhase2S2)
    : h1_(pinEnable, pinPhase1S1, pinPhase1S2)
    , h2_(pinEnable, pinPhase2S1, pinPhase2S2)
    , halfStepStateListIdx_(0)
    {
        h1_.Enable();
        h2_.Enable();
        HalfStepRight();        
    }
    
    void HalfStepLeft()
    {
        // Get new index into half step array
        if (halfStepStateListIdx_ == 0) { halfStepStateListIdx_ = 14; }
        else                            { halfStepStateListIdx_ -= 2; }
        
        H1(halfStepStateList_[halfStepStateListIdx_ + 0]);
        H2(halfStepStateList_[halfStepStateListIdx_ + 1]);
    }
    
    void HalfStepRight()
    {
        // Get new index into half step array
        halfStepStateListIdx_ = (halfStepStateListIdx_ + 2) % 16;
        
        H1(halfStepStateList_[halfStepStateListIdx_ + 0]);
        H2(halfStepStateList_[halfStepStateListIdx_ + 1]);
    }

    enum struct HState : uint8_t
    {
        FLOW_TOP_TO_BOTTOM = 0,
        FLOW_BOTTOM_TO_TOP,
        FLOW_DISABLED
    };
    
private:

    static void SetHState(HBridgeL293D &h, HState state)
    {
        switch (state)
        {
        case HState::FLOW_TOP_TO_BOTTOM: h.SetFlowDirectionTopToBottom(); break;
        case HState::FLOW_BOTTOM_TO_TOP: h.SetFlowDirectionBottomToTop(); break;
        case HState::FLOW_DISABLED     : h.SetFlowDisabled();             break;
            
        default: break;
        }
    }
    
    void H1(HState state) { SetHState(h1_, state); }
    void H2(HState state) { SetHState(h2_, state); }
    
    HBridgeL293D h1_;
    HBridgeL293D h2_;
    
    uint8_t halfStepStateListIdx_;
    
    constexpr static const HState halfStepStateList_[16] = {
        HState::FLOW_TOP_TO_BOTTOM, HState::FLOW_BOTTOM_TO_TOP,
        HState::FLOW_TOP_TO_BOTTOM, HState::FLOW_DISABLED,
        HState::FLOW_TOP_TO_BOTTOM, HState::FLOW_TOP_TO_BOTTOM,
        HState::FLOW_DISABLED,      HState::FLOW_TOP_TO_BOTTOM,
        HState::FLOW_BOTTOM_TO_TOP, HState::FLOW_TOP_TO_BOTTOM,
        HState::FLOW_BOTTOM_TO_TOP, HState::FLOW_DISABLED,
        HState::FLOW_BOTTOM_TO_TOP, HState::FLOW_BOTTOM_TO_TOP,
        HState::FLOW_DISABLED,      HState::FLOW_BOTTOM_TO_TOP
    };
};



template <class T>
class StepperControllerAsync
{
public:
    StepperControllerAsync(T &sc)
    : sc_(sc)
    , stepCount_(0)
    , stepDurationMs_(0)
    {
        // Nothing to do
    }
    
    void HalfStepLeft(uint32_t         stepCount,
                      uint32_t         stepDurationMs,
                      function<void()> cbFnOnComplete = [](){})
    {
        Start(Direction::LEFT, stepCount, stepDurationMs, cbFnOnComplete);
    }
    
    void HalfStepRight(uint32_t         stepCount,
                      uint32_t         stepDurationMs,
                      function<void()> cbFnOnComplete = [](){})
    {
        Start(Direction::RIGHT, stepCount, stepDurationMs, cbFnOnComplete);
    }
    
    void Stop()
    {
        ted_.DeRegisterForIdleTimeHiResTimedEvent();
        
        cbFnOnComplete_ = [](){};
    }
    
private:

    enum struct Direction : uint8_t
    {
        LEFT = 0,
        RIGHT
    };
    
    void Start(Direction        direction,
               uint32_t         stepCount,
               uint32_t         stepDurationMs,
               function<void()> cbFnOnComplete)
    {
        Stop();
        
        if (stepCount)
        {
            direction_      = direction;
            stepCount_      = stepCount;
            stepDurationMs_ = stepDurationMs;
            
            cbFnOnComplete_ = cbFnOnComplete;
            
            OnTimeout();
        }
        else
        {
            Finished();
        }
    }
    
    void Finished()
    {
        cbFnOnComplete_();
        
        Stop();
    }

    virtual void OnTimeout()
    {
        ted_.RegisterForIdleTimeHiResTimedEventInterval(stepDurationMs_ * 1000);
        ted_.SetCallback([this](){ OnTimeout(); });
        
        if (direction_ == Direction::LEFT) { sc_.HalfStepLeft();  }
        else                               { sc_.HalfStepRight(); }
        
        --stepCount_;
        
        if (!stepCount_)
        {
            Finished();
        }
    }

    T &sc_;
    
    Direction direction_;
    uint32_t  stepCount_;
    uint32_t  stepDurationMs_;
    
    function<void()> cbFnOnComplete_;
    
    IdleTimeHiResTimedEventHandlerDelegate ted_;
};




#endif  // __STEPPER_CONTROLLER_H__




































