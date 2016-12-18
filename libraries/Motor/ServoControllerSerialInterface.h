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
                
                // steps and direction (positive or negative number)
                int32_t deg = atoi(inputStr);
                
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
        });
    }

private:
    static const uint8_t BUF_SIZE = 10;
    SerialBufReader<BUF_SIZE> sr_;
    
    T &sc_;
    HardwareSerial &s_;

    TimedEventHandlerDelegate ted_;
};


#endif  // __STERVO_CONTROLLER_SERIAL_INTERFACE_H__