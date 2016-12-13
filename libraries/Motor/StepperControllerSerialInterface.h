#ifndef __STEPPER_CONTROLLER_SERIAL_INTERFACE_H__
#define __STEPPER_CONTROLLER_SERIAL_INTERFACE_H__


#include "Utl.h"
#include "StepperController.h"


template <typename T>
class StepperControllerSerialIface
{
public:
    StepperControllerSerialIface(T &sc, HardwareSerial &s)
    : sr_(s)
    , sc_(sc)
    , s_(s)
    {
        // Nothing to do
    }

    // called after the Serial stream init'd
    void Init()
    {
        s_.println("Starting");
        
        ted_.RegisterForTimedEventInterval(250);
        ted_.SetCallback([this](){
            char *inputStr = sr_.GetBuf();

            if (inputStr)
            {
                char *stepCountStr = inputStr;
                
                uint8_t doHalfSteps = 1;
                
                // Check for leading 'f' character to indicate that
                // Full steps are required
                if (inputStr[0] == 'f')
                {
                    doHalfSteps = 0;
                    
                    // advance to step count part of string
                    stepCountStr = strchr(inputStr, ' ');
                    ++stepCountStr;
                }
                
                // steps and direction (positive or negative number)
                int32_t stepCount = atoi(stepCountStr);

                // delayMs (optional)
                uint32_t delayMs = 1;
                char *delayMsStr = strchr(stepCountStr, ' ');
                if (delayMsStr)
                {
                    // move past the space
                    ++delayMsStr;

                    delayMs = atoi(delayMsStr);
                }
                
                // Call controller
                if (stepCount >= 0)
                {
                    if (doHalfSteps) { sc_.HalfStepCW(stepCount, delayMs); }
                    else             { sc_.FullStepCW(stepCount, delayMs); }
                }
                else
                {
                    if (doHalfSteps) { sc_.HalfStepCCW(-stepCount, delayMs); }
                    else             { sc_.FullStepCCW(-stepCount, delayMs); }
                }
            }
        });
    }

private:
    static const uint8_t BUF_SIZE = 10;
    SerialBufReader<BUF_SIZE> sr_;
    
    T &sc_;
    HardwareSerial &s_;

    TimedEventHandlerDelegate ted_;
};


#endif  // __STEPPER_CONTROLLER_SERIAL_INTERFACE_H__