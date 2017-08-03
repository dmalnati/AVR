#ifndef __MIDI_COMMAND_PARSER_H__
#define __MIDI_COMMAND_PARSER_H__


#include "Container.h"

#include "MIDICommand.h"


class MIDICommandParser
{
    static const uint8_t BUF_BYTES = 3;
public:

    // Allow for infinite stream of bytes to be shoved in.
    // It is the caller's responsibility to check after each byte that a
    // parsable message is available or not.
    void AddByte(uint8_t b)
    {
        if (!q_.Push(b))
        {
            uint8_t tmp;
            
            q_.Pop(tmp);
            q_.Push(b);
        }
    }
    
    uint8_t GetMsg(MIDICommand *cmd)
    {
        uint8_t retVal = 0;
        
        if (cmd && q_.Size())
        {
            // Look at first byte
            uint8_t firstByte = q_[0];
            
            // Check if high bit set, meaning command byte
            if (IsCommandByte(firstByte))
            {
                // Extract the 3 bits which indicate command type and use to
                // convert to enumerated type
                cmd->type = (MIDICommand::Type)((firstByte & 0b01110000) >> 4);
                    
                // Extract the 4 bits which determine the channel
                cmd->channel = (firstByte & 0b00001111);
                
                // Determine the number of parameters
                uint8_t parameterCount = 0;
                
                if (cmd->type ==  MIDICommand::Type::NOTE_OFF       ||
                    cmd->type ==  MIDICommand::Type::NOTE_ON        ||
                    cmd->type ==  MIDICommand::Type::AFTERTOUCH     ||
                    cmd->type ==  MIDICommand::Type::CONTROL_CHANGE ||
                    cmd->type ==  MIDICommand::Type::PITCH_BEND)
                {
                    parameterCount = 2;
                }
                else if (cmd->type == MIDICommand::Type::PROGRAM_CHANGE ||
                         cmd->type == MIDICommand::Type::CHANNEL_PRESSURE)
                {
                    parameterCount = 1;
                }
                else // MIDICommand::Type::NON_MUSICAL_COMMANDS
                {
                    // Do nothing
                }
                
                // Check if parameters are ready to be consumed
                // Don't forget that we still have the first byte in the queue.
                if (parameterCount)
                {
                    if (q_.Size() >= parameterCount + 1)
                    {
                        uint8_t ok = 1;
                        for (uint8_t i = 1; ok && i < parameterCount + 1; ++i)
                        {
                            // basically break on first failure
                            ok = !IsCommandByte(q_[i]);
                        }
                        
                        if (ok)
                        {
                            retVal = 1;
                            
                            cmd->param1 = q_[1];
                            
                            if (parameterCount == 2)
                            {
                                cmd->param2 = q_[2];
                            }
                            
                            // Good data, but we ate it all up
                            AdvanceHeadToNextCommandByte();
                        }
                        else
                        {
                            // Bad data
                            AdvanceHeadToNextCommandByte();
                        }
                    }
                    else
                    {
                        // Not all params in the buffer yet.  Do nothing.
                        // If the buf is full, the old bytes will get pushed
                        // out eventually.
                    }
                }
                else
                {
                    retVal = 1;
                    
                    // Good data, but we ate it all up
                    AdvanceHeadToNextCommandByte();
                }
            }
            else
            {
                // Bad data
                AdvanceHeadToNextCommandByte();
            }
        }
        
        return retVal;
    }

private:
    uint8_t IsCommandByte(uint8_t b)
    {
        return !!(b & 0b10000000);
    }
    
    void AdvanceHeadToNextCommandByte()
    {
        uint8_t tmp;
        
        if (q_.Size())
        {
            // Assume the first byte needs to be eliminated, or this function
            // wouldn't have been called.
            q_.Pop(tmp);
        }
        
        // Remove any trailing non-command bytes
        while (q_.Size() && !IsCommandByte(q_[0]))
        {
            q_.Pop(tmp);
        }
    }

    Queue<uint8_t, BUF_BYTES> q_;
};


#endif  // __MIDI_COMMAND_PARSER_H__













