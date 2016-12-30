#ifndef __STERVO_CONTROLLER_SERIAL_INTERFACE_H__
#define __STERVO_CONTROLLER_SERIAL_INTERFACE_H__


#include "Utl.h"
#include "ServoController.h"


template <typename T>
class ServoControllerSerialIface
{
public:
    ServoControllerSerialIface(T &sc, HardwareSerial &s)
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
        
        ted_.RegisterForTimedEventInterval(150);
        ted_.SetCallback([this](){
            char *inputStr = sr_.GetBuf();
            
            if (inputStr)
            {
                s_.print(inputStr);
                s_.println(" (input)");
                
                if (inputStr[0] == 'i')
                {
                    HandleInverseCommand();
                }
                else if (inputStr[0] == 'n')
                {
                    HandleNonInverseCommand();
                }
                else if (inputStr[0] == 'r')
                {
                    HandleRangeCommand(inputStr);
                }
                else
                {
                    HandleMoveCommand(inputStr);
                }
            }
        });
    }

private:

    void HandleInverseCommand()
    {
        sc_.SetModeInverted();
        
        Serial.println("Mode: Inverted");
    }
    
    void HandleNonInverseCommand()
    {
        sc_.SetModeNonInverted();
        
        Serial.println("Mode: NonInverted");
    }
    
    void HandleRangeCommand(char *inputStr)
    {
        char *rangeLowStr  = strchr(inputStr, ' ')    + 1;
        char *rangeHighStr = strchr(rangeLowStr, ' ') + 1;
        
        uint8_t rangeLow  = atoi(rangeLowStr);
        uint8_t rangeHigh = atoi(rangeHighStr);
        
        sc_.SetRange(rangeLow, rangeHigh);
        
        Serial.print("Range: ");
        Serial.print(rangeLow);
        Serial.print(" - ");
        Serial.println(rangeHigh);
    }
    
    void HandleMoveCommand(char *inputStr)
    {
        // degrees
        int32_t deg = atoi(inputStr);
        
        if (deg == -1)
        {
            sc_.Stop();
        }
        else
        {
            // search for optional duration parameter.  If present, we know
            // this will exercise the timed MoveTo function.
            char *durationMsStr = strchr(inputStr, ' ');
            
            if (durationMsStr)
            {
                uint32_t durationMs = atol(durationMsStr);
                
                sc_.MoveTo(deg, durationMs);
            }
            else
            {
                // Call controller
                sc_.MoveTo(deg);
            }
        }
    }

    static const uint8_t BUF_SIZE = 10;
    SerialBufReader<BUF_SIZE> sr_;
    
    T &sc_;
    HardwareSerial &s_;

    TimedEventHandlerDelegate ted_;
};


#endif  // __STERVO_CONTROLLER_SERIAL_INTERFACE_H__