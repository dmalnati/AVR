#ifndef __SYNTHESIZER_VOICE_SERIAL_INTERFACE_H__
#define __SYNTHESIZER_VOICE_SERIAL_INTERFACE_H__


#include <string.h>

#include "Function.h"
#include "SerialLink.h"


template <typename SynthesizerVoiceClass>
class SynthesizerVoiceSerialInterface
{
    static const uint8_t PROTOCOL_ID = 123;
    
public:
    static const uint8_t C_IDLE  = SerialLink::C_IDLE;
    static const uint8_t C_TIMED = SerialLink::C_TIMED;
    static const uint8_t C_INTER = SerialLink::C_INTER;

public:

    SynthesizerVoiceSerialInterface(SynthesizerVoiceClass *sv)
    : sv_(sv)
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
    
    
    enum class MessageType : uint8_t
    {
        SET_PHASE_LOCK = 5,
        SET_OSCILLATOR_1_FREQUENCY = 11,
        SET_OSCILLATOR_1_WAVE_TYPE = 12,
        SET_OSCILLATOR_1_PHASE_OFFSET = 13,
        SET_OSCILLATOR_BALANCE = 20,
        SET_OSCILLATOR_2_FREQUENCY = 21,
        SET_OSCILLATOR_2_WAVE_TYPE = 22,
        SET_OSCILLATOR_2_PHASE_OFFSET = 23,
        SET_LFO_FREQUENCY = 31,
        SET_LFO_WAVE_TYPE = 32,
        SET_LFO_PHASE_OFFSET = 33,
        SET_LFO_VIBRATO_PCT = 34,
        SET_LFO_TROMOLO_PCT = 35,
        ENVELOPE_ENABLE = 41,
        ENVELOPE_DISABLE = 42
    };

    
    
    void Process(uint8_t *bufWithType, uint8_t bufWithTypeSize)
    {
        // Extract message body
        uint8_t *buf     = &bufWithType[1];
        uint8_t  bufSize = bufWithTypeSize - 1;
        
        // Extract MessageType
        MessageType t = (MessageType)bufWithType[0];
        
        switch (t)
        {
        case MessageType::SET_PHASE_LOCK: SET_PHASE_LOCK(buf, bufSize); break;

        case MessageType::SET_OSCILLATOR_1_FREQUENCY: SET_OSCILLATOR_1_FREQUENCY(buf, bufSize); break;
        case MessageType::SET_OSCILLATOR_1_WAVE_TYPE: SET_OSCILLATOR_1_WAVE_TYPE(buf, bufSize); break;
        case MessageType::SET_OSCILLATOR_1_PHASE_OFFSET: SET_OSCILLATOR_1_PHASE_OFFSET(buf, bufSize); break;
        
        case MessageType::SET_OSCILLATOR_BALANCE: SET_OSCILLATOR_BALANCE(buf, bufSize); break;
        
        case MessageType::SET_OSCILLATOR_2_FREQUENCY: SET_OSCILLATOR_2_FREQUENCY(buf, bufSize); break;
        case MessageType::SET_OSCILLATOR_2_WAVE_TYPE: SET_OSCILLATOR_2_WAVE_TYPE(buf, bufSize); break;
        case MessageType::SET_OSCILLATOR_2_PHASE_OFFSET: SET_OSCILLATOR_2_PHASE_OFFSET(buf, bufSize); break;
        
        case MessageType::SET_LFO_FREQUENCY: SET_LFO_FREQUENCY(buf, bufSize); break;
        case MessageType::SET_LFO_WAVE_TYPE: SET_LFO_WAVE_TYPE(buf, bufSize); break;
        case MessageType::SET_LFO_PHASE_OFFSET: SET_LFO_PHASE_OFFSET(buf, bufSize); break;
        case MessageType::SET_LFO_VIBRATO_PCT: SET_LFO_VIBRATO_PCT(buf, bufSize); break;
        case MessageType::SET_LFO_TROMOLO_PCT: SET_LFO_TROMOLO_PCT(buf, bufSize); break;
        
        
        case MessageType::ENVELOPE_ENABLE: ENVELOPE_ENABLE(buf, bufSize); break;
        case MessageType::ENVELOPE_DISABLE: ENVELOPE_DISABLE(buf, bufSize); break;
        
        default:
            break;
        }
    }
    
    
    
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
    
    
    
    
    
    
    void SET_PHASE_LOCK(uint8_t *buf, uint8_t bufSize)
    {
        const uint8_t BYTES_NEEDED = sizeof(uint8_t);
        
        if (bufSize == BYTES_NEEDED)
        {
            uint8_t idx = 0;
            
            uint8_t phaseLock = GetU8(buf, idx);
            
            sv_->SetPhaseLock(phaseLock);
        }
    }
    
    
    
    
    
    
    
    
    void SET_OSCILLATOR_1_FREQUENCY(uint8_t *buf, uint8_t bufSize)
    {
        const uint8_t BYTES_NEEDED = sizeof(uint16_t);
        
        if (bufSize == BYTES_NEEDED)
        {
            uint8_t idx = 0;
            
            uint16_t frequency = GetU16(buf, idx);
            
            sv_->SetOscillator1Frequency(frequency);
        }
    }
    
