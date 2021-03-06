#ifndef __TIMER_HELPER_H__
#define __TIMER_HELPER_H__


#include "PAL.h"


template <typename TimerClass>
class TimerHelper
{
public:

    // Function to configure timer, transparently how to you, such that:
    // - Channel A was nuked.
    //   - pins aren't outputting anything, nor events firing on it.
    // - Channel B was untouched.
    // - Overflow interrupt was untouched.
    // - Use Channel A interrupt to capture operation.
    // - On success, A will be able to fire as close to frequency as possible
    //  - can be faster or slower
    // - What you don't know:
    //   - what prescaler in use
    void SetInterruptFrequency(uint16_t frequency)
    {
        double   cpuFrequency                    = PAL.GetCpuFreq();
        double   durationUsPerRequestedFrequency = 1000000.0 / (double)frequency;
        uint16_t timerTickCount                  = TimerClass::GetTickCount();
        
        typename TimerClass::TimerPrescaler timerPrescalerList[TimerClass::GetTimerPrescalerListLen()]; 
        TimerClass::GetTimerPrescalerList(timerPrescalerList);
        
        Attempt bestAttempt;
        uint8_t loopedOnceAlready = 0;
        
        // Try all of them, whichever is the closest wins.
        for (auto timerPrescaler : timerPrescalerList)
        {
            Attempt a;
            
            a.timerPrescaler    = timerPrescaler;
            a.timerPrescalerVal = TimerClass::ConvertTimerPrescalerToValue(a.timerPrescaler);
            
            a.durationUsSingleTick = 1000000.0 / (cpuFrequency / a.timerPrescalerVal);
            
            a.tickCountPerRequestedFrequency =
                durationUsPerRequestedFrequency / a.durationUsSingleTick;
            
            // Now let's find out what the error would be at this value
            a.tickCount = (uint16_t)round(a.tickCountPerRequestedFrequency);
            
            if (a.tickCount <= timerTickCount)
            {
                // It can be done with this timer and prescaler.
                a.durationUsUsingTickCount = a.tickCount * a.durationUsSingleTick;
                a.frequencyUsingTickCount  = (uint16_t)(1000000.0 / a.durationUsUsingTickCount);
                
                // Compare error to any other values of error.  If this is
                // closer, use it.
                a.error = (uint16_t)abs((int32_t)frequency - (int32_t)a.frequencyUsingTickCount);
                
                if (!loopedOnceAlready || a.error < bestAttempt.error)
                {
                    loopedOnceAlready = 1;
                    
                    bestAttempt = a;
                }
            }
        }
        
        /*
        LogNNL("SetOverflowInterruptFrequency: ");
        Log(frequency);
        Log("Final best");
        bestAttempt.Print();
        Log();  Log();
        */
        
        // Store the best attempt for use later
        bestAttempt_ = bestAttempt;
        
        
        // Actually set up the timer now
        
        // Disable prior operation of Channel A
        TimerChannel *tcA = TimerClass::GetTimerChannelA();
        
        tcA->DeRegisterForInterrupt();
        tcA->UnSetInterruptHandler();
        tcA->SetCTCModeBehavior(TimerChannel::CTCModeBehavior::NONE);
        
        // Set up timer to operate at previously calculated speeds
        TimerClass::SetTimerPrescaler(bestAttempt_.timerPrescaler);
        TimerClass::SetTimerValue(0);
        
        // Wrap on reaching channel A value.
        // Channel B can still be used for PWM as long as you're ok with the top
        // value.
        TimerClass::SetTimerMode(TimerClass::TimerMode::FAST_PWM_TOP_OCRNA);
        
        uint8_t top = bestAttempt_.tickCount - 1;  // don't forget that TOP is TICKS-1
        tcA->SetValue(top);
    }
    
    uint16_t GetInterruptFrequency()
    {
        return bestAttempt_.frequencyUsingTickCount;
    }

private:
    
    struct Attempt
    {
        typename TimerClass::TimerPrescaler timerPrescaler;
        
        double    timerPrescalerVal              = 0.0;
        double    durationUsSingleTick           = 0.0;
        double    tickCountPerRequestedFrequency = 0.0;
        uint16_t  tickCount                      = 0;
        double    durationUsUsingTickCount       = 0.0;
        uint16_t  frequencyUsingTickCount        = 0;
        uint16_t  error                          = 0;
        
        void Print()
        {
            /*
            LogNNL("timerPrescalerVal: ");
            Log(timerPrescalerVal);
            LogNNL("durationUsSingleTick: ");
            Log(durationUsSingleTick);
            LogNNL("tickCountPerRequestedFrequency: ");
            Log(tickCountPerRequestedFrequency);
            LogNNL("tickCount: ");
            Log(tickCount);
            LogNNL("durationUsUsingTickCount: ");
            Log(durationUsUsingTickCount);
            LogNNL("frequencyUsingTickCount: ");
            Log(frequencyUsingTickCount);
            LogNNL("error: ");
            Log(error);
            */
        }
    };
    
    Attempt bestAttempt_;
};


#endif // __TIMER_HELPER_H__







