#ifndef __UTL_SERIAL_H__
#define __UTL_SERIAL_H__


uint8_t SerialReadLine(char *buf, uint8_t bufSize)
{
    uint8_t retVal = 0;
 
    while (!retVal)
    {
        retVal = Serial.readBytesUntil('\n', buf, bufSize);

        if (retVal < bufSize)
        {
            buf[retVal] = '\0';
        }
        else
        {
            buf[bufSize - 1] = '\0';
            
            // newline wasn't hit, and bytes weren't discarded, so consume them
            uint8_t cont = 1;
            while (cont)
            {
                if (Serial.available())
                {
                    char c = Serial.read();
                    
                    if (c == '\n')
                    {
                        cont = 0;
                    }
                }
            }
        }
    }
    
    return retVal;
}


#endif  // __UTL_SERIAL_H__



