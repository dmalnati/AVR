#ifndef __UTL_STREAM_BLOB_H__
#define __UTL_STREAM_BLOB_H__


#include <inttypes.h>

//#include "Arduino.h"


extern void StreamBlob(uint8_t  *buf,
                       uint16_t  bufSize,
                       uint8_t   showBin,
                       uint8_t   showHex);


#endif  // __UTL_STREAM_BLOB_H__