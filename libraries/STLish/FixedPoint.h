#ifndef __FIXED_POINT_H__
#define __FIXED_POINT_H__


#include <math.h>
#include <stdint.h>


class Q08;


template <uint8_t  BITS_WHOLE,
          uint8_t  BITS_FRAC,
          typename STORAGE_TYPE,
          typename STORAGE_TYPE_HALF_SIZE>
class FixedPoint
{
    using FixedPointClass = FixedPoint<BITS_WHOLE,
                                       BITS_FRAC,
                                       STORAGE_TYPE,
                                       STORAGE_TYPE_HALF_SIZE>;
    
    
    ////////////////////////////////////////////////////////////////////
    //
    // FixedPointClass
    //
    ////////////////////////////////////////////////////////////////////

private:

    enum class CTorType : uint8_t
    {
        PRIVATE = 0
    };
    
    // For lightweight construction of temporary objects, private only
    inline FixedPoint(STORAGE_TYPE val, CTorType)
    : val_(val)
    {
        // Nothing to do
    }

    
public:

    // Moving member instantiation to explicit constructor since more ctors are
    // being added and better to be explicit, especially for debugging.
    inline FixedPoint()
    : val_(0)
    {
        // Nothing to do
    }
    
    // For copy elision, not actually part of a particular use case
    inline FixedPoint(const FixedPointClass &val)
    {
        operator=(val);
    }

    // Useful for assignment after subtraction in limit mode stepping
    inline void operator=(const FixedPointClass &rhs)
    {
        val_ = rhs.val_;
    }
    
    inline void operator+=(const FixedPointClass &rhs)
    {
        val_ += rhs.val_;
    }
    
    inline void operator-=(const FixedPointClass &rhs)
    {
        val_ -= rhs.val_;
    }
    
    inline FixedPointClass operator+(const FixedPointClass &rhs) const
    {
        STORAGE_TYPE retVal = val_ + rhs.val_;
        
        return FixedPointClass(retVal, CTorType::PRIVATE);
    }
    
    inline FixedPointClass operator-(const FixedPointClass &rhs) const
    {
        STORAGE_TYPE retVal = val_ - rhs.val_;
        
        return FixedPointClass(retVal, CTorType::PRIVATE);
    }
    
    inline bool operator>(const FixedPointClass &rhs) const
    {
        return val_ > rhs.val_;
    }
    
    inline bool operator<(const FixedPointClass &rhs) const
    {
        return val_ < rhs.val_;
    }
    
    
    ////////////////////////////////////////////////////////////////////
    //
    // Q08
    //
    ////////////////////////////////////////////////////////////////////
    
    inline FixedPointClass operator*(const Q08 &rhs) const
    {
        STORAGE_TYPE retVal = val_ * rhs;
        
        return FixedPointClass(retVal, CTorType::PRIVATE);
    }
    
    
    ////////////////////////////////////////////////////////////////////
    //
    // double
    //
    ////////////////////////////////////////////////////////////////////

    inline FixedPoint(const double val)
    {
        operator=(val);
    }
    
    inline void operator=(const double &rhs)
    {
        STORAGE_TYPE_HALF_SIZE whole = (STORAGE_TYPE_HALF_SIZE)rhs;
        double                 frac  = rhs - whole;
        
        STORAGE_TYPE_HALF_SIZE fracAsInt =
            (STORAGE_TYPE_HALF_SIZE)round(frac * ((STORAGE_TYPE)1 << BITS_FRAC));
        
        // val = whole + round(frac * 2^BITS_FRAC)
        val_ = (((STORAGE_TYPE)whole << BITS_WHOLE) | fracAsInt);
    }
    
    
    ////////////////////////////////////////////////////////////////////
    //
    // uint16_t
    //
    ////////////////////////////////////////////////////////////////////

    inline void operator=(const uint16_t rhs)
    {
        val_ = ((STORAGE_TYPE)rhs << BITS_WHOLE);
    }
    
    inline void operator-=(const uint16_t rhs)
    {
        STORAGE_TYPE tmp = ((STORAGE_TYPE)rhs << BITS_WHOLE);
        
        val_ -= tmp;
    }
    
    inline bool operator>(const uint16_t rhs) const
    {
        return val_ > ((STORAGE_TYPE)rhs << BITS_WHOLE);
    }
    
    inline bool operator<(const uint16_t rhs) const
    {
        return val_ < ((STORAGE_TYPE)rhs << BITS_WHOLE);
    }
    
    explicit inline operator uint16_t() const
    {
        return (uint16_t)(val_ >> BITS_WHOLE);
    }

    
    
    ////////////////////////////////////////////////////////////////////
    //
    // uint8_t
    //
    ////////////////////////////////////////////////////////////////////
    
