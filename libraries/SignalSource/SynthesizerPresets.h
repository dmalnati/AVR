#ifndef __SYNTHESIZER_PRESETS_H__
#define __SYNTHESIZER_PRESETS_H__



void ApplyInstrumentConfiguration()
{
    ((*this).*(instrumentPresetList_[instrumentPresetListIdx_]))();
}


///////////////////////////////////////////////////////////////////////
//
// Presets
//
///////////////////////////////////////////////////////////////////////

void SetInstrumentFlute1()
{
    SetCfgItemList((CfgItem[]){
        {SET_OSCILLATOR_1_WAVE_TYPE, (uint8_t)OscillatorType::SINE},
        {SET_OSCILLATOR_2_WAVE_TYPE, (uint8_t)OscillatorType::NONE},
        {SET_LFO_WAVE_TYPE,          (uint8_t)OscillatorType::NONE},
    });
    
    SetEnvelopeFast();
}

void SetInstrumentFlute2()
{
    SetCfgItemList((CfgItem[]){
        {SET_OSCILLATOR_1_WAVE_TYPE, (uint8_t)OscillatorType::SQUARE},
        {SET_OSCILLATOR_2_WAVE_TYPE, (uint8_t)OscillatorType::TRIANGLE},
        {SET_OSCILLATOR_BALANCE,     127},
        {SET_LFO_WAVE_TYPE,          (uint8_t)OscillatorType::NONE},
    });
    
    SetEnvelopeFast();
}

void SetInstrumentSynth1()
{
    SetCfgItemList((CfgItem[]){
        {SET_OSCILLATOR_1_WAVE_TYPE, (uint8_t)OscillatorType::SAWR},
        {SET_OSCILLATOR_2_WAVE_TYPE, (uint8_t)OscillatorType::SAWR},
        {SET_OSCILLATOR_BALANCE,     127},
        {SET_LFO_WAVE_TYPE,          (uint8_t)OscillatorType::NONE},
    });
    
    SetEnvelopeFast();
}

void SetInstrumentSynth2()
{
    SetCfgItemList((CfgItem[]){
        {SET_OSCILLATOR_1_WAVE_TYPE, (uint8_t)OscillatorType::TRIANGLE},
        {SET_OSCILLATOR_2_WAVE_TYPE, (uint8_t)OscillatorType::NONE},
        {SET_LFO_WAVE_TYPE,          (uint8_t)OscillatorType::NONE},
    });
    
    SetEnvelopeSlow();
}


void SetEnvelopeFast()
{
    SetCfgItemList((CfgItem[]){
        {SET_ENVELOPE_ATTACK_DURATION_MS,  150},
        {SET_ENVELOPE_DECAY_DURATION_MS,     0},
        {SET_ENVELOPE_SUSTAIN_LEVEL_PCT,   100},
        {SET_ENVELOPE_RELEASE_DURATION_MS, 150},
    });
}

void SetEnvelopeSlow()
{
    SetCfgItemList((CfgItem[]){
        {SET_ENVELOPE_ATTACK_DURATION_MS,   150},
        {SET_ENVELOPE_DECAY_DURATION_MS,    100},
        {SET_ENVELOPE_SUSTAIN_LEVEL_PCT,     80},
        {SET_ENVELOPE_RELEASE_DURATION_MS, 1000},
    });
}


template <uint8_t N>
void SetCfgItemList(const CfgItem (&& cfgItemList)[N])
{
    for (auto &cfgItem : cfgItemList)
    {
        SetCfgItem(cfgItem);
    }
}



///////////////////////////////////////////////////////////////////////
//
// Members
//
///////////////////////////////////////////////////////////////////////



static const uint8_t INSTRUMNET_PRESET_COUNT = 4;
FnPtr instrumentPresetList_[INSTRUMNET_PRESET_COUNT] = {
    &Synthesizer::SetInstrumentFlute1,
    &Synthesizer::SetInstrumentFlute2,
    &Synthesizer::SetInstrumentSynth1,
    &Synthesizer::SetInstrumentSynth2,
};
uint8_t instrumentPresetListIdx_ = 0;








#endif  // __SYNTHESIZER_PRESETS_H__





