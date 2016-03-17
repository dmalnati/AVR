#ifndef __SERIAL_LINK_H__
#define __SERIAL_LINK_H__


#include "IdleTimeEventHandler.h"


// Debug
#include "Utl.h"


template <typename T, uint8_t PAYLOAD_CAPACITY = 32>
class SerialLink
: public IdleTimeEventHandler
{
public:
    struct Header
    {
        uint8_t preamble;
        uint8_t dataLength;
        uint8_t checksum;
        uint8_t protocolId;
    };

private:
    typedef void (T::*OnRxAvailableCbFn)(Header  *hdr,
                                         uint8_t *buf,
                                         uint8_t  bufSize);
    
    static const uint16_t BAUD          = 9600;
    static const uint8_t  PREAMBLE_BYTE = 0x55;
    static const uint8_t  BUF_CAPACITY  = sizeof(Header) + PAYLOAD_CAPACITY;

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
        obj_       = NULL;
        rxCb_      = NULL;
        bufRxSize_ = 0;
        state_     = State::LOOKING_FOR_PREAMBLE_BYTE;
        
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
    
    uint8_t Send(uint8_t protocolId, uint8_t *buf, uint8_t bufSize)
    {
        uint8_t retVal = 0;
        
        // First check to see if it can all fit
        if (bufSize <= PAYLOAD_CAPACITY)
        {
            // Fill out header
            Header *hdr = (Header *)bufTx_;
            
            hdr->preamble   = PREAMBLE_BYTE;
            hdr->dataLength = bufSize;
            hdr->checksum   = 0;
            hdr->protocolId = protocolId;
        
            // Copy in user data
            memcpy(&(bufTx_[sizeof(Header)]), buf, bufSize);
            
            // Calculate checksum
            uint8_t checksum = CRC8(bufTx_, sizeof(Header) + bufSize);
            
            // Store checksum in header
            hdr->checksum = checksum;
            
            // Send via Serial connection
            uint8_t lenWritten = Serial.write(bufTx_, sizeof(Header) + bufSize);
            
            // Success if data fully sent
            retVal = (lenWritten == (sizeof(Header) + bufSize));
        }
        
        return retVal;
    }
    

private:

    // Code from:
    // http://www.evilgeniuslair.com/2015/01/14/crc-8/
    //CRC-8 - based on the CRC8 formulas by Dallas/Maxim
    //code released under the therms of the GNU GPL 3.0 license
    byte CRC8(const byte *data, byte len) {
        byte crc = 0x00;
        while (len--) {
            byte extract = *data++;
            for (byte tempI = 8; tempI; tempI--) {
                byte sum = (crc ^ extract) & 0x01;
                crc >>= 1;
                if (sum) {
                    crc ^= 0x8C;
                }
                extract >>= 1;
            }
        }
        return crc;
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
                bufRx_[0]  = firstByte;
                bufRxSize_ = 1;
                
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
        
        while (Serial.available() && bufRxSize_ != BUF_CAPACITY)
        {
            uint8_t nextByte = Serial.read();
            
            bufRx_[bufRxSize_] = nextByte;
            ++bufRxSize_;
            
            ++bytesAdded;
            
            PinToggle(6, 1);
        }
        
        return bytesAdded;
    }
    
    void ClearLeadingBufferBytesAndRecalibrate(uint8_t len)
    {
        // Want to remove the given number of bytes.  Also discard any
        // subsequent bytes which aren't the PREAMBLE_BYTE.
        // Also set the state of processing as a result.
        if (len <= bufRxSize_)
        {
            uint8_t *remainingData    = &(bufRx_[len]);
            uint8_t  remainingDataLen = bufRxSize_ - len;
            
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
                    bufRx_[i] = dataToShift[i];
                }
                
                // Set the buffer size and state
                bufRxSize_ = dataToShiftLen;
                state_     = State::LOOKING_FOR_END_OF_MESSAGE;
            }
            else
            {
                // Couldn't find it.  Zero out the buffer.
                bufRxSize_ = 0;
                state_     = State::LOOKING_FOR_PREAMBLE_BYTE;
            }
        }
    }
    
    uint8_t TryToProcessMessage()
    {
        uint8_t retVal = 0;
        
        // Check if there is enough data to examine the header
        if (bufRxSize_ >= sizeof(Header))
        {
            // Yes, check the message self-declared data length
            Header *hdr = (Header *)bufRx_;
            
            // Check if we have that much
            if (bufRxSize_ >= sizeof(Header) + hdr->dataLength)
            {
                // We do, confirm checksum before passing up.
                
                // First, take a copy of the message checksum so that you can
                // restore it after the checksum calculation, which demands that
                // the checksum field be zero.
                uint8_t checksumTmp = hdr->checksum;
                
                // Zero out the message checksum
                hdr->checksum = 0;
                
                // Calculate checksum of the message
                uint8_t checksum = CRC8(bufRx_, sizeof(Header) + hdr->dataLength);
                
                // Restore the message checksum
                hdr->checksum = checksumTmp;
                
                // Validate whether checksums match
                if (1 || checksum == hdr->checksum)
                {
                    retVal = 1;
                    
                    // Call back with complete message
                    ((*obj_).*rxCb_)(hdr,
                                     &(bufRx_[sizeof(Header)]),
                                     hdr->dataLength);
                }
                else
                {
                    // Error, destroy data and move on.
                    // This is required anyway, so do nothing here.
                }
                
                // Destroy data and move on.
                ClearLeadingBufferBytesAndRecalibrate(sizeof(Header) +
                                                      hdr->dataLength);
            }
            
            return retVal;
        }
        
        // Try to add more data if there is space to do so
        if (bufRxSize_ != BUF_CAPACITY)
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
            PinToggle(4, 10);
            TryToSyncStream();
        }
        
        if (state_ == State::LOOKING_FOR_END_OF_MESSAGE)
        {
            //PinToggle(4, 20);
            TryToProcessMessage();
        }
    }
    
    
    // Callback members
    T                 *obj_;
    OnRxAvailableCbFn  rxCb_;
    
    // RX Buffer members
    uint8_t bufRx_[BUF_CAPACITY];
    uint8_t bufRxSize_;
    State   state_;
    
    // TX Buffer members
    uint8_t bufTx_[BUF_CAPACITY];
};




#endif  // __SERIAL_LINK_H__







