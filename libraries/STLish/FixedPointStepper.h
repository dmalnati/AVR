#ifndef __FIXED_POINT_STEPPER__
#define __FIXED_POINT_STEPPER__


template <typename QT>
class FixedPointStepper
{
public:
    
    FixedPointStepper()
    : val_(0.0)
    , stepSize_(0.0)
    {
        // Nothing to do
    }
    
    inline void SetValue(double val)
    {
        val_ = val;
    }
    
    inline void SetStepSize(double stepSize)
    {
        stepSize_ = stepSize;
    }

    inline void operator++()
    {
        val_ += stepSize_;
    }
    
    inline operator uint8_t() const
    {
        return (uint8_t)val_;
    }
    
    
private:

    QT val_;
    QT stepSize_;
};


#endif  // __FIXED_POINT_STEPPER__