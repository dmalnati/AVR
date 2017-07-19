#ifndef __CFG_ITEM_H__
#define __CFG_ITEM_H__


#include "TypeTraits.h"


class CfgItem
{
public:
    CfgItem()
    : u16(0)
    {
        // Nothing to do
    }

    template <typename T>
    CfgItem(uint8_t typeInput, T valInput)
    {
        static_assert(is_same<T, uint16_t>::value ||
                      is_same<T,  int16_t>::value ||
                      is_same<T,  uint8_t>::value ||
                      is_same<T,   int8_t>::value,
                      "Use only supported types");

        type = typeInput;

        if (is_same<T, uint16_t>::value)
        {
            u16 = valInput;
        }
        else if (is_same<T, int16_t>::value)
        {
            i16 = valInput;
        }
        else if (is_same<T, uint8_t>::value)
        {
            u8 = valInput;
        }
        else if (is_same<T, int8_t>::value)
        {
            i8 = valInput;
        }
    }

    explicit inline operator uint16_t() const { return u16; }
    explicit inline operator  int16_t() const { return i16; }
    explicit inline operator  uint8_t() const { return  u8; }
    explicit inline operator   int8_t() const { return  i8; }


    uint8_t type = 0;

private:

    union
    {

        uint16_t u16;
        int16_t  i16;
        uint8_t   u8;
        int8_t    i8;

    };

};



#endif  // __CFG_ITEM_H__