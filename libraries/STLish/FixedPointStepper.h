#ifndef __FIXED_POINT_STEPPER__
#define __FIXED_POINT_STEPPER__


template <typename QT, uint16_t BUCKET_COUNT, uint16_t RESET_VALUE = 0>
class FixedPointStepper
{
public:
    
    FixedPointStepper()
    : idx_(0.0)
    , stepSize_(0.0)
    {
        // Nothing to do
    }
    
    void Calibrate(uint16_t sampleRate, uint16_t frequency)
    {
        if (sampleRate && frequency)
        {
            stepSize_ =
                (double)BUCKET_COUNT / ((double)sampleRate / (double)frequency);
        }
    }

    inline void operator++()
    {
        idx_ += stepSize_;
    }
    
    void Reset()
    {
        idx_ = (double)RESET_VALUE;
    }
    
    inline operator uint8_t() const
    {
        return (uint8_t)idx_;
    }
    
    
private:

    QT idx_;
    QT stepSize_;
};


#endif  // __FIXED_POINT_STEPPER__