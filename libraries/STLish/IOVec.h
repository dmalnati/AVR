#ifndef __IO_VECTOR_H__
#define __IO_VECTOR_H__


#include <stdint.h>


struct IOVec
{
    uint8_t *buf    = nullptr;
    uint8_t  bufLen = 0;
};


#endif  // __IO_VECTOR_H__