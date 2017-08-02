#ifndef __APP_SYNTHESIZER1_H__
#define __APP_SYNTHESIZER1_H__


#include "Evm.h"

#include "ShiftRegisterIn.h"
#include "ShiftRegisterInput.h"

#include "ShiftRegisterOut.h"
#include "ShiftRegisterOutput.h"

#include "MuxAnalogDigitalCD74HC4067.h"

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
    uint8_t pinSipoSerial;
    
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
    
    static const uint8_t SHIFT_REGISTER_IN_COUNT  = 3;
    static const uint8_t SHIFT_REGISTER_OUT_COUNT = 3;
    
public:
    AppSynthesizer1(const AppSynthesizer1Config &cfg)
    : cfg_(cfg)
    , srIn_(cfg_.pinPisoLoad, cfg_.pinPisoClock, cfg_.pinPisoClockEnable, cfg_.pinPisoSerial)
    , srInput_(srIn_)
    , srOut_(cfg_.pinSipoClock, cfg_.pinSipoLatch, cfg_.pinSipoSerial)
    , srOutput_(srOut_)
    , mux_(cfg_.pinMuxBit0, cfg_.pinMuxBit1, cfg_.pinMuxBit2, cfg_.pinMuxBit3, cfg_.pinMuxAnalog)
    {
        // Nothing to do
    }
    
    void Run()
    {
        // Operate the serial port
        Serial.begin(BAUD);
        
        // Set up Synthesizer
        midiSynth_.Init();
        
        // Read and monitor controls
        SetUpInputsAndDefaults();
        
        // Start Synthesizer
        midiSynth_.Start();
        
        // Handle events
        evm_.MainLoop();
    }

