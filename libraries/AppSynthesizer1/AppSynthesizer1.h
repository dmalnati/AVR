#ifndef __APP_SYNTHESIZER1_H__
#define __APP_SYNTHESIZER1_H__


#include "Evm.h"

#include "PianoKeyboardPimoroniHAT.h"
#include "MIDICommandFromPianoKeyboardPimoroniHAT.h"

#include "MIDICommandFromSerial.h"
#include "MIDISynthesizer.h"


struct AppSynthesizer1Config
{
    // Synth
    uint8_t pinLogicalPisoPhaseLock;
    
    
    // Oscillator 1
    uint8_t pinLogicalPisoOsc1ButtonWaveTypeSine;
    uint8_t pinLogicalPisoOsc1ButtonWaveTypeSawr;
    uint8_t pinLogicalPisoOsc1ButtonWaveTypeSawl;
    uint8_t pinLogicalPisoOsc1ButtonWaveTypeSquare;
    uint8_t pinLogicalPisoOsc1ButtonWaveTypeTriangle;
    uint8_t pinLogicalPisoOsc1ButtonWaveTypeNone;
    
    uint8_t pinLogicalSipoOsc1LedWaveTypeSine;
    uint8_t pinLogicalSipoOsc1LedWaveTypeSawr;
    uint8_t pinLogicalSipoOsc1LedWaveTypeSawl;
    uint8_t pinLogicalSipoOsc1LedWaveTypeSquare;
    uint8_t pinLogicalSipoOsc1LedWaveTypeTriangle;
    uint8_t pinLogicalSipoOsc1LedWaveTypeNone;
    
    uint8_t pinLogicalMuxOsc1Frequency;
    
    
    // Oscillator 2
    uint8_t pinLogicalPisoOsc2ButtonWaveTypeSine;
    uint8_t pinLogicalPisoOsc2ButtonWaveTypeSawr;
    uint8_t pinLogicalPisoOsc2ButtonWaveTypeSawl;
    uint8_t pinLogicalPisoOsc2ButtonWaveTypeSquare;
    uint8_t pinLogicalPisoOsc2ButtonWaveTypeTriangle;
    uint8_t pinLogicalPisoOsc2ButtonWaveTypeNone;
    
    uint8_t pinLogicalSipoOsc2LedWaveTypeSine;
    uint8_t pinLogicalSipoOsc2LedWaveTypeSawr;
    uint8_t pinLogicalSipoOsc2LedWaveTypeSawl;
    uint8_t pinLogicalSipoOsc2LedWaveTypeSquare;
    uint8_t pinLogicalSipoOsc2LedWaveTypeTriangle;
    uint8_t pinLogicalSipoOsc2LedWaveTypeNone;
    
    uint8_t pinLogicalMuxOsc2Frequency;
    
    
    // Oscillator Balance
    uint8_t pinLogicalMuxOscBalance;

    
    // LFO
    uint8_t pinLogicalPisoLfoButtonWaveTypeSine;
    uint8_t pinLogicalPisoLfoButtonWaveTypeSawr;
    uint8_t pinLogicalPisoLfoButtonWaveTypeSawl;
    uint8_t pinLogicalPisoLfoButtonWaveTypeSquare;
    uint8_t pinLogicalPisoLfoButtonWaveTypeTriangle;
    uint8_t pinLogicalPisoLfoButtonWaveTypeNone;
    
    uint8_t pinLogicalSipoLfoLedWaveTypeSine;
    uint8_t pinLogicalSipoLfoLedWaveTypeSawr;
    uint8_t pinLogicalSipoLfoLedWaveTypeSawl;
    uint8_t pinLogicalSipoLfoLedWaveTypeSquare;
    uint8_t pinLogicalSipoLfoLedWaveTypeTriangle;
    uint8_t pinLogicalSipoLfoLedWaveTypeNone;
    
    uint8_t pinLogicalMuxLfoFrequency;
    
    uint8_t pinLogicalMuxTromello;
    uint8_t pinLogicalMuxVibrato;
    
    
    // Envelope
    uint8_t pinLogicalPisoEnvEnable;
    uint8_t pinLogicalPisoEnvPulse;
    uint8_t pinLogicalMuxEnvAttack;
    uint8_t pinLogicalMuxEnvDecay;
    uint8_t pinLogicalMuxEnvSustain;
    uint8_t pinLogicalMuxEnvRelease;
    
    
    // Physical Pin Layout
    uint8_t pinPisoLoad;
    uint8_t pinPisoClock;
    uint8_t pinPisoClockEnable;
    uint8_t pinPisoSerial;
    
    uint8_t pinSipoClock;
    uint8_t pinSipoLatch;
    uint8_t pinSerial;
    
    uint8_t pinMuxBit0;
    uint8_t pinMuxBit1;
    uint8_t pinMuxBit2;
    uint8_t pinMuxBit3;
    uint8_t pinMuxAnalog;
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
        SetUpInputsAndDefaults();
        
        // Set up and start Synthesizer
        midiSynth_.Init();
        midiSynth_.Start();
        
        // Handle events
        evm_.MainLoop();
    }

private:

    void SetUpInputsAndDefaults()
    {
        // Setup synthesizer defaults
        midiSynth_.SetCfgItem({SET_OSCILLATOR_1_WAVE_TYPE, (uint8_t)OscillatorType::SINE});
        midiSynth_.SetCfgItem({SET_OSCILLATOR_2_WAVE_TYPE, (uint8_t)OscillatorType::SINE});
        
        // Direct inbound MIDI to the synthesizer
        midiReader_.SetCallbackOnMIDICommand([this](MIDICommand cmd){
            midiSynth_.ProcessCommand(cmd);
        });
        midiReader_.Start();
        
        // Set up keyboard
        hatToMidi_.GetHat().EnableLEDs();
        hatToMidi_.Init();
    }



    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

    const AppSynthesizer1Config &cfg_;
    
    MIDICommandFromSerial  midiReader_;
    MIDISynthesizer        midiSynth_;
    
    MIDICommandFromPianoKeyboardPimoroniHAT  hatToMidi_;

};


#endif  // __APP_SYNTHESIZER1_H__













