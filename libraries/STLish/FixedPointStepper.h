#ifndef __FIXED_POINT_STEPPER__
#define __FIXED_POINT_STEPPER__


template <typename QT>
class FixedPointStepper
{
public:
    
    FixedPointStepper()
    : val_(0.0)
    , stepSize_(0.0)
    , limitLower_(0.0)
    , limitUpper_(0.0)
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
    
    inline void SetStepSize(const QT &stepSize)
    {
        stepSize_ = stepSize;
    }
    
    inline void Incr()
    {
        val_ += stepSize_;
    }
    
    inline void SetLimitLower(double limitLower)
    {
        limitLower_ = limitLower;
    }
    
    inline void SetLimitUpper(double limitUpper)
    {
        limitUpper_ = limitUpper;
    }
    
    void IncrTowardLimit()
    {
        // wrap-safe logic
        QT diffToLimit = limitUpper_ - val_;
     
        if (diffToLimit < stepSize_)
        {
            // taking another step will go past limit
            val_ = limitUpper_;
        }
        else
        {
            // taking another step is safe, won't go past limit
            val_ += stepSize_;
        }
    }
    
    void DecrTowardLimit()
    {
        // wrap-safe logic
        QT diffToLimit = val_ - limitLower_;
     
        if (diffToLimit < stepSize_)
        {
            // taking another step will go past limit
            val_ = limitLower_;
        }
        else
        {
            // taking another step is safe, won't go past limit
            val_ -= stepSize_;
        }
    }

    inline uint8_t GetUnsignedInt8() const
    {
        return val_.GetUnsignedInt8();
    }
    
    
private:

    QT val_;
    QT stepSize_;
    
    QT limitLower_;
    QT limitUpper_;
};


#endif  // __FIXED_POINT_STEPPER__