#ifndef __APP_SYNTHESIZER1_H__
#define __APP_SYNTHESIZER1_H__


#include "Evm.h"

#include "ShiftRegisterIn.h"
#include "ShiftRegisterInputSingleCallback.h"

#include "ShiftRegisterOut.h"
#include "ShiftRegisterOutput.h"

#include "PinInputAnalog.h"

#include "MuxAnalogDigitalCD74HC4067.h"

#include "PianoKeyboardPimoroniHAT.h"
#include "MIDICommandFromPianoKeyboardPimoroniHAT.h"

#include "MIDICommandFrom@fix@Serial.h"
#include "MIDISynthesizer.h"

#include "LCDFrentaly20x4.h"


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
    static const uint8_t C_IDLE  = 0;
    static const uint8_t C_TIMED = 30;
    static const uint8_t C_INTER = 0;
    
    static const uint16_t BAUD = 31250;
    
    static const uint32_t DEFAULT_ANALOG_POLL_PERIOD_MS = 20;
    static const uint16_t DEFAULT_ANALOG_MINIMUM_CHANGE = 15;
    
    static const uint8_t SHIFT_REGISTER_IN_COUNT  = 3;
    static const uint8_t SHIFT_REGISTER_OUT_COUNT = 3;
    
    static const uint16_t MAX_OSC_FREQ = 1000;
    static const uint16_t MAX_LFO_FREQ =   60;
    
    static const uint16_t MAX_ENVELOPE_ATTACK_DURATION_MS  = 1500;
    static const uint16_t MAX_ENVELOPE_DECAY_DURATION_MS   = 1500;
    static const uint16_t MAX_ENVELOPE_RELEASE_DURATION_MS = 1500;
    
