#ifndef __SIGNAL_SOURCE_SINE_WAVE_H__
#define __SIGNAL_SOURCE_SINE_WAVE_H__


#include <avr/pgmspace.h>


extern const uint8_t SINE_TABLE[] PROGMEM;


/*
 * Caller gets a stepper (just one)
 * Calibrates it whenever frequency changes
 * Does ++obj when moving to the next
 * Uses obj as a parameter to get next value
 *
 */


class SignalSourceSineWave
{
    static const uint16_t SINE_TABLE_LEN = 512;
    
    class StepperTypeDouble
    {
        friend class SignalSourceSineWave;
        
        using ThisClass = StepperTypeDouble;
        
    public:
        StepperTypeDouble()
        {
            // Nothing to do
        }
        
        StepperTypeDouble(uint16_t sampleRate, uint16_t frequency)
        {
            Calibrate(sampleRate, frequency);
        }
        
        void ResetIdx()
        {
            idx_ = 0.0;
        }
        
        void Calibrate(uint16_t sampleRate, uint16_t frequency)
        {
            // only change stepSize_, leave idx_ so that this (re)calibration
            // can be run whenever.
            
            stepSize_ =
                (double)SINE_TABLE_LEN / ((double)sampleRate / (double)frequency);
                
            Serial.print("    stepSize: ");  Serial.println(stepSize_);
            Serial.print("    idx: ");  Serial.println(idx_);
        }
    
        inline ThisClass &operator++()
        {
            idx_ += stepSize_;
            
            //Serial.print("    idx: ");  Serial.println(idx_);
            
            if (idx_ > SINE_TABLE_LEN)
            {
                idx_ -= SINE_TABLE_LEN;
                
                //Serial.print("    idx: ");  Serial.print(idx_);
                //Serial.println(" (corrected)");
            }
            
            return *this;
        }
        
    private:
    
        inline operator uint16_t() const
        {
            return (uint16_t)idx_;
        }
        
        double idx_      = 0.0;
        double stepSize_ = 0.0;
    };
    
    
    
    
    class Q1616
    {
        static const uint8_t BITS_WHOLE = 16;
        static const uint8_t BITS_FRAC  = 16;
        
    public:
        Q1616(double val)
        {
            operator=(val);
        }
        
        inline Q1616 &operator=(const double& rhs)
        {
            double dTmp = 0.0;
            
            uint16_t whole = (uint16_t)rhs;
            dTmp = rhs - whole;
            
            uint16_t frac = (uint16_t)round(dTmp * ((uint32_t)2 << BITS_FRAC));
            
            // whole + round(frac * 2^BITS_FRAC)
            val_ = (((uint32_t)whole << BITS_WHOLE) | frac);
            
            return *this;
        }
        
        inline bool operator<(const uint16_t &rhs)
        {
            return val_ < ((uint32_t)rhs << BITS_WHOLE);
        }
        
        inline Q1616 &operator=(const uint16_t& rhs)
        {
            val_ = ((uint32_t)rhs << BITS_WHOLE);
            
            return *this;
        }
        
        inline Q1616 operator+=(const Q1616 &rhs)
        {
            val_ += rhs;
            
            return *this;
        }
        
        inline Q1616 operator-=(const uint16_t &rhs)
        {
            uint32_t qTmp = ((uint32_t)rhs << BITS_WHOLE);
            
            val_ -= qTmp;
            
            return *this;
        }
        
        inline operator uint16_t() const
        {
            return (uint16_t)(val_ >> BITS_WHOLE);
        }
    
    private:
        uint32_t val_ = 0;
    };
    
    
    
    
    
    class StepperTypeFixedPointDouble
    {
        friend class SignalSourceSineWave;
        
        using ThisClass = StepperTypeFixedPointDouble;
        
    public:
        StepperTypeFixedPointDouble()
        {
            // Nothing to do
        }
        
        StepperTypeFixedPointDouble(uint16_t sampleRate, uint16_t frequency)
        {
            Calibrate(sampleRate, frequency);
        }
        
        void ResetIdx()
        {
            idx_ = 0.0;
        }
        
        void Calibrate(uint16_t sampleRate, uint16_t frequency)
        {
            // only change stepSize_, leave idx_ so that this (re)calibration
            // can be run whenever.
            
            stepSize_ =
                (double)SINE_TABLE_LEN / ((double)sampleRate / (double)frequency);
                
            Serial.print("    stepSize: ");  Serial.println(stepSize_);
            Serial.print("    idx: ");  Serial.println(idx_);
        }
    
        inline ThisClass &operator++()
        {
            idx_ += stepSize_;
            
            //Serial.print("    idx: ");  Serial.println(idx_);
            
            if (idx_ > SINE_TABLE_LEN)
            {
                idx_ -= SINE_TABLE_LEN;
                
                //Serial.print("    idx: ");  Serial.print(idx_);
                //Serial.println(" (corrected)");
            }
            
            return *this;
        }
        
    private:
    
        inline operator uint16_t() const
        {
            return (uint16_t)idx_;
        }
        
        Q1616 idx_      = 0.0;
        Q1616 stepSize_ = 0.0;
    };
    
public:
    SignalSourceSineWave()
    {
        // Nothing to do
    }
    
    //using IdxType = StepperTypeDouble;
    using IdxType = StepperTypeFixedPointDouble;
    
    static inline uint8_t GetSampleAtIdx(IdxType idx)
    {
        uint16_t sineTableIdx = (uint16_t)idx;
        
        uint16_t pgmByteLocation = (uint16_t)SINE_TABLE + sineTableIdx;

        uint8_t val = pgm_read_byte_near(pgmByteLocation);
        
        return val;
    }
    
    
private:
};


#endif  // __SIGNAL_SOURCE_SINE_WAVE_H__













