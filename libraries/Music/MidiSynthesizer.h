#ifndef __MIDI_SYNTHESIZER_H__
#define __MIDI_SYNTHESIZER_H__


#include "Timer2.h"
#include "SignalSourceSineWave.h"
#include "SignalDAC.h"


struct MidiCommand
{
    enum class Type : uint8_t
    {
        INVALID               = 0b11111111,
        NOTE_OFF              = 0b00000000,
        NOTE_ON               = 0b00000001,
        AFTERTOUCH            = 0b00000010,
        CONTINUOUS_CONTROLLER = 0b00000011,
        PATCH_CHANGE          = 0b00000100,
        CHANNEL_PRESSURE      = 0b00000101,
        PITCH_BEND            = 0b00000110,
        NON_MUSICAL_COMMANDS  = 0b00000111
    };
    
    Type type       = Type::INVALID;
    uint8_t channel = 0;
    uint8_t param1  = 0;
    uint8_t param2  = 0;
};



#include "Container.h"

class MidiCommandParser
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
    
    uint8_t GetMsg(MidiCommand *cmd)
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
                cmd->type = (MidiCommand::Type)((firstByte & 0b01110000) >> 4);
                    
                // Extract the 4 bits which determine the channel
                cmd->channel = (firstByte & 0b00001111);
                
                // Determine the number of parameters
                uint8_t parameterCount = 0;
                
                switch (cmd->type)
                {
                case MidiCommand::Type::NOTE_OFF:
                case MidiCommand::Type::NOTE_ON:
                case MidiCommand::Type::AFTERTOUCH:
                case MidiCommand::Type::CONTINUOUS_CONTROLLER:
                case MidiCommand::Type::PITCH_BEND:
                    parameterCount = 2;
                    break;
                    
                case MidiCommand::Type::PATCH_CHANGE:
                case MidiCommand::Type::CHANNEL_PRESSURE:
                    parameterCount = 1;
                    break;
                
                case MidiCommand::Type::NON_MUSICAL_COMMANDS:
                default:
                    // Not actually possible to get an enum value that isn't
                    // in the list above.  3 bits are fully accounted for.
                    // As a result I won't account for this below.
                    break;
                }
                
                // Check if parameters are ready to be consumed
                // Don't forget that we still have the first byte in the queue.
                if (parameterCount && q_.Size() >= parameterCount + 1)
                {
                    uint8_t ok = 1;
                    for (uint8_t i = 1; ok && i < parameterCount + 1; ++i)
                    {
                        // basically break on first failure
                        ok = IsCommandByte(q_[i]);
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


class MidiCommandFromSerial
{
    uint8_t MAX_BYTES_PER_ATTEMPT = 6;
    
public:
    
    void Init(MidiCommandParser *mcp)
    {
        mcp_ = mcp;
    }

    uint8_t GetMidiCommand(MidiCommand *cmd)
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
    MidiCommandParser *mcp_ = NULL;
};





class MidiSynthesizer
{
    using DAC = SignalDAC<SignalSourceSineWave, Timer2>;
    
    static const uint16_t SAMPLE_RATE = 24000;
    
public:

    void Init()
    {
        dac_.SetSampleRate(SAMPLE_RATE);
    }
    
    void Start()
    {
        dac_.Start();
    }
    
    void Stop()
    {
        dac_.Stop();
    }
    
    void ProcessCommand(const MidiCommand &/*cmd*/)
    {
        
    }

private:
    DAC dac_;
};






#endif  // __MIDI_SYNTHESIZER_H__





