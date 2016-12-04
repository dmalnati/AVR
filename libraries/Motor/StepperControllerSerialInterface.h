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
        
        ted_.RegisterForTimedEventInterval(100);
        ted_.SetCallback([this](){
            char *inputStr = sr_.GetBuf();

            if (inputStr)
            {
                // steps and direction (positive or negative number)
                int16_t stepCount = atoi(inputStr);

                // delayMs (optional)
                uint16_t delayMs = 1;
                char *delayMsStr = strchr(inputStr, ' ');
                if (delayMsStr)
                {
                    // move past the space
                    ++delayMsStr;

                    delayMs = atoi(delayMsStr);
                }

                // Call controller
                if (stepCount >= 0)
                {
                    sc_.HalfStepRight(stepCount, delayMs);
                }
                else
                {
                    sc_.HalfStepLeft(-stepCount, delayMs);
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