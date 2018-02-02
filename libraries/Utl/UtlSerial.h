#ifndef __UTL_SERIAL_H__
#define __UTL_SERIAL_H__


uint8_t SerialReadLine(char *buf, uint8_t bufSize)
{
    uint8_t retVal = 0;
 
    while (!retVal)
    {
        retVal = Serial.readBytesUntil('\n', buf, bufSize);

        if (retVal)
        {
            buf[retVal] = '\0';
        }
    }
    
    return retVal;
}


#endif  // __UTL_SERIAL_H__



