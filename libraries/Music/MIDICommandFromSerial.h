#ifndef __MIDI_COMMAND_FROM_SERIAL_H__
#define __MIDI_COMMAND_FROM_SERIAL_H__


#include "Function.h"

#include "TimedEventHandler.h"

#include "MIDICommand.h"
#include "MIDICommandParser.h"


class MIDICommandFromSerial
{
    static const uint8_t DEFAULT_POLL_PERIOD   = 10;
    static const uint8_t MAX_BYTES_PER_ATTEMPT = 6;
    
public:
    
    void SetCallbackOnMIDICommand(function<void(MIDICommand cmd)> cbFn)
    {
        cbFn_ = cbFn;
    }
    
    void Start()
    {
        ted_.SetCallback([this](){
            OnPoll();
        });
        
        ted_.RegisterForTimedEventInterval(DEFAULT_POLL_PERIOD);
    }
    
    void Stop()
    {
        ted_.DeRegisterForTimedEvent();
    }
    

private:

    void OnPoll()
    {
        // Attempt to extract a MIDICommand
        MIDICommand cmd;
        
        if (GetMIDICommand(&cmd))
        {
            cbFn_(cmd);
        }
    }
    
    uint8_t GetMIDICommand(MIDICommand *cmd)
    {
        uint8_t retVal    = 0;
        uint8_t bytesSeen = 0;
        
        if (cmd && S0.Available())
        {
            uint8_t cont = 1;
            
            while (cont)
            {
                uint8_t b = S0.Read();
                
                ++bytesSeen;
                
                mcp_.AddByte(b);

                if (mcp_.GetMsg(cmd))
                {
                    retVal = 1;
                    
                    cont = 0;
                }
                else
                {
                    if (S0.Available() == 0)
                    {
                        cont = 0;
                    }
                }
                
                // Don't allow self to get caught in endless stream of data
                if (bytesSeen == MAX_BYTES_PER_ATTEMPT)
                {
                    cont = 0;
                }
            }
        }
        
        return retVal;
    }


    MIDICommandParser mcp_;
    
    TimedEventHandlerDelegate ted_;

    function<void(MIDICommand cmd)> cbFn_;
};




#endif  // __MIDI_COMMAND_FROM_SERIAL_H__