private:

    void SetUpInputsAndDefaults()
    {
        // Setup Button Inputs
        SetupButtonInputs();
        
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
    
    void SetupButtonInputs()
    {
        SetupOsc1Inputs();
        SetupOsc2Inputs();
        SetupLfoInputs();
    }
    
    void SetupOsc1Inputs()
    {
        srInput_.SetCallback(cfg_.pinLogicalPisoOsc1ButtonWaveTypeSine, [this](uint8_t){
            ClearOsc1Leds();
            srOutput_.DigitalWrite(cfg_.pinLogicalSipoLfoLedWaveTypeSine, HIGH);
            midiSynth_.SetCfgItem({SET_OSCILLATOR_1_WAVE_TYPE, (uint8_t)OscillatorType::SINE});
        });
        srInput_.SetCallback(cfg_.pinLogicalPisoOsc1ButtonWaveTypeSawr, [this](uint8_t){
            ClearOsc1Leds();
            srOutput_.DigitalWrite(cfg_.pinLogicalSipoLfoLedWaveTypeSawr, HIGH);
            midiSynth_.SetCfgItem({SET_OSCILLATOR_1_WAVE_TYPE, (uint8_t)OscillatorType::SAWR});
        });
        srInput_.SetCallback(cfg_.pinLogicalPisoOsc1ButtonWaveTypeSawl, [this](uint8_t){
            ClearOsc1Leds();
            srOutput_.DigitalWrite(cfg_.pinLogicalSipoLfoLedWaveTypeSawl, HIGH);
            midiSynth_.SetCfgItem({SET_OSCILLATOR_1_WAVE_TYPE, (uint8_t)OscillatorType::SAWL});
        });
        srInput_.SetCallback(cfg_.pinLogicalPisoOsc1ButtonWaveTypeSquare, [this](uint8_t){
            ClearOsc1Leds();
            srOutput_.DigitalWrite(cfg_.pinLogicalSipoLfoLedWaveTypeSquare, HIGH);
            midiSynth_.SetCfgItem({SET_OSCILLATOR_1_WAVE_TYPE, (uint8_t)OscillatorType::SQUARE});
        });
        srInput_.SetCallback(cfg_.pinLogicalPisoOsc1ButtonWaveTypeTriangle, [this](uint8_t){
            ClearOsc1Leds();
            srOutput_.DigitalWrite(cfg_.pinLogicalSipoLfoLedWaveTypeTriangle, HIGH);
            midiSynth_.SetCfgItem({SET_OSCILLATOR_1_WAVE_TYPE, (uint8_t)OscillatorType::TRIANGLE});
        });
        srInput_.SetCallback(cfg_.pinLogicalPisoOsc1ButtonWaveTypeNone, [this](uint8_t){
            ClearOsc1Leds();
            srOutput_.DigitalWrite(cfg_.pinLogicalSipoLfoLedWaveTypeNone, HIGH);
            midiSynth_.SetCfgItem({SET_OSCILLATOR_1_WAVE_TYPE, (uint8_t)OscillatorType::NONE});
        });
    }
    
    void ClearOsc1Leds()
    {
        uint8_t pinList[] = {
            cfg_.pinLogicalSipoOsc1LedWaveTypeSine,
            cfg_.pinLogicalSipoOsc1LedWaveTypeSawr,
            cfg_.pinLogicalSipoOsc1LedWaveTypeSawl,
            cfg_.pinLogicalSipoOsc1LedWaveTypeSquare,
            cfg_.pinLogicalSipoOsc1LedWaveTypeTriangle,
            cfg_.pinLogicalSipoOsc1LedWaveTypeNone,
        };
        
        for (auto &pinLogical : pinList)
        {
            srOutput_.DigitalWrite(pinLogical, LOW);
        }
    }

    void SetupOsc2Inputs()
    {
        srInput_.SetCallback(cfg_.pinLogicalPisoOsc2ButtonWaveTypeSine, [this](uint8_t){
            ClearOsc2Leds();
            srOutput_.DigitalWrite(cfg_.pinLogicalSipoLfoLedWaveTypeSine, HIGH);
            midiSynth_.SetCfgItem({SET_OSCILLATOR_2_WAVE_TYPE, (uint8_t)OscillatorType::SINE});
        });
        srInput_.SetCallback(cfg_.pinLogicalPisoOsc2ButtonWaveTypeSawr, [this](uint8_t){
            ClearOsc2Leds();
            srOutput_.DigitalWrite(cfg_.pinLogicalSipoLfoLedWaveTypeSawr, HIGH);
            midiSynth_.SetCfgItem({SET_OSCILLATOR_2_WAVE_TYPE, (uint8_t)OscillatorType::SAWR});
        });
        srInput_.SetCallback(cfg_.pinLogicalPisoOsc2ButtonWaveTypeSawl, [this](uint8_t){
            ClearOsc2Leds();
            srOutput_.DigitalWrite(cfg_.pinLogicalSipoLfoLedWaveTypeSawl, HIGH);
            midiSynth_.SetCfgItem({SET_OSCILLATOR_2_WAVE_TYPE, (uint8_t)OscillatorType::SAWL});
        });
        srInput_.SetCallback(cfg_.pinLogicalPisoOsc2ButtonWaveTypeSquare, [this](uint8_t){
            ClearOsc2Leds();
            srOutput_.DigitalWrite(cfg_.pinLogicalSipoLfoLedWaveTypeSquare, HIGH);
            midiSynth_.SetCfgItem({SET_OSCILLATOR_2_WAVE_TYPE, (uint8_t)OscillatorType::SQUARE});
        });
        srInput_.SetCallback(cfg_.pinLogicalPisoOsc2ButtonWaveTypeTriangle, [this](uint8_t){
            ClearOsc2Leds();
            srOutput_.DigitalWrite(cfg_.pinLogicalSipoLfoLedWaveTypeTriangle, HIGH);
            midiSynth_.SetCfgItem({SET_OSCILLATOR_2_WAVE_TYPE, (uint8_t)OscillatorType::TRIANGLE});
        });
        srInput_.SetCallback(cfg_.pinLogicalPisoOsc2ButtonWaveTypeNone, [this](uint8_t){
            ClearOsc2Leds();
            srOutput_.DigitalWrite(cfg_.pinLogicalSipoLfoLedWaveTypeNone, HIGH);
            midiSynth_.SetCfgItem({SET_OSCILLATOR_2_WAVE_TYPE, (uint8_t)OscillatorType::NONE});
        });
    }
    
    void ClearOsc2Leds()
    {
        uint8_t pinList[] = {
            cfg_.pinLogicalSipoOsc2LedWaveTypeSine,
            cfg_.pinLogicalSipoOsc2LedWaveTypeSawr,
            cfg_.pinLogicalSipoOsc2LedWaveTypeSawl,
            cfg_.pinLogicalSipoOsc2LedWaveTypeSquare,
            cfg_.pinLogicalSipoOsc2LedWaveTypeTriangle,
            cfg_.pinLogicalSipoOsc2LedWaveTypeNone,
        };
        
        for (auto &pinLogical : pinList)
        {
            srOutput_.DigitalWrite(pinLogical, LOW);
        }
    }

    void SetupLfoInputs()
    {
        srInput_.SetCallback(cfg_.pinLogicalPisoLfoButtonWaveTypeSine, [this](uint8_t){
            ClearLfoLeds();
            srOutput_.DigitalWrite(cfg_.pinLogicalSipoLfoLedWaveTypeSine, HIGH);
            midiSynth_.SetCfgItem({SET_LFO_WAVE_TYPE, (uint8_t)OscillatorType::SINE});
        });
        srInput_.SetCallback(cfg_.pinLogicalPisoLfoButtonWaveTypeSawr, [this](uint8_t){
            ClearLfoLeds();
            srOutput_.DigitalWrite(cfg_.pinLogicalSipoLfoLedWaveTypeSawr, HIGH);
            midiSynth_.SetCfgItem({SET_LFO_WAVE_TYPE, (uint8_t)OscillatorType::SAWR});
        });
        srInput_.SetCallback(cfg_.pinLogicalPisoLfoButtonWaveTypeSawl, [this](uint8_t){
            ClearLfoLeds();
            srOutput_.DigitalWrite(cfg_.pinLogicalSipoLfoLedWaveTypeSawl, HIGH);
            midiSynth_.SetCfgItem({SET_LFO_WAVE_TYPE, (uint8_t)OscillatorType::SAWL});
        });
        srInput_.SetCallback(cfg_.pinLogicalPisoLfoButtonWaveTypeSquare, [this](uint8_t){
            ClearLfoLeds();
            srOutput_.DigitalWrite(cfg_.pinLogicalSipoLfoLedWaveTypeSquare, HIGH);
            midiSynth_.SetCfgItem({SET_LFO_WAVE_TYPE, (uint8_t)OscillatorType::SQUARE});
        });
        srInput_.SetCallback(cfg_.pinLogicalPisoLfoButtonWaveTypeTriangle, [this](uint8_t){
            ClearLfoLeds();
            srOutput_.DigitalWrite(cfg_.pinLogicalSipoLfoLedWaveTypeTriangle, HIGH);
            midiSynth_.SetCfgItem({SET_LFO_WAVE_TYPE, (uint8_t)OscillatorType::TRIANGLE});
        });
        srInput_.SetCallback(cfg_.pinLogicalPisoLfoButtonWaveTypeNone, [this](uint8_t){
            ClearLfoLeds();
            srOutput_.DigitalWrite(cfg_.pinLogicalSipoLfoLedWaveTypeNone, HIGH);
            midiSynth_.SetCfgItem({SET_LFO_WAVE_TYPE, (uint8_t)OscillatorType::NONE});
        });
    }
    
    void ClearLfoLeds()
    {
        uint8_t pinList[] = {
            cfg_.pinLogicalSipoLfoLedWaveTypeSine,
            cfg_.pinLogicalSipoLfoLedWaveTypeSawr,
            cfg_.pinLogicalSipoLfoLedWaveTypeSawl,
            cfg_.pinLogicalSipoLfoLedWaveTypeSquare,
            cfg_.pinLogicalSipoLfoLedWaveTypeTriangle,
            cfg_.pinLogicalSipoLfoLedWaveTypeNone,
        };
        
        for (auto &pinLogical : pinList)
        {
            srOutput_.DigitalWrite(pinLogical, LOW);
        }
    }



    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

    const AppSynthesizer1Config &cfg_;
    
    ShiftRegisterIn                             srIn_;
    ShiftRegisterInput<SHIFT_REGISTER_IN_COUNT> srInput_;
    
    ShiftRegisterOut                              srOut_;
    ShiftRegisterOutput<SHIFT_REGISTER_OUT_COUNT> srOutput_;
    
    MuxAnalogDigitalCD74HC4067 mux_;
    
    MIDICommandFromSerial  midiReader_;
    MIDISynthesizer        midiSynth_;
    
    MIDICommandFromPianoKeyboardPimoroniHAT  hatToMidi_;

};


#endif  // __APP_SYNTHESIZER1_H__













