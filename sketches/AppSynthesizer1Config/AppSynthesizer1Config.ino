#include "AppSynthesizer1.h"

static struct AppSynthesizer1Config cfg = {
    // Synth
    .pinLogicalPisoPhaseLock = 6,


    // Oscillator 1
    .pinLogicalPisoOsc1ButtonWaveTypeSine     = 0,
    .pinLogicalPisoOsc1ButtonWaveTypeSawr     = 1,
    .pinLogicalPisoOsc1ButtonWaveTypeSawl     = 2,
    .pinLogicalPisoOsc1ButtonWaveTypeSquare   = 3,
    .pinLogicalPisoOsc1ButtonWaveTypeTriangle = 4,
    .pinLogicalPisoOsc1ButtonWaveTypeNone     = 5,

    .pinLogicalSipoOsc1LedWaveTypeSine     = 0,
    .pinLogicalSipoOsc1LedWaveTypeSawr     = 1,
    .pinLogicalSipoOsc1LedWaveTypeSawl     = 2,
    .pinLogicalSipoOsc1LedWaveTypeSquare   = 3,
    .pinLogicalSipoOsc1LedWaveTypeTriangle = 4,
    .pinLogicalSipoOsc1LedWaveTypeNone     = 5,
    
    .pinLogicalMuxOsc1Frequency = 0,
    
    
    // Oscillator 2
    .pinLogicalPisoOsc2ButtonWaveTypeSine     = 8,
    .pinLogicalPisoOsc2ButtonWaveTypeSawr     = 9,
    .pinLogicalPisoOsc2ButtonWaveTypeSawl     = 10,
    .pinLogicalPisoOsc2ButtonWaveTypeSquare   = 11,
    .pinLogicalPisoOsc2ButtonWaveTypeTriangle = 12,
    .pinLogicalPisoOsc2ButtonWaveTypeNone     = 13,

    .pinLogicalSipoOsc2LedWaveTypeSine     = 8,
    .pinLogicalSipoOsc2LedWaveTypeSawr     = 9,
    .pinLogicalSipoOsc2LedWaveTypeSawl     = 10,
    .pinLogicalSipoOsc2LedWaveTypeSquare   = 11,
    .pinLogicalSipoOsc2LedWaveTypeTriangle = 12,
    .pinLogicalSipoOsc2LedWaveTypeNone     = 13,
    
    .pinLogicalMuxOsc2Frequency = 1,
    
    
    // Oscillator Balance
    .pinLogicalMuxOscBalance = 2,


    // LFO
    .pinLogicalPisoLfoButtonWaveTypeSine     = 16,
    .pinLogicalPisoLfoButtonWaveTypeSawr     = 17,
    .pinLogicalPisoLfoButtonWaveTypeSawl     = 18,
    .pinLogicalPisoLfoButtonWaveTypeSquare   = 19,
    .pinLogicalPisoLfoButtonWaveTypeTriangle = 20,
    .pinLogicalPisoLfoButtonWaveTypeNone     = 21,

    .pinLogicalSipoLfoLedWaveTypeSine     = 16,
    .pinLogicalSipoLfoLedWaveTypeSawr     = 17,
    .pinLogicalSipoLfoLedWaveTypeSawl     = 18,
    .pinLogicalSipoLfoLedWaveTypeSquare   = 19,
    .pinLogicalSipoLfoLedWaveTypeTriangle = 20,
    .pinLogicalSipoLfoLedWaveTypeNone     = 21,

    .pinLogicalMuxLfoFrequency = 3,
    
    .pinLogicalMuxTromello = 4,
    .pinLogicalMuxVibrato = 5,


    // Envelope
    .pinLogicalPisoEnvEnable = 22,
    .pinLogicalPisoEnvPulse  = 23,
    .pinLogicalMuxEnvAttack  = 6,
    .pinLogicalMuxEnvDecay   = 7,
    .pinLogicalMuxEnvSustain = 8,
    .pinLogicalMuxEnvRelease = 9,


    // Physical Pin Layout
    .pinPisoLoad        = 9,
    .pinPisoClock       = 10,
    .pinPisoClockEnable = 14,
    .pinPisoSerial      = 15,

    .pinSipoClock  = 23,
    .pinSipoLatch  = 24,
    .pinSipoSerial = 25,

    .pinMuxBit0   = 16,
    .pinMuxBit1   = 17,
    .pinMuxBit2   = 18,
    .pinMuxBit3   = 19,
    .pinMuxAnalog = 26,
};


static AppSynthesizer1 app(cfg);


void setup()
{
    app.Run();
}


void loop() {}


















