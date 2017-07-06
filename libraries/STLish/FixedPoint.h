#ifndef __FIXED_POINT_H__
#define __FIXED_POINT_H__


#include <math.h>
#include <stdint.h>


class Q1616
{
    static const uint8_t BITS_WHOLE = 16;
    static const uint8_t BITS_FRAC  = 16;
    
public:

    
    ////////////////////////////////////////////////////////////////////
    //
    // Q1616
    //
    ////////////////////////////////////////////////////////////////////

    inline void operator+=(const Q1616 &rhs)
    {
        val_ += rhs.val_;
    }
    
    inline void operator-=(const Q1616 &rhs)
    {
        val_ -= rhs.val_;
    }
    
    inline bool operator>(const Q1616 &rhs) const
    {
        return val_ > rhs.val_;
    }
    
    
    ////////////////////////////////////////////////////////////////////
    //
    // double
    //
    ////////////////////////////////////////////////////////////////////

    inline explicit Q1616(const double val)
    {
        operator=(val);
    }
    
    inline void operator=(const double &rhs)
    {
        uint16_t whole = (uint16_t)rhs;
        double   frac  = rhs - whole;
        
        uint16_t fracAsInt = (uint16_t)round(frac * ((uint32_t)1 << BITS_FRAC));
        
        // val = whole + round(frac * 2^BITS_FRAC)
        val_ = (((uint32_t)whole << BITS_WHOLE) | fracAsInt);
    }
    
    
    ////////////////////////////////////////////////////////////////////
    //
    // uint16_t
    //
    ////////////////////////////////////////////////////////////////////

    inline void operator=(const uint16_t rhs)
    {
        val_ = ((uint32_t)rhs << BITS_WHOLE);
    }
    
    inline void operator-=(const uint16_t rhs)
    {
        uint32_t tmp = ((uint32_t)rhs << BITS_WHOLE);
        
        val_ -= tmp;
    }
    
    inline bool operator>(const uint16_t rhs) const
    {
        return val_ > ((uint32_t)rhs << BITS_WHOLE);
    }
    
    inline bool operator<(const uint16_t rhs) const
    {
        return val_ < ((uint32_t)rhs << BITS_WHOLE);
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
        return val_ > ((uint32_t)rhs << BITS_WHOLE);
    }

    inline bool operator<(const uint8_t rhs) const
    {
        return val_ < ((uint32_t)rhs << BITS_WHOLE);
    }

    inline operator uint8_t() const
    {
        return (uint8_t)(val_ >> BITS_WHOLE);
    }
    
    
private:

    uint32_t val_ = 0;
};


#endif  // __FIXED_POINT_H__









