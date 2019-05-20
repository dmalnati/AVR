#ifndef __SERIAL_H__
#define __SERIAL_H__


#include <math.h>
#include <string.h>
#include <util/atomic.h>

#include "Container.h"
#include "StreamWindow.h"


class Serial0
{
public:
    
    void Start(uint32_t baud);
    void Stop();
    
    void Write(uint8_t b)
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            // Synchronous.
            // Don't allow function to return until byte fully transmitted.
            //
            // No need to check if data is already in the buffer, this is the only
            // function which can put data there, and we're ensuring it's gone by
            // the time we return.
            //
            // The buffer, UDR0, is not the buffer the USART uses to transmit from.
            // The USART uses a shift register, which loads from the UDR0 when it is
            // ready to send a new byte.
            // 
            // So waiting until the UDR0 is empty means that the shift register has
            // taken a copy of its value and is sending it (or has sent it).
            //
            // So in terms of streamlining transmissions, this should be just fine
            // waiting for synchronous write, as the next byte can be loaded into
            // UDR0 while the shift register is working the prior.
            
            
            // Clear the TXCn flag by writing 1 to it.
            // The last two bits are the only other writeable bits, and are
            // appropriate to set to 0.
            //
            // This supports the ability to wait for final data to be shifted out of
            // register if Stop is called before complete.
            UCSR0A = 0b01000000;
            
            // Put data into buffer leading to shift register
            UDR0 = b;
            
            // Wait for this data to be put into the shift register and now be empty
            while (!(UCSR0A & (1 << UDRE0)));
            
            // Keep track of state for disambiguation later
            hasWrittenAtLeastOneByte_ = 1;
        }
    }
    
    void Write(uint8_t *buf, uint16_t bufSize)
    {
        for (uint16_t i = 0; i < bufSize; ++i)
        {
            Write(buf[i]);
        }
    }
    
    void Write(const char *str)
    {
        uint16_t bufSize = strlen(str);
        
        Write((uint8_t *)str, bufSize);
    }
    
    uint8_t Available() const
    {
        return rxQ_.Size();
    }
    
    uint8_t Read()
    {
        uint8_t retVal = 0;
        
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            rxQ_.Pop(retVal);
        }
        
        return retVal;
    }
    
    // This sucks, would rather not implement but have to due to legacy
    // SerialReadLine requirement.  Drop as soon as possible.
    uint8_t ReadBytesUntil(uint8_t bStop, uint8_t *buf, uint16_t bufSize)
    {
        uint8_t retVal = 0;
        
        uint8_t cont = 1;
        while (cont)
        {
            if (retVal < bufSize)
            {
                if (Available())
                {
                    uint8_t b = Read();
                    
                    if (b == bStop)
                    {
                        // we can stop now, we found our byte.
                        // we don't increment the number of bytes consumed
                        // because we're not storing this terminator.
                        cont = 0;
                    }
                    else
                    {
                        // found a non-stop byte, add to buffer and increment
                        // the number of bytes found.
                        
                        buf[retVal] = b;
                        
                        ++retVal;
                    }
                }
                else
                {
                    // nothing to do really, just keep waiting
                }
            }
            else
            {
                // buffer filled
                cont = 0;
            }
        }
        
        return retVal;
    }

private:

    static uint8_t hasWrittenAtLeastOneByte_;
    
public:

    // RX Q needs to be public so ISR can get to it
    
    static const uint8_t BUF_RX_SIZE = 32;
    static Queue<uint8_t, BUF_RX_SIZE> rxQ_;
};


extern Serial0 S0;



#endif  // __SERIAL_H__


























