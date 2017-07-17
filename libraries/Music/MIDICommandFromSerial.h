#ifndef __MIDI_COMMAND_FROM_SERIAL_H__
#define __MIDI_COMMAND_FROM_SERIAL_H__


#include "MIDICommand.h"
#include "MIDICommandParser.h"


class MIDICommandFromSerial
{
    uint8_t MAX_BYTES_PER_ATTEMPT = 6;
    
public:
    
    void Init(MIDICommandParser *mcp)
    {
        mcp_ = mcp;
    }

    uint8_t GetMIDICommand(MIDICommand *cmd)
    {
        uint8_t retVal    = 0;
        uint8_t bytesSeen = 0;
        
        if (mcp_ && cmd && Serial.available())
        {
            uint8_t cont = 1;
            
            while (cont)
            {
                uint8_t b = Serial.read();
                
                ++bytesSeen;
                
                mcp_->AddByte(b);

                if (mcp_->GetMsg(cmd))
                {
                    retVal = 1;
                    
                    cont = 0;
                }
                else
                {
                    if (Serial.available() == 0)
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

private:
    MIDICommandParser *mcp_ = NULL;
};




#endif  // __MIDI_COMMAND_FROM_SERIAL_H__




