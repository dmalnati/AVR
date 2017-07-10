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
    
    int8_t GetNextSample()
    {
        int8_t retVal = ss_.GetSampleAtIdx(idxSignalSource_);
        
        ++idxSignalSource_;
        
        return retVal;
    }
    
    void Reset()
    {
        idxSignalSource_.Reset();
    }

private:

    uint16_t  sampleRate_;

    SignalSource  ss_;
    
    typename SignalSource::IdxType  idxSignalSource_;
};


#endif  // __SIGNAL_OSCILLATOR_H__
















