#ifndef __APP_SYNTHESIZER1_H__
#define __APP_SYNTHESIZER1_H__


#include "Evm.h"

#include "MIDICommandFromSerial.h"
#include "MIDISynthesizer.h"


struct AppSynthesizer1Config
{
    
};

class AppSynthesizer1
{
private:
    static const uint8_t C_IDLE  = 10;
    static const uint8_t C_TIMED = 10;
    static const uint8_t C_INTER = 10;
    
    static const uint16_t BAUD = 31250;
    
public:
    AppSynthesizer1(const AppSynthesizer1Config &cfg)
    : cfg_(cfg)
    {
        // Nothing to do
    }
    
    void Run()
    {
        // Operate the serial port
        Serial.begin(BAUD);
        
        // Read and monitor controls
        ReadAndMonitorControls();
        
        // Direct inbound MIDI to the synthesizer
        midiReader_.SetCallbackOnMIDICommand([this](MIDICommand cmd){
            midiSynth_.ProcessCommand(cmd);
        });
        
        // Poll for new MIDI
        midiReader_.Start();
        
        // Set up and start synthesizer
        midiSynth_.Init();
        midiSynth_.Start();
        
        // Handle events
        evm_.MainLoop();
    }

private:

    void ReadAndMonitorControls()
    {
        // midiSynth_.SetCfgItem({SET_ENVELOPE_ATTACK_DURATION_MS,  (uint16_t)150});
        // midiSynth_.SetCfgItem({SET_ENVELOPE_DECAY_DURATION_MS,   (uint16_t)150});
        // midiSynth_.SetCfgItem({SET_ENVELOPE_SUSTAIN_LEVEL_PCT,   (uint8_t)80});
        // midiSynth_.SetCfgItem({SET_ENVELOPE_RELEASE_DURATION_MS, (uint16_t)500});
        
        midiSynth_.SetCfgItem({SET_OSCILLATOR_1_WAVE_TYPE, (uint8_t)OscillatorType::SINE});
        // midiSynth_.SetCfgItem({SET_OSCILLATOR_2_WAVE_TYPE, (uint8_t)OscillatorType::SINE});
    }



    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

    const AppSynthesizer1Config &cfg_;
    
    MIDICommandFromSerial  midiReader_;
    MIDISynthesizer        midiSynth_;
};


#endif  // __APP_SYNTHESIZER1_H__













