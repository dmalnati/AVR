#ifndef __SERVO_CONTROLLER_H__
#define __SERVO_CONTROLLER_H__


#include "PAL.h"
#include "TimedEventHandler.h"
#include "IdleTimeHiResTimedEventHandler.h"


class ServoController
{
public:
    static const uint8_t C_IDLE  = 1;
    static const uint8_t C_TIMED = 2;
    static const uint8_t C_INTER = 0;
    
private:
    static const uint8_t PERIOD_MS = 20;
    
    static const int8_t DEFAULT_DEG_CURRENT = -1;
    
public:
    ServoController(uint8_t pin)
    : isInverted_(0)
    , rangeLow_(0)
    , rangeHigh_(180)
    , maxDurationMs_(0)
    , degCurrent_(DEFAULT_DEG_CURRENT)
    , modeIgnoreMoveToCurrentPosition_(0)
    {
        Init(pin);
    }

    ServoController()
    : ServoController(0)
    {
        // Nothing to do
    }

    void Init(uint8_t pin)
    {
        pin_ = pin;
        
        PAL.PinMode(pin_, OUTPUT);
        PAL.DigitalWrite(pin_, LOW);
    }
    
    void SetModeInverted()    { isInverted_ = 1; }
    void SetModeNonInverted() { isInverted_ = 0; }
    
    void SetRange(uint8_t rangeLow, uint8_t rangeHigh)
    {
        if (rangeLow <= rangeHigh)
        {
            rangeLow_  = rangeLow;
            rangeHigh_ = rangeHigh;
        }
    }
    
    void SetMaxDurationMotorEnabled(uint32_t durationMs)
    {
        maxDurationMs_ = durationMs;
    }
    
    void SetModeIgnoreMoveToCurrentPosition()
    {
        modeIgnoreMoveToCurrentPosition_ = 1;
    }
    
    void MoveTo(uint8_t deg)
    {
        // Constrain to range
        if      (deg < rangeLow_)  { deg = rangeLow_;  }
        else if (deg > rangeHigh_) { deg = rangeHigh_; }
        
        // Invert if necessary
        if (isInverted_) { deg = 180 - deg; }
        
        // Check if we should ignore moves to the current position
        if (!modeIgnoreMoveToCurrentPosition_ ||
            (modeIgnoreMoveToCurrentPosition_ && degCurrent_ != deg))
        {
            // Assume instantaneous move.  Needed to allow for calculations with
            // timed MoveTo.
            degCurrent_ = deg;
            
            // Set up logic for simulating PWM
            // Two timers are used:
            // - A timer to go off every PERIOD_MS representing the start of the
            //   period.
            // - A timer going off within the period to end the duty cycle.
            
            // Calculate duty cycle time in microseconds.
            // Range between ~0.5ms - ~2.5ms to cover a 180 degree range of motion.
            uint32_t dutyCycleExpireUs = DegToUs(deg);
            
            // Set up the end-of-duty-cycle callback
            auto cbFnOnDutyCycleEnd = [&]() {
                PAL.DigitalWrite(pin_, LOW);
            };
            timerDutyCycle_.SetCallback(cbFnOnDutyCycleEnd);
            
            // Set up the callback to fire when the period starts
            auto cbFnOnPeriodStart = [&, dutyCycleExpireUs]() {
                // Re-start timer to end duty cycle.  Should have expired by now.
                timerDutyCycle_.RegisterForIdleTimeHiResTimedEvent(dutyCycleExpireUs);
                
                // Begin the period by starting the duty cycle
                PAL.DigitalWrite(pin_, HIGH);
            };
            timerPeriod_.SetCallback(cbFnOnPeriodStart);
            
            // Start the timer so it fires at the next (and subsequent) periods.
            timerPeriod_.RegisterForTimedEventInterval(PERIOD_MS);
            
            // Check if there is a max duration in effect
            if (maxDurationMs_ != 0)
            {
                maxDurationTimer_.SetCallback([this](){
                    Stop();
                });
                
                maxDurationTimer_.RegisterForTimedEvent(maxDurationMs_);
            }
        }
    }
    
    void Stop()
    {
        PAL.DigitalWrite(pin_, LOW);
        
        timerPeriod_.DeRegisterForTimedEvent();
        timerDutyCycle_.DeRegisterForIdleTimeHiResTimedEvent();
        
        maxDurationTimer_.DeRegisterForTimedEvent();
    }


private:

    /*
     * Servos have been seen to have a non-linear response to pulse duration.
     *
     * Basically the servos are linear in the first 90 degrees, and in the
     * second 90, but at different ratios.
     *
     * Through empirical testing, suitable proportions have been found and are
     * hard coded here.
     * 
     */
    uint16_t DegToUs(uint8_t deg)
    {
        uint16_t retVal = 0;

        if (deg <= 90)
        {
            uint16_t rangeLowUs  =  550;
            uint16_t rangeHighUs = 1296;
            
            uint16_t durationUs = rangeLowUs + (((double)deg / 90.0) * (rangeHighUs - rangeLowUs));
        
            retVal = durationUs;
        }
        else
        {
            uint16_t rangeLowUs  = 1296;
            uint16_t rangeHighUs = 2294;
            
            uint16_t durationUs = rangeLowUs + (((double)(deg - 90) / 90.0) * (rangeHighUs - rangeLowUs));

            retVal = durationUs;
        }
        
        return retVal;
    }


    uint8_t pin_;
    
    uint8_t isInverted_;
    
    uint8_t rangeLow_;
    uint8_t rangeHigh_;

    uint32_t                   maxDurationMs_;
    TimedEventHandlerDelegate  maxDurationTimer_;
    
    int16_t degCurrent_;
    uint8_t modeIgnoreMoveToCurrentPosition_;

    TimedEventHandlerDelegate              timerPeriod_;
    IdleTimeHiResTimedEventHandlerDelegate timerDutyCycle_;
};


#endif  // __SERVO_CONTROLLER_H__







