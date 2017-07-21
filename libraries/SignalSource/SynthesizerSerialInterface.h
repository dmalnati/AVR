#ifndef __SYNTHESIZER_SERIAL_INTERFACE_H__
#define __SYNTHESIZER_SERIAL_INTERFACE_H__


#include <string.h>

#include "Function.h"
#include "SerialLink.h"

#include "FunctionGenerator.h"


class SynthesizerSerialInterface
{
    static const uint8_t PROTOCOL_ID = 123;
    
public:
    static const uint8_t C_IDLE  = SerialLink::C_IDLE;
    static const uint8_t C_TIMED = SerialLink::C_TIMED;
    static const uint8_t C_INTER = SerialLink::C_INTER;

public:

    SynthesizerSerialInterface(Synthesizer *s)
    : s_(s)
    {
        // Nothing to do
    }

    void Init()
    {
        serialLink_.Init([this](SerialLinkHeader *hdr,
                                uint8_t          *buf,
                                uint8_t           bufSize){
            OnSerialRxAvailable(hdr, buf, bufSize);
        });
    }


private:

    void OnSerialRxAvailable(SerialLinkHeader *hdr,
                             uint8_t          *buf,
                             uint8_t           bufSize)
    {
        // Confirm that at a minimum there is a MessageType
        if (hdr->protocolId == PROTOCOL_ID && bufSize)
        {
            Process(buf, bufSize);
        }
    }
    
    
    

    
    void Process(uint8_t *bufWithType, uint8_t bufWithTypeSize)
    {
        ///////////////////////////////////////////////////////////////////////
        //
        // Messages which match a configuration item
        //
        ///////////////////////////////////////////////////////////////////////
        
        // Extract message body
        uint8_t *buf     = &bufWithType[1];
        uint8_t  bufSize = bufWithTypeSize - 1;
        
        uint8_t messageTypeByte = bufWithType[0];
        
        switch (messageTypeByte)
        {
        case SET_PHASE_LOCK:
            ON_SET_PHASE_LOCK(buf, bufSize);
            break;

        case SET_OSCILLATOR_1_FREQUENCY:
            ON_SET_OSCILLATOR_1_FREQUENCY(buf, bufSize);
            break;
            
        case SET_OSCILLATOR_1_WAVE_TYPE:
            ON_SET_OSCILLATOR_1_WAVE_TYPE(buf, bufSize);
            break;
            
        case SET_OSCILLATOR_1_PHASE_OFFSET:
            ON_SET_OSCILLATOR_1_PHASE_OFFSET(buf, bufSize);
            break;
        
        case SET_OSCILLATOR_BALANCE:
            ON_SET_OSCILLATOR_BALANCE(buf, bufSize);
            break;
        
        case SET_OSCILLATOR_2_FREQUENCY:
            ON_SET_OSCILLATOR_2_FREQUENCY(buf, bufSize);
            break;
            
        case SET_OSCILLATOR_2_WAVE_TYPE:
            ON_SET_OSCILLATOR_2_WAVE_TYPE(buf, bufSize);
            break;
            
        case SET_OSCILLATOR_2_PHASE_OFFSET:
            ON_SET_OSCILLATOR_2_PHASE_OFFSET(buf, bufSize);
            break;
        
        case SET_LFO_FREQUENCY:
            ON_SET_LFO_FREQUENCY(buf, bufSize);
            break;
            
        case SET_LFO_WAVE_TYPE:
            ON_SET_LFO_WAVE_TYPE(buf, bufSize);
            break;
            
        case SET_LFO_PHASE_OFFSET:
            ON_SET_LFO_PHASE_OFFSET(buf, bufSize);
            break;
            
        case SET_LFO_VIBRATO_PCT:
            ON_SET_LFO_VIBRATO_PCT(buf, bufSize);
            break;
            
        case SET_LFO_TROMOLO_PCT:
            ON_SET_LFO_TROMOLO_PCT(buf, bufSize);
            break;
        
        
        
        
        
        case SET_ENVELOPE_ON_OFF:
            ON_SET_ENVELOPE_ON_OFF(buf, bufSize);
            break;
            
        case SET_ENVELOPE_ATTACK_DURATION_MS:
            ON_SET_ENVELOPE_ATTACK_DURATION_MS(buf, bufSize);
            break;
            
        case SET_ENVELOPE_DECAY_DURATION_MS:
            ON_SET_ENVELOPE_DECAY_DURATION_MS(buf, bufSize);
            break;
            
        case SET_ENVELOPE_SUSTAIN_LEVEL_PCT:
            ON_SET_ENVELOPE_SUSTAIN_LEVEL_PCT(buf, bufSize);
            break;
            
        case SET_ENVELOPE_RELEASE_DURATION_MS:
            ON_SET_ENVELOPE_RELEASE_DURATION_MS(buf, bufSize);
            break;
        
        

        
        default:
            break;
        }
        
        
        
        ///////////////////////////////////////////////////////////////////////
        //
        // Messages which do not match a configuration item
        //
        ///////////////////////////////////////////////////////////////////////
        
        enum class MessageType : uint8_t
        {
            FN_SYNTHESIZER_KEY_DOWN = 61,
            FN_SYNTHESIZER_KEY_DOWN_NOTE = 62,
            FN_SYNTHESIZER_KEY_UP = 63,
            FN_SYNTHESIZER_CYCLE_TO_NEXT_INSTRUMENT = 64
        };
        
        MessageType t = (MessageType)bufWithType[0];
        
        switch (t)
        {
        case MessageType::FN_SYNTHESIZER_KEY_DOWN:
            FN_SYNTHESIZER_KEY_DOWN(buf, bufSize);
            break;
        
        case MessageType::FN_SYNTHESIZER_KEY_DOWN_NOTE:
            FN_SYNTHESIZER_KEY_DOWN_NOTE(buf, bufSize);
            break;
            
        case MessageType::FN_SYNTHESIZER_KEY_UP:
            FN_SYNTHESIZER_KEY_UP(buf, bufSize);
            break;

        case MessageType::FN_SYNTHESIZER_CYCLE_TO_NEXT_INSTRUMENT:
            FN_SYNTHESIZER_CYCLE_TO_NEXT_INSTRUMENT(buf, bufSize);
            break;
        
        default:
            break;
        }
    }
    
    
    ///////////////////////////////////////////////////////////////////////
    //
    // Message Parsing Utility
    //
    ///////////////////////////////////////////////////////////////////////

    
    uint8_t GetI8(uint8_t *buf, uint8_t &idx)
    {
        int8_t retVal = buf[idx];
        
        ++idx;
        
        return retVal;
    }
    