public:
    AppSynthesizer1(const AppSynthesizer1Config &cfg)
    : cfg_(cfg)
    , srIn_(cfg_.pinPisoLoad, cfg_.pinPisoClock, cfg_.pinPisoClockEnable, cfg_.pinPisoSerial)
    , srInputSC_(srIn_)
    , srOut_(cfg_.pinSipoClock, cfg_.pinSipoLatch, cfg_.pinSipoSerial)
    , srOutput_(srOut_)
    , mux_(cfg_.pinMuxBit0, cfg_.pinMuxBit1, cfg_.pinMuxBit2, cfg_.pinMuxBit3, cfg_.pinMuxAnalog)
    , piaOsc1Freq_(cfg_.pinMuxAnalog, DEFAULT_ANALOG_POLL_PERIOD_MS)
    , piaOsc2Freq_(cfg_.pinMuxAnalog, DEFAULT_ANALOG_POLL_PERIOD_MS)
    , piaOscBalance_(cfg_.pinMuxAnalog, DEFAULT_ANALOG_POLL_PERIOD_MS)
    , piaLfoFreq_(cfg_.pinMuxAnalog, DEFAULT_ANALOG_POLL_PERIOD_MS)
    , piaLfoTromello_(cfg_.pinMuxAnalog, DEFAULT_ANALOG_POLL_PERIOD_MS)
    , piaLfoVibrato_(cfg_.pinMuxAnalog, DEFAULT_ANALOG_POLL_PERIOD_MS)
    , piaEnvAttack_(cfg_.pinMuxAnalog, DEFAULT_ANALOG_POLL_PERIOD_MS)
    , piaEnvDecay_(cfg_.pinMuxAnalog, DEFAULT_ANALOG_POLL_PERIOD_MS)
    , piaEnvSustain_(cfg_.pinMuxAnalog, DEFAULT_ANALOG_POLL_PERIOD_MS)
    , piaEnvRelease_(cfg_.pinMuxAnalog, DEFAULT_ANALOG_POLL_PERIOD_MS)
    {
        // Nothing to do
    }
    
    void Run()
    {
        // Operate the serial port
        @fix@Serial.begin(BAUD);
        
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
        SetupDigitalInputs();
        
        // Setup analog inputs
        SetupAnalogInputs();
        
        // Direct inbound MIDI to the synthesizer
        midiReader_.SetCallbackOnMIDICommand([this](MIDICommand cmd){
            midiSynth_.ProcessCommand(cmd);
            
            UpdateLCDAfterMidiCommand();
            UpdateLEDsAfterMidiCommand();
        });
        midiReader_.Start();
        
        // Set up keyboard
        hatToMidi_.GetHat().EnableLEDs();
        hatToMidi_.GetHat().SetIgnoreKeyCNextOctave(1);
        hatToMidi_.Init();
        
        // Set up LCD Printer
        printer_.Init();
        
        // Set up defaults by simulating button presses
        OnPinChange(cfg_.pinLogicalPisoPhaseLock,
                    srInputSC_.DigitalReadCached(cfg_.pinLogicalPisoPhaseLock));
        
        OnPinChange(cfg_.pinLogicalPisoOsc1ButtonWaveTypeNone, 1);
        OnPinChange(cfg_.pinLogicalPisoOsc2ButtonWaveTypeNone, 1);
        OnPinChange(cfg_.pinLogicalPisoLfoButtonWaveTypeNone, 1);
        
        OnPinChange(cfg_.pinLogicalPisoEnvEnable,
                    srInputSC_.DigitalReadCached(cfg_.pinLogicalPisoEnvEnable));
    }
    
    void SetupDigitalInputs()
    {
        srInputSC_.SetCallback([this](uint8_t pinLogical, uint8_t logicLevel){
            OnPinChange(pinLogical, logicLevel);
        });
        
        srInputSC_.Init();
    }
    
    void OnPinChange(uint8_t pinLogical, uint8_t logicLevel)
    {
        if (pinLogical == cfg_.pinLogicalPisoPhaseLock)
        {
            midiSynth_.SetCfgItem({SET_PHASE_LOCK, logicLevel});
        }
        
        // Group wave selection , and only examine if logic level
        // is rising, since that's all they care about
        if (logicLevel)
        {
            // Oscillator 1
            if (pinLogical == cfg_.pinLogicalPisoOsc1ButtonWaveTypeSine)
            {
                SetOsc1Led(pinLogical);
                midiSynth_.SetCfgItem({SET_OSCILLATOR_1_WAVE_TYPE, (uint8_t)OscillatorType::SINE});
            }
            else if (pinLogical == cfg_.pinLogicalPisoOsc1ButtonWaveTypeSawr)
            {
                SetOsc1Led(pinLogical);
                midiSynth_.SetCfgItem({SET_OSCILLATOR_1_WAVE_TYPE, (uint8_t)OscillatorType::SAWR});
            }
            else if (pinLogical == cfg_.pinLogicalPisoOsc1ButtonWaveTypeSawl)
            {
                SetOsc1Led(pinLogical);
                midiSynth_.SetCfgItem({SET_OSCILLATOR_1_WAVE_TYPE, (uint8_t)OscillatorType::SAWL});
            }
            else if (pinLogical == cfg_.pinLogicalPisoOsc1ButtonWaveTypeSquare)
            {
                SetOsc1Led(pinLogical);
                midiSynth_.SetCfgItem({SET_OSCILLATOR_1_WAVE_TYPE, (uint8_t)OscillatorType::SQUARE});
            }
            else if (pinLogical == cfg_.pinLogicalPisoOsc1ButtonWaveTypeTriangle)
            {
                SetOsc1Led(pinLogical);
                midiSynth_.SetCfgItem({SET_OSCILLATOR_1_WAVE_TYPE, (uint8_t)OscillatorType::TRIANGLE});
            }
            else if (pinLogical == cfg_.pinLogicalPisoOsc1ButtonWaveTypeNone)
            {
                SetOsc1Led(pinLogical);
                midiSynth_.SetCfgItem({SET_OSCILLATOR_1_WAVE_TYPE, (uint8_t)OscillatorType::NONE});
            }
            
            // Oscillator 2
            if (pinLogical == cfg_.pinLogicalPisoOsc2ButtonWaveTypeSine)
            {
                SetOsc2Led(pinLogical);
                midiSynth_.SetCfgItem({SET_OSCILLATOR_2_WAVE_TYPE, (uint8_t)OscillatorType::SINE});
            }
            else if (pinLogical == cfg_.pinLogicalPisoOsc2ButtonWaveTypeSawr)
            {
                SetOsc2Led(pinLogical);
                midiSynth_.SetCfgItem({SET_OSCILLATOR_2_WAVE_TYPE, (uint8_t)OscillatorType::SAWR});
            }
            else if (pinLogical == cfg_.pinLogicalPisoOsc2ButtonWaveTypeSawl)
            {
                SetOsc2Led(pinLogical);
                midiSynth_.SetCfgItem({SET_OSCILLATOR_2_WAVE_TYPE, (uint8_t)OscillatorType::SAWL});
            }
            else if (pinLogical == cfg_.pinLogicalPisoOsc2ButtonWaveTypeSquare)
            {
                SetOsc2Led(pinLogical);
                midiSynth_.SetCfgItem({SET_OSCILLATOR_2_WAVE_TYPE, (uint8_t)OscillatorType::SQUARE});
            }
            else if (pinLogical == cfg_.pinLogicalPisoOsc2ButtonWaveTypeTriangle)
            {
                SetOsc2Led(pinLogical);
                midiSynth_.SetCfgItem({SET_OSCILLATOR_2_WAVE_TYPE, (uint8_t)OscillatorType::TRIANGLE});
            }
            else if (pinLogical == cfg_.pinLogicalPisoOsc2ButtonWaveTypeNone)
            {
                SetOsc2Led(pinLogical);
                midiSynth_.SetCfgItem({SET_OSCILLATOR_2_WAVE_TYPE, (uint8_t)OscillatorType::NONE});
            }
            
            // LFO
            if (pinLogical == cfg_.pinLogicalPisoLfoButtonWaveTypeSine)
            {
                SetLfoLed(pinLogical);
                midiSynth_.SetCfgItem({SET_LFO_WAVE_TYPE, (uint8_t)OscillatorType::SINE});
            }
            else if (pinLogical == cfg_.pinLogicalPisoLfoButtonWaveTypeSawr)
            {
                SetLfoLed(pinLogical);
                midiSynth_.SetCfgItem({SET_LFO_WAVE_TYPE, (uint8_t)OscillatorType::SAWR});
            }
            else if (pinLogical == cfg_.pinLogicalPisoLfoButtonWaveTypeSawl)
            {
                SetLfoLed(pinLogical);
                midiSynth_.SetCfgItem({SET_LFO_WAVE_TYPE, (uint8_t)OscillatorType::SAWL});
            }
            else if (pinLogical == cfg_.pinLogicalPisoLfoButtonWaveTypeSquare)
            {
                SetLfoLed(pinLogical);
                midiSynth_.SetCfgItem({SET_LFO_WAVE_TYPE, (uint8_t)OscillatorType::SQUARE});
            }
            else if (pinLogical == cfg_.pinLogicalPisoLfoButtonWaveTypeTriangle)
            {
                SetLfoLed(pinLogical);
                midiSynth_.SetCfgItem({SET_LFO_WAVE_TYPE, (uint8_t)OscillatorType::TRIANGLE});
            }
            else if (pinLogical == cfg_.pinLogicalPisoLfoButtonWaveTypeNone)
            {
                SetLfoLed(pinLogical);
                midiSynth_.SetCfgItem({SET_LFO_WAVE_TYPE, (uint8_t)OscillatorType::NONE});
            }
        }
        
        // Examine Envelope pins.
        if (pinLogical == cfg_.pinLogicalPisoEnvEnable)
        {
            midiSynth_.SetCfgItem({SET_ENVELOPE_ON_OFF, logicLevel});
        }
        else if (pinLogical == cfg_.pinLogicalPisoEnvPulse)
        {
            if (logicLevel)
            {
                midiSynth_.OnKeyDown();
            }
            else
            {
                midiSynth_.OnKeyUp();
            }
        }
    }
    
    void SetOsc1Led(uint8_t pinLogicalNowActive)
    {
        static uint8_t pinLogicalLast = 255;
        
        // Check if the pin is already active, do nothing if so
        if (pinLogicalLast != pinLogicalNowActive)
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
            
            srOutput_.DigitalWrite(pinLogicalNowActive, HIGH);
        }
        
        pinLogicalLast = pinLogicalNowActive;
    }
    
    void SetOsc2Led(uint8_t pinLogicalNowActive)
    {
        static uint8_t pinLogicalLast = 255;
        
        // Check if the pin is already active, do nothing if so
        if (pinLogicalLast != pinLogicalNowActive)
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
            
            srOutput_.DigitalWrite(pinLogicalNowActive, HIGH);
        }
        
        pinLogicalLast = pinLogicalNowActive;
    }

    void SetLfoLed(uint8_t pinLogicalNowActive)
    {
        static uint8_t pinLogicalLast = 255;
        
        // Check if the pin is already active, do nothing if so
        if (pinLogicalLast != pinLogicalNowActive)
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
            
            srOutput_.DigitalWrite(pinLogicalNowActive, HIGH);
        }
        
        pinLogicalLast = pinLogicalNowActive;
    }
    
    void SetupAnalogInputs()
    {
        // Configure all analog inputs to use mux as input source and
        // enable input monitoring
        {
            PinInputAnalog *piaList[] = {
                &piaOsc1Freq_,
                &piaOsc2Freq_,
                &piaOscBalance_,
                &piaLfoFreq_,
                &piaLfoTromello_,
                &piaLfoVibrato_,
                &piaEnvAttack_,
                &piaEnvDecay_,
                &piaEnvSustain_,
                &piaEnvRelease_,
            };

            uint8_t channel = 0;
            for (auto pia : piaList)
            {
                pia->SetAnalogReadFunction([=](uint8_t pin){
                    mux_.ConnectToChannel(channel);
            
                    return PAL.AnalogRead(pin);
                });
                
                pia->SetMinimumChange(DEFAULT_ANALOG_MINIMUM_CHANGE);
                pia->Enable();
                
                ++channel;
            }
        }
        
        
        
        // Set callbacks
        // Broken out into loops to minimize lambda creation and therefore
        // vtable ram usage as a result of distinct function<> types
        {
            PinInputAnalog *piaList[] = {
                &piaOsc1Freq_,
                &piaOsc2Freq_,
                &piaLfoFreq_,
            };
            
            uint8_t cfgTypeList[] = {
                SET_OSCILLATOR_1_FREQUENCY,
                SET_OSCILLATOR_2_FREQUENCY,
                SET_LFO_FREQUENCY,
            };
            
            uint16_t scaleToList[] = {
                MAX_OSC_FREQ,
                MAX_OSC_FREQ,
                MAX_LFO_FREQ,
            };
            
            uint8_t count = 0;
            for (auto pia : piaList)
            {
                uint8_t  cfgType = cfgTypeList[count];
                uint16_t scaleTo = scaleToList[count];
                
                pia->SetCallback([=](uint16_t val){
                    midiSynth_.SetCfgItem({cfgType, ScaleTo16(val, scaleTo)});
                    
                    printer_.OnParamChange({cfgType, ScaleTo16(val, scaleTo)});
                });
                
                ++count;
            }
        }
        
        {
            PinInputAnalog *piaList[] = {
                &piaOscBalance_,
                &piaLfoTromello_,
                &piaLfoVibrato_,
                &piaEnvSustain_,
            };
            
            uint8_t cfgTypeList[] = {
                SET_OSCILLATOR_BALANCE,
                SET_LFO_TROMOLO_PCT,
                SET_LFO_VIBRATO_PCT,
                SET_ENVELOPE_SUSTAIN_LEVEL_PCT,
            };
            
            uint8_t scaleToList[] = {
                255,
                255,
                255,
                100
            };
            
            uint8_t count = 0;
            for (auto pia : piaList)
            {
                uint8_t cfgType = cfgTypeList[count];
                uint8_t scaleTo = scaleToList[count];
                
                pia->SetCallback([=](uint16_t val){
                    midiSynth_.SetCfgItem({cfgType, ScaleTo8(val, scaleTo)});
                    
                    printer_.OnParamChange({cfgType, ScaleTo8(val, scaleTo)});
                });
                
                ++count;
            }
        }
        
        {
            PinInputAnalog *piaList[] = {
                &piaEnvAttack_,
                &piaEnvDecay_,
                &piaEnvRelease_,
            };
            
            uint8_t cfgTypeList[] = {
                SET_ENVELOPE_ATTACK_DURATION_MS,
                SET_ENVELOPE_DECAY_DURATION_MS,
                SET_ENVELOPE_RELEASE_DURATION_MS,
            };
            
            uint16_t scaleToList[] = {
                MAX_ENVELOPE_ATTACK_DURATION_MS,
                MAX_ENVELOPE_DECAY_DURATION_MS,
                MAX_ENVELOPE_RELEASE_DURATION_MS,
            };
            
            uint8_t count = 0;
            for (auto pia : piaList)
            {
                uint8_t  cfgType = cfgTypeList[count];
                uint16_t scaleTo = scaleToList[count];
                
                pia->SetCallback([=](uint16_t val){
                    midiSynth_.SetCfgItem({cfgType, ScaleTo16(val, scaleTo)});
                    
                    printer_.OnParamChange({cfgType, ScaleTo16(val, scaleTo)});
                });
                
                ++count;
            }
        }
    }
    
    uint8_t ScaleTo8(uint16_t val, uint8_t top)
    {
        return (double)val / PAL.AnalogMaxValue() * top;
    }
    
    uint16_t ScaleTo16(uint16_t val, uint16_t top)
    {
        return (double)val / PAL.AnalogMaxValue() * top;
    }

    
    void UpdateLCDAfterMidiCommand()
    {
        uint8_t cfgTypeList[] = {
            SET_OSCILLATOR_1_FREQUENCY,
            SET_OSCILLATOR_2_FREQUENCY,
            SET_LFO_FREQUENCY,
            SET_LFO_VIBRATO_PCT,
            SET_LFO_TROMOLO_PCT,
            SET_ENVELOPE_ATTACK_DURATION_MS,
            SET_ENVELOPE_DECAY_DURATION_MS,
            SET_ENVELOPE_SUSTAIN_LEVEL_PCT,
            SET_ENVELOPE_RELEASE_DURATION_MS,
        };

        for (auto cfgType : cfgTypeList)
        {
            CfgItem c;
            
            if (midiSynth_.GetCfgItem(cfgType, c))
            {
                printer_.OnParamChange(c);
            }
        }
    }

    void UpdateLEDsAfterMidiCommand()
    {
        uint8_t cfgTypeList[] = {
            SET_OSCILLATOR_1_WAVE_TYPE,
            SET_OSCILLATOR_2_WAVE_TYPE,
            SET_LFO_WAVE_TYPE,
        };
        
        struct WaveToPin
        {
            OscillatorType waveType;
            uint8_t        pinLogicalPiso;
        };
        
        const uint8_t WAVE_TO_PIN_LIST_LEN = 6;
        WaveToPin waveToPinListOsc1[] = {
            { OscillatorType::SINE,     cfg_.pinLogicalPisoOsc1ButtonWaveTypeSine     },
            { OscillatorType::SAWR,     cfg_.pinLogicalPisoOsc1ButtonWaveTypeSawr     },
            { OscillatorType::SAWL,     cfg_.pinLogicalPisoOsc1ButtonWaveTypeSawl     },
            { OscillatorType::SQUARE,   cfg_.pinLogicalPisoOsc1ButtonWaveTypeSquare   },
            { OscillatorType::TRIANGLE, cfg_.pinLogicalPisoOsc1ButtonWaveTypeTriangle },
            { OscillatorType::NONE,     cfg_.pinLogicalPisoOsc1ButtonWaveTypeNone     },
        };
        WaveToPin waveToPinListOsc2[] = {
            { OscillatorType::SINE,     cfg_.pinLogicalPisoOsc2ButtonWaveTypeSine     },
            { OscillatorType::SAWR,     cfg_.pinLogicalPisoOsc2ButtonWaveTypeSawr     },
            { OscillatorType::SAWL,     cfg_.pinLogicalPisoOsc2ButtonWaveTypeSawl     },
            { OscillatorType::SQUARE,   cfg_.pinLogicalPisoOsc2ButtonWaveTypeSquare   },
            { OscillatorType::TRIANGLE, cfg_.pinLogicalPisoOsc2ButtonWaveTypeTriangle },
            { OscillatorType::NONE,     cfg_.pinLogicalPisoOsc2ButtonWaveTypeNone     },
        };
        WaveToPin waveToPinListLfo[] = {
            { OscillatorType::SINE,     cfg_.pinLogicalPisoLfoButtonWaveTypeSine     },
            { OscillatorType::SAWR,     cfg_.pinLogicalPisoLfoButtonWaveTypeSawr     },
            { OscillatorType::SAWL,     cfg_.pinLogicalPisoLfoButtonWaveTypeSawl     },
            { OscillatorType::SQUARE,   cfg_.pinLogicalPisoLfoButtonWaveTypeSquare   },
            { OscillatorType::TRIANGLE, cfg_.pinLogicalPisoLfoButtonWaveTypeTriangle },
            { OscillatorType::NONE,     cfg_.pinLogicalPisoLfoButtonWaveTypeNone     },
        };
        
        const uint8_t OSC_COUNT = 3;
        WaveToPin *waveToPinListList[] = {
            waveToPinListOsc1,
            waveToPinListOsc2,
            waveToPinListLfo,
        };
        
        for (uint8_t i = 0; i < OSC_COUNT; ++i)
        {
            // Find out which wave type is active
            uint8_t cfgType = cfgTypeList[i];
            
            // Get the mapping between wave type and pin to trigger
            WaveToPin *waveToPinList = waveToPinListList[i];
            
            CfgItem c;
            if (midiSynth_.GetCfgItem(cfgType, c))
            {
                OscillatorType waveType = (OscillatorType)(uint8_t)c;
                
                // Simulate a button press, which will light up the LED
                for (uint8_t j = 0; j < WAVE_TO_PIN_LIST_LEN; ++j)
                {
                    WaveToPin wtp = waveToPinList[j];
                    
                    if (wtp.waveType == waveType)
                    {
                        OnPinChange(wtp.pinLogicalPiso, 1);
                    }
                }
            }
        }
    }


    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

    const AppSynthesizer1Config &cfg_;
    
    ShiftRegisterIn                                           srIn_;
    ShiftRegisterInputSingleCallback<SHIFT_REGISTER_IN_COUNT> srInputSC_;
    
    ShiftRegisterOut                              srOut_;
    ShiftRegisterOutput<SHIFT_REGISTER_OUT_COUNT> srOutput_;
    
    MuxAnalogDigitalCD74HC4067 mux_;
    
    PinInputAnalog piaOsc1Freq_;
    PinInputAnalog piaOsc2Freq_;
    PinInputAnalog piaOscBalance_;
    PinInputAnalog piaLfoFreq_;
    PinInputAnalog piaLfoTromello_;
    PinInputAnalog piaLfoVibrato_;
    PinInputAnalog piaEnvAttack_;
    PinInputAnalog piaEnvDecay_;
    PinInputAnalog piaEnvSustain_;
    PinInputAnalog piaEnvRelease_;
    
    MIDICommandFromSerial  midiReader_;
    MIDISynthesizer        midiSynth_;
    
    MIDICommandFromPianoKeyboardPimoroniHAT  hatToMidi_;
    
    
    /*
     * 01234567890123456789
     *  
     * O1  xxxxx  O2 xxxxx
     * LFO xxxxx VT xxx xxx
     * A xxxxms   D xxxxms
     * S  xxx%    R xxxxms
     */
    class LCDPrinter
    {
        // Space for 5-digit 16-bit number, plus null
        static const uint8_t MAX_WIDTH = 5;
        static const uint8_t BUF_SIZE  = MAX_WIDTH + 1;
        
    public:

        void Init()
        {
            lcd_.Init();
            
            lcd_.PrintAt( 0, 0, "O1");
            lcd_.PrintAt(11, 0, "O2");
            lcd_.PrintAt( 0, 1, "LFO");
            lcd_.PrintAt(10, 1, "VT");
            lcd_.PrintAt( 0, 2, 'A');
            lcd_.PrintAt(11, 2, 'D');
            lcd_.PrintAt( 0, 3, 'S');
            lcd_.PrintAt(11, 3, 'R');
            
            struct { uint8_t x; uint8_t y; } xyList[] = {
                {  6, 2 },
                { 17, 2 },
                { 17, 3 },
            };
            
            for (auto xy : xyList)
            {
                lcd_.PrintAt(xy.x, xy.y, "ms");
            }
            
            lcd_.PrintAt(6, 3, '%');
        }

        void OnParamChange(CfgItem c)
        {
            if (c.type == SET_OSCILLATOR_1_FREQUENCY)
            {
                static uint16_t valLast = UINT16_MAX;
                
                if (valLast == UINT16_MAX || valLast != (uint16_t)c)
                {
                    lcd_.PrintAt(4, 0, ToStr((uint16_t)c, 5));
                }
                
                valLast = (uint16_t)c;
            }
            else if (c.type == SET_OSCILLATOR_2_FREQUENCY)
            {
                static uint16_t valLast = UINT16_MAX;
                
                if (valLast == UINT16_MAX || valLast != (uint16_t)c)
                {
                    lcd_.PrintAt(14, 0, ToStr((uint16_t)c, 5));
                }
                
                valLast = (uint16_t)c;
            }
            else if (c.type == SET_LFO_FREQUENCY)
            {
                static uint16_t valLast = UINT16_MAX;
                
                if (valLast == UINT16_MAX || valLast != (uint16_t)c)
                {
                    lcd_.PrintAt(4, 1, ToStr((uint16_t)c, 5));
                }
                
                valLast = (uint16_t)c;
            }
            else if (c.type == SET_LFO_VIBRATO_PCT)
            {
                static uint8_t valLast = UINT8_MAX;
                
                if (valLast == UINT8_MAX || valLast != (uint8_t)c)
                {
                    lcd_.PrintAt(13, 1, ToStr(((uint8_t)c * 100.0 / 255.0), 3));
                }
                
                valLast = (uint8_t)c;
            }
            else if (c.type == SET_LFO_TROMOLO_PCT)
            {
                static uint8_t valLast = UINT8_MAX;
                
                if (valLast == UINT8_MAX || valLast != (uint8_t)c)
                {
                    lcd_.PrintAt(17, 1, ToStr(((uint8_t)c * 100.0 / 255.0), 3));
                }
                
                valLast = (uint8_t)c;
            }
            else if (c.type == SET_ENVELOPE_ATTACK_DURATION_MS)
            {
                static uint16_t valLast = UINT16_MAX;
                
                if (valLast == UINT16_MAX || valLast != (uint16_t)c)
                {
                    lcd_.PrintAt(2, 2, ToStr((uint16_t)c, 4));
                }
                
                valLast = (uint16_t)c;
            }
            else if (c.type == SET_ENVELOPE_DECAY_DURATION_MS)
            {
                static uint16_t valLast = UINT16_MAX;
                
                if (valLast == UINT16_MAX || valLast != (uint16_t)c)
                {
                    lcd_.PrintAt(13, 2, ToStr((uint16_t)c, 4));
                }
                
                valLast = (uint16_t)c;
            }
            else if (c.type == SET_ENVELOPE_SUSTAIN_LEVEL_PCT)
            {
                static uint8_t valLast = UINT8_MAX;
                
                if (valLast == UINT8_MAX || valLast != (uint8_t)c)
                {
                    lcd_.PrintAt(3, 3, ToStr((uint8_t)c, 3));
                }
                
                valLast = (uint8_t)c;
            }
            else if (c.type == SET_ENVELOPE_RELEASE_DURATION_MS)
            {
                static uint16_t valLast = UINT16_MAX;
                
                if (valLast == UINT16_MAX || valLast != (uint16_t)c)
                {
                    lcd_.PrintAt(13, 3, ToStr((uint16_t)c, 4));
                }
                
                valLast = (uint16_t)c;
            }
        }

    private:
    
        char *ToStr(uint16_t val, uint8_t width)
        {
            char *retVal = buf_;
            
            buf_[0] = 'E'; buf_[1] = 'R'; buf_[2] = 'R'; buf_[3] = '\0';
            
            // Make sure it fits and can't become negative upon conversion
            if (width <= MAX_WIDTH && val <= (UINT16_MAX / 2))
            {
                utoa((int16_t)val, buf_, 10);
                
                uint8_t len = strlen(buf_);
                
                if (len < width)
                {
                    memmove(&buf_[width - len], buf_, len + 1);
                    memset(buf_, ' ', width - len);
                }
            }
            
            return retVal;
        }

        LCDFrentaly20x4 lcd_;
        
        char buf_[BUF_SIZE];
    };
    
    LCDPrinter printer_;
};


#endif  // __APP_SYNTHESIZER1_H__













