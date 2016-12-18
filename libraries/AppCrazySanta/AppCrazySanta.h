#ifndef __APP_CRAZY_SANTA_H__
#define __APP_CRAZY_SANTA_H__


#include "Evm.h"
#include "Utl.h"
#include "TimedEventHandler.h"
#include "ServoController.h"
#include "LedFader.h"
#include "SensorUltrasonicRangeFinder.h"



struct AppCrazySantaConfig
{
    uint8_t pinServoLeftArm;
    uint8_t pinServoRightArm;
    uint8_t pinServoHead;
    
    uint8_t pinLedLeftEye;
    uint8_t pinLedRightEye;
    
    uint8_t pinRangeFinder;
};

class AppCrazySanta
{
private:
    static const uint8_t  LIMIT_DEG_LEFT_ARM_LOW   =  90;
    static const uint8_t  LIMIT_DEG_LEFT_ARM_HIGH  = 180;
    static const uint8_t  DEG_LEFT_ARM_START       = LIMIT_DEG_LEFT_ARM_HIGH;
    static const uint8_t  LIMIT_DEG_RIGHT_ARM_LOW  =   0;
    static const uint8_t  LIMIT_DEG_RIGHT_ARM_HIGH =  70;
    static const uint8_t  DEG_RIGHT_ARM_START      = LIMIT_DEG_RIGHT_ARM_LOW;
    static const uint8_t  LIMIT_DEG_HEAD_LOW       =  60;
    static const uint8_t  LIMIT_DEG_HEAD_HIGH      = 135;
    static const uint8_t  DEG_HEAD_START           =  70;
    static const uint8_t  LIMIT_MS_EYE_MS_LOW      = 100;
    static const uint16_t LIMIT_MS_EYE_MS_HIGH     = 600;
    
    static const uint32_t RETURN_TO_START_POS_MS   = 500;
    
    static const uint32_t RANGE_FINDER_POLL_MS     = 100;
    static const uint8_t  RANGE_FINDER_LIMIT_IN    = 24;    // 2 feet
    
    using LedFaderClass = LEDFader<1,0>;
    
    static const uint8_t C_IDLE  = ServoController::C_IDLE  * 3 +
                                   LedFaderClass::C_IDLE * 2;
    static const uint8_t C_TIMED = ServoController::C_TIMED * 3 +
                                   LedFaderClass::C_TIMED * 2 +
                                   1;   // TimedEventHandlerDelegate
    static const uint8_t C_INTER = ServoController::C_INTER * 3 +
                                   LedFaderClass::C_INTER * 2;
    
public:
    AppCrazySanta(AppCrazySantaConfig &cfg)
    : scLeftArm_(cfg.pinServoLeftArm)
    , scRightArm_(cfg.pinServoRightArm)
    , scHead_(cfg.pinServoHead)
    , rangeFinder_(cfg.pinRangeFinder)
    {
        ledFaderLeftEye_.AddLED(cfg.pinLedLeftEye);
        ledFaderRightEye_.AddLED(cfg.pinLedRightEye);
    }
    
    void Run()
    {
        FlashEyes();
        GoToStartingPosition();
        StopAll();
        
        StartRangeFinder();
        
        evm_.MainLoop();
    }

private:

    void FlashEyes()
    {
        ledFaderLeftEye_.FadeForever(LIMIT_MS_EYE_MS_HIGH / 3);
        ledFaderRightEye_.FadeForever(LIMIT_MS_EYE_MS_HIGH);
        
        evm_.HoldStackDangerously(LIMIT_MS_EYE_MS_HIGH);
        
        ledFaderLeftEye_.Stop();
        ledFaderRightEye_.Stop();
    }
    
    
    void SelectCrazy(uint32_t durationMs)
    {
        uint32_t degLeftArm = GetRandomInRange(LIMIT_DEG_LEFT_ARM_LOW,
                                               LIMIT_DEG_LEFT_ARM_HIGH);
        uint32_t degRightArm = GetRandomInRange(LIMIT_DEG_RIGHT_ARM_LOW,
                                                LIMIT_DEG_RIGHT_ARM_HIGH);
        uint32_t degHead = GetRandomInRange(LIMIT_DEG_HEAD_LOW,
                                            LIMIT_DEG_HEAD_HIGH);
        uint32_t msEyeLeft = GetRandomInRange(LIMIT_MS_EYE_MS_LOW,
                                              LIMIT_MS_EYE_MS_HIGH);
        uint32_t msEyeRight = GetRandomInRange(LIMIT_MS_EYE_MS_LOW,
                                               LIMIT_MS_EYE_MS_HIGH);
                                               
        scLeftArm_.MoveTo(degLeftArm);
        scRightArm_.MoveTo(degRightArm);
        scHead_.MoveTo(degHead);
        ledFaderLeftEye_.FadeForever(msEyeLeft);
        ledFaderRightEye_.FadeForever(msEyeRight);
        
        evm_.HoldStackDangerously(durationMs);
    }
    
    void StopAll()
    {
        scLeftArm_.Stop();
        scRightArm_.Stop();
        scHead_.Stop();
        
        ledFaderLeftEye_.Stop();
        ledFaderRightEye_.Stop();
    }
    
    void GoToStartingPosition()
    {
        scLeftArm_.MoveTo(DEG_LEFT_ARM_START);
        scRightArm_.MoveTo(DEG_RIGHT_ARM_START);
        scHead_.MoveTo(DEG_HEAD_START);
        
        evm_.HoldStackDangerously(RETURN_TO_START_POS_MS);
    }
    
    void GoCrazy()
    {
        StopRangeFinder();

        const uint32_t FIT_DURATION_MS = 400;
        const uint8_t  FIT_COUNT       = 15;
        
        FlashEyes();
        StopAll();
        
        for (uint8_t i = 0; i < FIT_COUNT; ++i)
        {
            SelectCrazy(FIT_DURATION_MS);
        }
        
        GoToStartingPosition();
        StopAll();
        FlashEyes();
        FlashEyes();
        StopAll();
        
        StartRangeFinder();
    }
    
    void StartRangeFinder()
    {
        ted_.SetCallback([this](){ OnGetRangeFinderMeasurement(); });
        
        ted_.RegisterForTimedEventInterval(RANGE_FINDER_POLL_MS);
    }
    
    void StopRangeFinder()
    {
        ted_.DeRegisterForTimedEvent();
    }
    
    void OnGetRangeFinderMeasurement()
    {
        // Get Measurement
        SensorUltrasonicRangeFinder::Measurement m;

        rangeFinder_.GetMeasurement(&m);
        
        // Check if within range to warrant going crazy
        if (m.distIn <= RANGE_FINDER_LIMIT_IN)
        {
            GoCrazy();
        }
    }
    
    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;
    
    ServoController scLeftArm_;
    ServoController scRightArm_;
    ServoController scHead_;
    
    LedFaderClass ledFaderLeftEye_;
    LedFaderClass ledFaderRightEye_;
    
    TimedEventHandlerDelegate   ted_;
    SensorUltrasonicRangeFinder rangeFinder_;
};



#endif  // __APP_CRAZY_SANTA_H__