    inline FixedPoint(const uint8_t val)
    {
        val_ = ((STORAGE_TYPE)val << BITS_WHOLE);
    }
    
    inline bool operator>(const uint8_t rhs) const
    {
        return val_ > ((STORAGE_TYPE)rhs << BITS_WHOLE);
    }

    inline bool operator<(const uint8_t rhs) const
    {
        return val_ < ((STORAGE_TYPE)rhs << BITS_WHOLE);
    }

    explicit inline operator uint8_t() const
    {
        return (uint8_t)(val_ >> BITS_WHOLE);
    }
        
private:

    STORAGE_TYPE val_;
};



using Q1616 = FixedPoint<16, 16, uint32_t, uint16_t>;
using Q88   = FixedPoint<8, 8, uint16_t, uint8_t>;


/*
 * Note that this class specifically avoids casting multiplication operation
 * return values to something consistent with what was multiplied against.
 *
 * The natural effect of a multiply leads to an intermediate value which is
 * an 'int', which in AVR is int16_t.  It doesn't matter whether the multiply
 * is signed or unsigned.
 *
 * Avoiding the cast back from the int16_t intermediate state is a massive
 * performance optimization.
 *
 * In testing, in a fairly simple case, the duration of time between two
 * operations went from ~24.5us to ~18.8us, which matches the performance
 * when using primitives.
 *
 */
class Q08
{
    static const uint8_t BITS_WHOLE = 0;
    static const uint8_t BITS_FRAC  = 8;

public:

    ////////////////////////////////////////////////////////////////////
    //
    // Q08
    //
    ////////////////////////////////////////////////////////////////////

    inline Q08()
    {
        // Nothing to do
    }

    inline Q08(const Q08 &val)
    {
        operator=(val);
    }

    inline void operator=(const Q08 &rhs)
    {
        val_ = rhs.val_;
    }
    
    inline Q08 operator*(const Q08 &rhs) const
    {
        return (uint8_t)(val_ * rhs.val_ / 256);
    }
    
    ////////////////////////////////////////////////////////////////////
    //
    // FixedPoint
    //
    ////////////////////////////////////////////////////////////////////
    
    // Use deduction to make sure we're only referring to FixedPoint types here
    template <uint8_t BITS_WHOLE, uint8_t BITS_FRAC, typename STORAGE_TYPE, typename STORAGE_TYPE_HALF_SIZE>
    inline FixedPoint<BITS_WHOLE,BITS_FRAC,STORAGE_TYPE,STORAGE_TYPE_HALF_SIZE>
    operator*(const FixedPoint<BITS_WHOLE,BITS_FRAC,STORAGE_TYPE,STORAGE_TYPE_HALF_SIZE> &rhs) const
    {
        return rhs * *this;
    }


    ////////////////////////////////////////////////////////////////////
    //
    // double
    //
    ////////////////////////////////////////////////////////////////////

    inline Q08(const double val)
    {
        operator=(val);
    }

    inline void operator=(const double rhs)
    {
        // convert to range 0-255
        double frac = rhs - trunc(rhs);

        uint8_t fracAsInt =
            (uint8_t)round(frac * ((uint8_t)1 << BITS_FRAC));

        val_ = fracAsInt;
    }
    
    
    ////////////////////////////////////////////////////////////////////
    //
    // uint16_t
    //
    ////////////////////////////////////////////////////////////////////
    
    inline int32_t operator*(const uint16_t rhs) const
    {
        return ((int32_t)val_ * (int32_t)rhs) / 256;
    }
    

    ////////////////////////////////////////////////////////////////////
    //
    // uint8_t
    //
    ////////////////////////////////////////////////////////////////////

    inline Q08(const uint8_t val)
    {
        operator=(val);
    }

    inline void operator=(const uint8_t rhs)
    {
        val_ = rhs;
    }

    inline int16_t operator*(const uint8_t rhs) const
    {
        return val_ * rhs / 256;
    }
    
    explicit inline operator uint8_t() const
    {
        return val_;
    }


    ////////////////////////////////////////////////////////////////////
    //
    // int8_t
    //
    ////////////////////////////////////////////////////////////////////

    inline Q08(const int8_t val)
    {
        operator=(val);
    }

    inline void operator=(const int8_t rhs)
    {
        val_ = rhs;
    }
    
    inline int16_t operator*(const int8_t rhs) const
    {
        return val_ * rhs / 256;
    }


private:

    uint8_t val_ = 0;
};


inline int16_t operator*(const uint16_t lhs, const Q08 &rhs)
{
    return rhs * lhs;
}

inline int16_t operator*(const uint8_t lhs, const Q08 &rhs)
{
    return rhs * lhs;
}

inline int16_t operator*(const int8_t lhs, const Q08 &rhs)
{
    return rhs * lhs;
}








#endif  // __FIXED_POINT_H__









