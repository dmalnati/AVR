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
    
public:
    ServoController(uint8_t pin)
    {
        Init(pin);
    }

    ServoController()
    : pin_(0)
    {
        // Nothing to do
    }

    void Init(uint8_t pin)
    {
        pin_ = pin;
        
        PAL.PinMode(pin_, OUTPUT);
        PAL.DigitalWrite(pin_, LOW);
    }
    
    void MoveTo(uint8_t deg)
    {
        // Enforce 0 - 180 degree range
        if (deg > 180)
        {
            deg = 180;
        }
        
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
    
    void Stop()
    {
        PAL.DigitalWrite(pin_, LOW);
        
        timerPeriod_.DeRegisterForTimedEvent();
        timerDutyCycle_.DeRegisterForIdleTimeHiResTimedEvent();
    }


private:

    uint8_t pin_;

    TimedEventHandlerDelegate              timerPeriod_;
    IdleTimeHiResTimedEventHandlerDelegate timerDutyCycle_;
};


#endif  // __SERVO_CONTROLLER_H__







