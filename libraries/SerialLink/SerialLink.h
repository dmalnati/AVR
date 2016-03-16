#ifndef __SERIAL_LINK_H__
#define __SERIAL_LINK_H__


#include "IdleTimeEventHandler.h"


template <typename T, uint8_t DATA_LENGTH = 32>
class SerialLink
: public IdleTimeEventHandler
{
private:
    typedef void (T::*OnRxAvailableCbFn)(uint8_t *buf, uint8_t bufSize);
    
    static const uint16_t BAUD              = 9600;
    static const uint8_t  PREAMBLE_BYTE     = 0x55;
    static const uint8_t  PROTOCOL_OVERHEAD = 3;
    static const uint8_t  BUF_CAPACITY      = DATA_LENGTH + PROTOCOL_OVERHEAD;

    enum class State : uint8_t
    {
        LOOKING_FOR_PREAMBLE_BYTE,
        LOOKING_FOR_END_OF_MESSAGE
    };
    
public:
    SerialLink()  { Reset(); }
    ~SerialLink() { Reset(); }
    
    void Reset()
    {
        obj_     = NULL;
        rxCb_    = NULL;
        bufSize_ = 0;
        state_   = State::LOOKING_FOR_PREAMBLE_BYTE;
        
        Serial.end();
        
        DeRegisterForIdleTimeEvent();
    }
    
    uint8_t Init(T *obj, OnRxAvailableCbFn rxCb)
    {
        uint8_t retVal = 0;

        Reset();
        
        if (obj && rxCb)
        {
            retVal = 1;
            
            obj_  = obj;
            rxCb_ = rxCb;
            
            Serial.begin(BAUD);
            
            RegisterForIdleTimeEvent();
        }
        
        return retVal;
    }
    
    uint8_t Send(uint8_t *buf, uint8_t len)
    {
        uint8_t lenWritten = Serial.write(buf, len);
        
        return (len == lenWritten);
    }
    

private:

    // Lightly modified code from:
    // https://chromium.googlesource.com/chromiumos/platform/vboot_reference/+/master/firmware/lib/crc8.c
    /**
     * Return CRC-8 of the data, using x^8 + x^2 + x + 1 polynomial.  A table-based
     * algorithm would be faster, but for only a few bytes it isn't worth the code
     * size. */
    //uint8_t Crc8(const void *vptr, int len)
    uint8_t Crc8(const uint8_t *data, uint8_t len)
    {
        //const uint8_t *data = vptr;
        //unsigned crc = 0;
        //int i, j;
        uint16_t crc = 0;
        uint8_t i, j;
        for (j = len; j; j--, data++) {
            crc ^= (*data << 8);
            for(i = 8; i; i--) {
                if (crc & 0x8000)
                    crc ^= (0x1070 << 3);
                crc <<= 1;
            }
        }
        return (uint8_t)(crc >> 8);
    }

    uint8_t TryToSyncStream()
    {
        uint8_t retVal = 0;
        
        uint8_t cont = Serial.available();
        while (cont)
        {
            uint8_t firstByte = Serial.read();
            
            if (firstByte == PREAMBLE_BYTE)
            {
                retVal = 1;
                cont = 0;
                
                // store data
                buf_[0] = firstByte;
                bufSize_ = 1;
                
                // change state to now look for rest of message
                state_ = State::LOOKING_FOR_END_OF_MESSAGE;
            }

            if (cont)
            {
                cont = Serial.available();
            }
        }
        
        return retVal;
    }
    
    uint8_t TryToAddMoreData()
    {
        uint8_t bytesAdded = 0;
        
        while (Serial.available() && bufSize_ != BUF_CAPACITY)
        {
            uint8_t nextByte = Serial.read();
            
            buf_[bufSize_] = nextByte;
            ++bufSize_;
            
            ++bytesAdded;
        }
        
        return bytesAdded;
    }
    
    void ClearLeadingBufferBytesAndRecalibrate(uint8_t len)
    {
        // Want to remove the given number of bytes.  Also discard any
        // subsequent bytes which aren't the PREAMBLE_BYTE.
        // Also set the state of processing as a result.
        if (len <= bufSize_)
        {
            uint8_t *remainingData = &(buf_[len]);
            uint8_t  remainingDataLen = bufSize_ - len;
            
            // Search for preamble byte
            uint8_t found       = 0;
            uint8_t idxPreamble = 0;
            for (uint8_t i = 0; i < remainingDataLen && !found; ++i)
            {
                if (remainingData[i] == PREAMBLE_BYTE)
                {
                    found       = 1;
                    idxPreamble = i;
                    break;
                }
            }
            
            // If preamble byte found, shift it and all subsequent bytes to the
            // front of the buffer.
            if (found)
            {
                // Calculate where the data is and its size
                uint8_t *dataToShift    = &(remainingData[idxPreamble]);
                uint8_t  dataToShiftLen = remainingDataLen - idxPreamble;
                
                // Shift it
                for (uint8_t i = 0; i < dataToShiftLen; ++i)
                {
                    buf_[i] = dataToShift[i];
                }
                
                // Set the buffer size and state
                bufSize_ = dataToShiftLen;
                state_   = State::LOOKING_FOR_END_OF_MESSAGE;
            }
            else
            {
                // Couldn't find it.  Zero out the buffer.
                bufSize_ = 0;
                state_   = State::LOOKING_FOR_PREAMBLE_BYTE;
            }
        }
    }
    
    uint8_t TryToProcessMessage()
    {
        uint8_t retVal = 0;
        
        // Check if there is enough data to know the message length
        if (bufSize_ >= 2)
        {
            // Yes, check the message self-declared data length
            uint8_t msgDataLen = buf_[1];
            
            // Check if we have that much
            if (bufSize_ >= msgDataLen + PROTOCOL_OVERHEAD)
            {
                // We do, confirm checksum before passing up.
                uint8_t msgChecksum = buf_[2 + msgDataLen];
                
                // Calculate a checksum of everything but the msgChecksum
                uint8_t checksum = Crc8(buf_,
                                        msgDataLen + (PROTOCOL_OVERHEAD - 1));
                
                if (checksum == msgChecksum)
                {
                    retVal = 1;
                    
                    // Call back with complete message
                    ((*obj_).*rxCb_)(&(buf_[2]), msgDataLen);
                }
                else
                {
                    // Error, destroy data and move on.
                    // This is required anyway, so do nothing here.
                }
                
                // Destroy data and move on.
                ClearLeadingBufferBytesAndRecalibrate(msgDataLen + PROTOCOL_OVERHEAD);
            }
            
            return retVal;
        }
        
        // Try to add more data if there is space to do so
        if (bufSize_ != BUF_CAPACITY)
        {
            TryToAddMoreData();
        }
        else
        {
            // Error -- should have found a message if the buffer is filled.
            // Possibly corruption mangled the msgDataLen value.
            
            // Destroy existing preamble byte, which should be at the start
            // of the buffer, which will also trigger a re-scan for the next
            // preamble byte.
            ClearLeadingBufferBytesAndRecalibrate(1);
        }
        
        return retVal;
    }

    // Implement IdleTimeEventHandler callback
    virtual void OnIdleTimeEvent()
    {
        // Look for the first byte to synchronize stream
        if (state_ == State::LOOKING_FOR_PREAMBLE_BYTE)
        {
            TryToSyncStream();
        }
        
        if (state_ == State::LOOKING_FOR_END_OF_MESSAGE)
        {
            TryToProcessMessage();
        }
    }
    
    

    T                 *obj_;
    OnRxAvailableCbFn  rxCb_;
    
    uint8_t buf_[BUF_CAPACITY];
    uint8_t bufSize_;
    
    State state_;
};




#endif  // __SERIAL_LINK_H__







