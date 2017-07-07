#ifndef __FIXED_POINT_H__
#define __FIXED_POINT_H__


#include <math.h>
#include <stdint.h>


template <uint8_t BITS_WHOLE,
         uint8_t BITS_FRAC,
         typename STORAGE_TYPE,
         typename STORAGE_TYPE_HALF_SIZE>
class FixedPoint
{
    using FixedPointClass = FixedPoint<BITS_WHOLE,
                                       BITS_FRAC,
                                       STORAGE_TYPE,
                                       STORAGE_TYPE_HALF_SIZE>;
    
public:

    
    ////////////////////////////////////////////////////////////////////
    //
    // FixedPointClass
    //
    ////////////////////////////////////////////////////////////////////

    inline void operator+=(const FixedPointClass &rhs)
    {
        val_ += rhs.val_;
    }
    
    inline void operator-=(const FixedPointClass &rhs)
    {
        val_ -= rhs.val_;
    }
    
    inline bool operator>(const FixedPointClass &rhs) const
    {
        return val_ > rhs.val_;
    }
    
    
    ////////////////////////////////////////////////////////////////////
    //
    // double
    //
    ////////////////////////////////////////////////////////////////////

    inline explicit FixedPoint(const double val)
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
    
    inline operator uint16_t() const
    {
        return (uint16_t)(val_ >> BITS_WHOLE);
    }
    
    
    ////////////////////////////////////////////////////////////////////
    //
    // uint8_t
    //
    ////////////////////////////////////////////////////////////////////
    
    inline bool operator>(const uint8_t rhs) const
    {
        return val_ > ((STORAGE_TYPE)rhs << BITS_WHOLE);
    }

    inline bool operator<(const uint8_t rhs) const
    {
        return val_ < ((STORAGE_TYPE)rhs << BITS_WHOLE);
    }

    inline operator uint8_t() const
    {
        return (uint8_t)(val_ >> BITS_WHOLE);
    }
    
    
private:

    STORAGE_TYPE val_ = 0;
};



using Q1616 = FixedPoint<16, 16, uint32_t, uint16_t>;
using Q88   = FixedPoint<8, 8, uint16_t, uint8_t>;





#endif  // __FIXED_POINT_H__