    uint8_t GetU8(uint8_t *buf, uint8_t &idx)
    {
        uint8_t retVal = buf[idx];
        
        ++idx;
        
        return retVal;
    }
    
    uint16_t GetU16(uint8_t *buf, uint8_t &idx)
    {
        uint16_t retVal;
        
        memcpy((void *)&retVal, (void *)&buf[idx], sizeof(uint16_t));
        
        idx += sizeof(uint16_t);
        
        return PAL.ntohs(retVal);
    }
    
    
    ///////////////////////////////////////////////////////////////////////
    //
    // Message Parsing
    //
    ///////////////////////////////////////////////////////////////////////
    
    void ON_SET_PHASE_LOCK(uint8_t *buf, uint8_t bufSize)
    {
        const uint8_t BYTES_NEEDED = sizeof(uint8_t);
        
        if (bufSize == BYTES_NEEDED)
        {
            uint8_t idx = 0;
            
            uint8_t phaseLock = GetU8(buf, idx);
            
            s_->SetCfgItem({SET_PHASE_LOCK, phaseLock});
        }
    }
    
    
    void ON_SET_OSCILLATOR_1_FREQUENCY(uint8_t *buf, uint8_t bufSize)
    {
        const uint8_t BYTES_NEEDED = sizeof(uint16_t);
        
        if (bufSize == BYTES_NEEDED)
        {
            uint8_t idx = 0;
            
            uint16_t frequency = GetU16(buf, idx);
            
            s_->SetCfgItem({SET_OSCILLATOR_1_FREQUENCY, frequency});
        }
    }
    
    void ON_SET_OSCILLATOR_1_WAVE_TYPE(uint8_t *buf, uint8_t bufSize)
    {
        const uint8_t BYTES_NEEDED = sizeof(uint8_t);
        
        if (bufSize == BYTES_NEEDED)
        {
            uint8_t idx = 0;
            
            uint8_t waveType = GetU8(buf, idx);
            
            s_->SetCfgItem({SET_OSCILLATOR_1_WAVE_TYPE, waveType});
        }
    }
    
