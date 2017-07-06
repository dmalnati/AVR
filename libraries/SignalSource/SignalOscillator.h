#ifndef __SIGNAL_OSCILLATOR_H__
#define __SIGNAL_OSCILLATOR_H__


template <typename SignalSource>
class SignalOscillator
{
public:
    void SetSampleRate(uint16_t sampleRate)
    {
        sampleRate_ = sampleRate;
    }
    
    void SetFrequency(uint16_t frequency)
    {
        idxSignalSource_.Calibrate(sampleRate_, frequency);
    }
    
    uint8_t GetNextSample()
    {
        uint8_t retVal = ss_.GetSampleAtIdx(idxSignalSource_);
        
        ++idxSignalSource_;
        
        return retVal;
    }
    
    void Reset()
    {
        idxSignalSource_.Reset();
    }

private:

    static uint16_t  sampleRate_;
    
    static          SignalSource           ss_;
    static typename SignalSource::IdxType  idxSignalSource_;
};


template <typename SignalSource>
uint16_t SignalOscillator<SignalSource>::sampleRate_;

template <typename SignalSource>
SignalSource SignalOscillator<SignalSource>::ss_;

template <typename SignalSource>
typename SignalSource::IdxType SignalOscillator<SignalSource>::idxSignalSource_;




#endif  // __SIGNAL_OSCILLATOR_H__
















