#ifndef __UTL_STREAM_BLOB_H__
#define __UTL_STREAM_BLOB_H__


#include <inttypes.h>

#include "Arduino.h"


extern void StreamBlob(Print    &stream,
                       uint8_t  *buf,
                       uint16_t  bufSize,
                       uint8_t   showBin = 0,
                       uint8_t   showHex = 1);


#endif  // __UTL_STREAM_BLOB_H__