    void ON_SET_OSCILLATOR_1_PHASE_OFFSET(uint8_t *buf, uint8_t bufSize)
    {
        const uint8_t BYTES_NEEDED = sizeof(int8_t);
        
        if (bufSize == BYTES_NEEDED)
        {
            uint8_t idx = 0;
            
            int8_t phaseOffset = GetI8(buf, idx);
            
            s_->SetCfgItem({SET_OSCILLATOR_1_PHASE_OFFSET, phaseOffset});
        }
    }
    
    
    void ON_SET_OSCILLATOR_BALANCE(uint8_t *buf, uint8_t bufSize)
    {
        const uint8_t BYTES_NEEDED = sizeof(uint8_t);
        
        if (bufSize == BYTES_NEEDED)
        {
            uint8_t idx = 0;
            
            uint8_t balance = GetU8(buf, idx);
            
            s_->SetCfgItem({SET_OSCILLATOR_BALANCE, balance});
        }
    }


    void ON_SET_OSCILLATOR_2_FREQUENCY(uint8_t *buf, uint8_t bufSize)
    {
        const uint8_t BYTES_NEEDED = sizeof(uint16_t);
        
        if (bufSize == BYTES_NEEDED)
        {
            uint8_t idx = 0;
            
            uint16_t frequency = GetU16(buf, idx);
            
            s_->SetCfgItem({SET_OSCILLATOR_2_FREQUENCY, frequency});
        }
    }
    
    void ON_SET_OSCILLATOR_2_WAVE_TYPE(uint8_t *buf, uint8_t bufSize)
    {
        const uint8_t BYTES_NEEDED = sizeof(uint8_t);
        
        if (bufSize == BYTES_NEEDED)
        {
            uint8_t idx = 0;
            
            uint8_t waveType = GetU8(buf, idx);
            
            s_->SetCfgItem({SET_OSCILLATOR_2_WAVE_TYPE, waveType});
        }
    }
    
    void ON_SET_OSCILLATOR_2_PHASE_OFFSET(uint8_t *buf, uint8_t bufSize)
    {
        const uint8_t BYTES_NEEDED = sizeof(int8_t);
        
        if (bufSize == BYTES_NEEDED)
        {
            uint8_t idx = 0;
            
            int8_t phaseOffset = GetI8(buf, idx);
            
            s_->SetCfgItem({SET_OSCILLATOR_2_PHASE_OFFSET, phaseOffset});
        }
    }
    
    
    void ON_SET_LFO_FREQUENCY(uint8_t *buf, uint8_t bufSize)
    {
        const uint8_t BYTES_NEEDED = sizeof(uint16_t);
        
        if (bufSize == BYTES_NEEDED)
        {
            uint8_t idx = 0;
            
            uint16_t frequency = GetU16(buf, idx);
            
            s_->SetCfgItem({SET_LFO_FREQUENCY, frequency});
        }
    }
    
    void ON_SET_LFO_WAVE_TYPE(uint8_t *buf, uint8_t bufSize)
    {
        const uint8_t BYTES_NEEDED = sizeof(uint8_t);
        
        if (bufSize == BYTES_NEEDED)
        {
            uint8_t idx = 0;
            
            uint8_t waveType = GetU8(buf, idx);
            
            s_->SetCfgItem({SET_LFO_WAVE_TYPE, waveType});
        }
    }
    
    void ON_SET_LFO_PHASE_OFFSET(uint8_t *buf, uint8_t bufSize)
    {
        const uint8_t BYTES_NEEDED = sizeof(int8_t);
        
        if (bufSize == BYTES_NEEDED)
        {
            uint8_t idx = 0;
            
            int8_t phaseOffset = GetI8(buf, idx);
            
            s_->SetCfgItem({SET_LFO_PHASE_OFFSET, phaseOffset});
        }
    }
    
    void ON_SET_LFO_VIBRATO_PCT(uint8_t *buf, uint8_t bufSize)
    {
        const uint8_t BYTES_NEEDED = sizeof(uint8_t);
        
        if (bufSize == BYTES_NEEDED)
        {
            uint8_t idx = 0;
            
            uint8_t vibratoPct = GetU8(buf, idx);
            
            s_->SetCfgItem({SET_LFO_VIBRATO_PCT, vibratoPct});
        }
    }
    
    void ON_SET_LFO_TROMOLO_PCT(uint8_t *buf, uint8_t bufSize)
    {
        const uint8_t BYTES_NEEDED = sizeof(uint8_t);
        
        if (bufSize == BYTES_NEEDED)
        {
            uint8_t idx = 0;
            
            uint8_t tromoloPct = GetU8(buf, idx);
            
            s_->SetCfgItem({SET_LFO_TROMOLO_PCT, tromoloPct});
        }
    }
    
    
    
    
    
    
    
    
    void ON_SET_ENVELOPE_ON_OFF(uint8_t *buf, uint8_t bufSize)
    {
        const uint8_t BYTES_NEEDED = sizeof(uint8_t);
        
        if (bufSize == BYTES_NEEDED)
        {
            uint8_t idx = 0;
            
            uint8_t onOff = GetU8(buf, idx);
            
            s_->SetCfgItem({SET_ENVELOPE_ON_OFF, onOff});
        }
    }
    