    void SET_OSCILLATOR_1_WAVE_TYPE(uint8_t *buf, uint8_t bufSize)
    {
        const uint8_t BYTES_NEEDED = sizeof(uint8_t);
        
        if (bufSize == BYTES_NEEDED)
        {
            uint8_t idx = 0;
            
            typename SynthesizerVoiceClass::OscillatorType waveType =
                (typename SynthesizerVoiceClass::OscillatorType)GetU8(buf, idx);
            
            sv_->SetOscillator1WaveType(waveType);
        }
    }
    
    void SET_OSCILLATOR_1_PHASE_OFFSET(uint8_t *buf, uint8_t bufSize)
    {
        const uint8_t BYTES_NEEDED = sizeof(int8_t);
        
        if (bufSize == BYTES_NEEDED)
        {
            uint8_t idx = 0;
            
            int8_t phaseOffset = GetI8(buf, idx);
            
            sv_->SetOscillator1PhaseOffset(phaseOffset);
        }
    }
    
    
    
    
    void SET_OSCILLATOR_BALANCE(uint8_t *buf, uint8_t bufSize)
    {
        const uint8_t BYTES_NEEDED = sizeof(uint8_t);
        
        if (bufSize == BYTES_NEEDED)
        {
            uint8_t idx = 0;
            
            uint8_t balance = GetU8(buf, idx);
            
            sv_->SetOscillatorBalance(balance);
        }
    }

    
    
    
    
    
    
    
    
    
    

    void SET_OSCILLATOR_2_FREQUENCY(uint8_t *buf, uint8_t bufSize)
    {
        const uint8_t BYTES_NEEDED = sizeof(uint16_t);
        
        if (bufSize == BYTES_NEEDED)
        {
            uint8_t idx = 0;
            
            uint16_t frequency = GetU16(buf, idx);
            
            sv_->SetOscillator2Frequency(frequency);
        }
    }
    
    void SET_OSCILLATOR_2_WAVE_TYPE(uint8_t *buf, uint8_t bufSize)
    {
        const uint8_t BYTES_NEEDED = sizeof(uint8_t);
        
        if (bufSize == BYTES_NEEDED)
        {
            uint8_t idx = 0;
            
            typename SynthesizerVoiceClass::OscillatorType waveType =
                (typename SynthesizerVoiceClass::OscillatorType)GetU8(buf, idx);
            
            sv_->SetOscillator2WaveType(waveType);
        }
    }
    
    void SET_OSCILLATOR_2_PHASE_OFFSET(uint8_t *buf, uint8_t bufSize)
    {
        const uint8_t BYTES_NEEDED = sizeof(int8_t);
        
        if (bufSize == BYTES_NEEDED)
        {
            uint8_t idx = 0;
            
            int8_t phaseOffset = GetI8(buf, idx);
            
            sv_->SetOscillator2PhaseOffset(phaseOffset);
        }
    }
    
    
    
    
    
    
    





    
    void SET_LFO_FREQUENCY(uint8_t *buf, uint8_t bufSize)
    {
        const uint8_t BYTES_NEEDED = sizeof(uint16_t);
        
        if (bufSize == BYTES_NEEDED)
        {
            uint8_t idx = 0;
            
            uint16_t frequency = GetU16(buf, idx);
            
            sv_->SetLFOFrequency(frequency);
        }
    }
    
    
    void SET_LFO_WAVE_TYPE(uint8_t *buf, uint8_t bufSize)
    {
        const uint8_t BYTES_NEEDED = sizeof(uint8_t);
        
        if (bufSize == BYTES_NEEDED)
        {
            uint8_t idx = 0;
            
            typename SynthesizerVoiceClass::OscillatorType waveType =
                (typename SynthesizerVoiceClass::OscillatorType)GetU8(buf, idx);
            
            sv_->SetLFOWaveType(waveType);
        }
    }
    
    void SET_LFO_PHASE_OFFSET(uint8_t *buf, uint8_t bufSize)
    {
        const uint8_t BYTES_NEEDED = sizeof(int8_t);
        
        if (bufSize == BYTES_NEEDED)
        {
            uint8_t idx = 0;
            
            int8_t phaseOffset = GetI8(buf, idx);
            
            sv_->SetLFOPhaseOffset(phaseOffset);
        }
    }
    
    void SET_LFO_VIBRATO_PCT(uint8_t *buf, uint8_t bufSize)
    {
        const uint8_t BYTES_NEEDED = sizeof(uint8_t);
        
        if (bufSize == BYTES_NEEDED)
        {
            uint8_t idx = 0;
            
            uint8_t vibratoPct = GetU8(buf, idx);
            
            sv_->SetLFOVibratoPct(vibratoPct);
        }
    }
    
    void SET_LFO_TROMOLO_PCT(uint8_t *buf, uint8_t bufSize)
    {
        const uint8_t BYTES_NEEDED = sizeof(uint8_t);
        
        if (bufSize == BYTES_NEEDED)
        {
            uint8_t idx = 0;
            
            uint8_t tromoloPct = GetU8(buf, idx);
            
            sv_->SetLFOTromoloPct(tromoloPct);
        }
    }
    
    
    
    
    
    
    
    
    void ENVELOPE_ENABLE(uint8_t *, uint8_t)
    {
        sv_->EnableEnvelopeADSR();
    }
    
    void ENVELOPE_DISABLE(uint8_t *, uint8_t)
    {
        sv_->DisableEnvelopeADSR();
    }
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    


    SerialLink serialLink_;
    
    SynthesizerVoiceClass *sv_;
};


#endif  // __SYNTHESIZER_VOICE_SERIAL_INTERFACE_H__




