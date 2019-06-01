#ifndef __UTL_H__
#define __UTL_H__


#include <stdlib.h>


class Utl
{
public:

    static uint32_t GetRandomInRange(uint32_t rangeLow, uint32_t rangeHigh)
    {
        uint32_t retVal = 0;

        srand(PAL.Micros());

        retVal = rangeLow + (rand() % (rangeHigh - rangeLow + 1));

        return retVal;
    }

};


#endif // __UTL_H__