    void ON_SET_ENVELOPE_ATTACK_DURATION_MS(uint8_t *buf, uint8_t bufSize)
    {
        const uint8_t BYTES_NEEDED = sizeof(uint16_t);
        
        if (bufSize == BYTES_NEEDED)
        {
            uint8_t idx = 0;
            
            uint16_t durationMs = GetU16(buf, idx);
            
            s_->SetCfgItem({SET_ENVELOPE_ATTACK_DURATION_MS, durationMs});
        }
    }
    
    void ON_SET_ENVELOPE_DECAY_DURATION_MS(uint8_t *buf, uint8_t bufSize)
    {
        const uint8_t BYTES_NEEDED = sizeof(uint16_t);
        
        if (bufSize == BYTES_NEEDED)
        {
            uint8_t idx = 0;
            
            uint16_t durationMs = GetU16(buf, idx);
            
            s_->SetCfgItem({SET_ENVELOPE_DECAY_DURATION_MS, durationMs});
        }
    }
    
    void ON_SET_ENVELOPE_SUSTAIN_LEVEL_PCT(uint8_t *buf, uint8_t bufSize)
    {
        const uint8_t BYTES_NEEDED = sizeof(uint8_t);
        
        if (bufSize == BYTES_NEEDED)
        {
            uint8_t idx = 0;
            
            uint8_t levelPct = GetU8(buf, idx);
            
            s_->SetCfgItem({SET_ENVELOPE_SUSTAIN_LEVEL_PCT, levelPct});
        }
    }
    
    void ON_SET_ENVELOPE_RELEASE_DURATION_MS(uint8_t *buf, uint8_t bufSize)
    {
        const uint8_t BYTES_NEEDED = sizeof(uint16_t);
        
        if (bufSize == BYTES_NEEDED)
        {
            uint8_t idx = 0;
            
            uint16_t durationMs = GetU16(buf, idx);
            
            s_->SetCfgItem({SET_ENVELOPE_RELEASE_DURATION_MS, durationMs});
        }
    }
    
    
    
    
    
    
    
    void FN_SYNTHESIZER_KEY_DOWN(uint8_t *, uint8_t)
    {
        s_->OnKeyDown();
    }
    
    void FN_SYNTHESIZER_KEY_DOWN_NOTE(uint8_t *buf, uint8_t bufSize)
    {
        const uint8_t BYTES_NEEDED = sizeof(uint8_t);
        
        if (bufSize == BYTES_NEEDED)
        {
            uint8_t idx = 0;
            
            uint8_t key = GetU8(buf, idx);
            
            Synthesizer::Note n;
            
            switch (key)
            {
            case  1: n = Synthesizer::Note::C       ; break;
            case  2: n = Synthesizer::Note::C_SHARP ; break;
            case  3: n = Synthesizer::Note::D       ; break;
            case  4: n = Synthesizer::Note::D_SHARP ; break;
            case  5: n = Synthesizer::Note::E       ; break;
            case  6: n = Synthesizer::Note::F       ; break;
            case  7: n = Synthesizer::Note::G_FLAT  ; break;
            case  8: n = Synthesizer::Note::G       ; break;
            case  9: n = Synthesizer::Note::A_FLAT  ; break;
            case 10: n = Synthesizer::Note::A       ; break;
            case 11: n = Synthesizer::Note::B_FLAT  ; break;
            case 12: n = Synthesizer::Note::B       ; break;
            
            default: n = Synthesizer::Note::A       ; break;
            }
            
            s_->OnKeyDown(n);
        }
    }
    
    void FN_SYNTHESIZER_KEY_UP(uint8_t *, uint8_t)
    {
        s_->OnKeyUp();
    }
    
    void FN_SYNTHESIZER_CYCLE_TO_NEXT_INSTRUMENT(uint8_t *, uint8_t)
    {
        s_->CycleToNextInstrument();
    }
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    


    SerialLink serialLink_;
    
    Synthesizer *s_;
};


#endif  // __SYNTHESIZER_SERIAL_INTERFACE_H__




