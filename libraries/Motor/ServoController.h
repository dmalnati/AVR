#ifndef __SERVO_CONTROLLER_H__
#define __SERVO_CONTROLLER_H__


#include "PAL.h"
#include "TimedEventHandler.h"
#include "IdleTimeHiResTimedEventHandler.h"


class ServoController
{
public:
    static const uint8_t C_IDLE  = 1;
    static const uint8_t C_TIMED = 1;
    static const uint8_t C_INTER = 0;
    
private:
    static const uint8_t PERIOD_MS = 20;
    
    static const uint8_t DEFAULT_DEG_CURRENT = 90;
    
public:
    ServoController(uint8_t pin)
    : degCurrent_(DEFAULT_DEG_CURRENT)
    , isInverted_(0)
    , rangeLow_(0)
    , rangeHigh_(180)
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
    
    void MoveTo(uint8_t deg)
    {
        // Constrain to range
        if      (deg < rangeLow_)  { deg = rangeLow_;  }
        else if (deg > rangeHigh_) { deg = rangeHigh_; }
        
        // Invert if necessary
        if (isInverted_) { deg = 180 - deg; }
        
        // Assume instantaneous move.  Needed to allow for calculations with
        // timed MoveTo.
        degCurrent_ = deg;
        
        // Set up logic for simulating PWM
        // Two timers are used:
        // - A timer to go off every PERIOD_MS representing the start of the
        //   period.
        // - A timer going off within the period to end the duty cycle.
        
        // Calculate duty cycle time in microseconds.
        // Range between 0.5ms - 2.5ms to cover a 180 degree range of motion.
        uint32_t dutyCycleExpireUs = (0.5 + (((double)deg / 180.0) * 2)) * 1000.0;
        
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
        
        // Start this period manually
        cbFnOnPeriodStart();
    }
    
    void MoveTo(uint8_t deg, uint32_t durationMs)
    {
        if (durationMs < PERIOD_MS)
        {
            MoveTo(deg);
        }
        else
        {
            if (deg > 180)
            {
                deg = 180;
            }
            
            // Could be positive or negative
            double degDiff = deg - degCurrent_;
            
            // How many PERIOD_MS cycles will occur in this duration?
            // Will be greater than 1 given the if-condition above.
            // Equally likely to be fractional (1.7) vs whole (1.0).
            // We round to simply make a decision, don't care much if it's
            // early or late, both are really close (less than PERIOD_MS) wrong
            // and realistically won't matter much given that these aren't
            // precision devices nor precision timed.
            uint32_t periodCount = (uint32_t)round((double)durationMs /
                                                   (double)PERIOD_MS);
            
            
            // Calculate the number of degrees to change each period.
            // This could just as likely be 0.1 or 50.
            degStepSize_ = degDiff / periodCount;
            
            // Set up state for applying these changes every period.
            stepCurrent_ = 0;
            stepMax_     = periodCount;
            
            // Set up the end-of-duty-cycle callback
            auto cbFnOnDutyCycleEnd = [this]() {
                PAL.DigitalWrite(pin_, LOW);
            };
            timerDutyCycle_.SetCallback(cbFnOnDutyCycleEnd);

            // Set up the callback to fire when the period starts
            auto cbFnOnPeriodStart = [this]() {
                if (stepCurrent_ < stepMax_)
                {
                    // Need to increment the degree target
                    degCurrent_ += degStepSize_;
                    
                    // Keep track of number of steps taken
                    ++stepCurrent_;
                    
                    // Calculate duty cycle time in microseconds.
                    // Range between 0.5ms - 2.5ms to cover a 180 degree range
                    // of motion.
                    // Save in member variable to avoid re-calculation when
                    // final position hit and degree target no longer changing.
                    dutyCycleExpireUs_ = (uint32_t)
                        ((0.5 + ((degCurrent_ / 180.0) * 2)) * 1000.0);
                }
                else
                {
                    // No need to adjust the degree target.
                    // Will make use of the cached prior calculation of
                    // dutyCycleExpireUs_
                }
                
                // Re-start timer to end duty cycle.  Should have expired by now.
                timerDutyCycle_.RegisterForIdleTimeHiResTimedEvent(dutyCycleExpireUs_);
                
                // Begin the period by starting the duty cycle
                PAL.DigitalWrite(pin_, HIGH);
            };
            timerPeriod_.SetCallback(cbFnOnPeriodStart);
            
            // Start the timer so it fires at the next (and subsequent) periods.
            timerPeriod_.RegisterForTimedEventInterval(PERIOD_MS);
            
            // Start this period manually
            cbFnOnPeriodStart();
        }
    }
    
    void Stop()
    {
        PAL.DigitalWrite(pin_, LOW);
        
        timerPeriod_.DeRegisterForTimedEvent();
        timerDutyCycle_.DeRegisterForIdleTimeHiResTimedEvent();
    }


private:

    uint8_t pin_;
    
    double   degCurrent_;
    double   degStepSize_;
    uint32_t stepCurrent_;
    uint32_t stepMax_;
    uint32_t dutyCycleExpireUs_;
    
    uint8_t isInverted_;
    uint8_t rangeLow_;
    uint8_t rangeHigh_;

    TimedEventHandlerDelegate              timerPeriod_;
    IdleTimeHiResTimedEventHandlerDelegate timerDutyCycle_;
};


#endif  // __SERVO_CONTROLLER_H__







