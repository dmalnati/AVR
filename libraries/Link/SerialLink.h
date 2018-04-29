#ifndef __SERIAL_LINK_H__
#define __SERIAL_LINK_H__


#include "Serial.h"
#include "Function.h"
#include "TimedEventHandler.h"
#include "crc.h"


/*
 * Not protected:
 * - bad bytes lead a good message.
 *  - a preamble byte is first
 *  - a large dataLength byte is next
 *  - a legitimate message, smaller than the bad dataLength byte prior, follows
 *
 * That can be overcome by timing out unfinished messages and clearing the
 * leading preamble byte.  Too complex for now, especially given the very low
 * likelihood of this particular failure.
 *
 */
 
struct SerialLinkHeader
{
    uint8_t preamble;
    uint8_t dataLength;
    uint8_t checksum;
    uint8_t protocolId;
};
 

class SerialLink
: private TimedEventHandler
{
    static const uint8_t PAYLOAD_CAPACITY = 32;
    
public:
    static const uint8_t C_IDLE  = 0;
    static const uint8_t C_TIMED = 1;
    static const uint8_t C_INTER = 0;

private:
    using CbFunction = function<void(SerialLinkHeader  *hdr,
                                         uint8_t       *buf,
                                         uint8_t        bufSize)>;
    
    static const uint8_t  POLL_PERIOD_MS = 10;
    static const uint16_t BAUD           = 9600;
    static const uint8_t  PREAMBLE_BYTE  = 0x55;
    static const uint8_t  BUF_CAPACITY   = sizeof(SerialLinkHeader) + PAYLOAD_CAPACITY;

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
        cbFn_      = CbFunction{};
        bufRxSize_ = 0;
        state_     = State::LOOKING_FOR_PREAMBLE_BYTE;
        
        DeRegisterForTimedEvent();
    }
    
    uint8_t Init(CbFunction cbFn)
    {
        uint8_t retVal = 1;

        Reset();
        
        cbFn_ = cbFn;

        RegisterForTimedEventInterval(POLL_PERIOD_MS);
        
        return retVal;
    }
    
    uint8_t Send(uint8_t protocolId, uint8_t *buf, uint8_t bufSize)
    {
        uint8_t retVal = 0;
        
        // First check to see if it can all fit
        if (bufSize <= PAYLOAD_CAPACITY)
        {
            // Fill out header
            SerialLinkHeader *hdr = (SerialLinkHeader *)bufTx_;
            
            hdr->preamble   = PREAMBLE_BYTE;
            hdr->dataLength = bufSize;
            hdr->checksum   = 0;
            hdr->protocolId = protocolId;
        
            // Copy in user data
            memcpy(&(bufTx_[sizeof(SerialLinkHeader)]), buf, bufSize);
            
            // Calculate checksum
            uint8_t checksum = CRC8(bufTx_, sizeof(SerialLinkHeader) + bufSize);
            
            // Store checksum in header
            hdr->checksum = checksum;
            
            // Send via Serial connection
            S0.Write((bufTx_, sizeof(SerialLinkHeader) + bufSize));
            
            // Success if data fully sent
            retVal = 1;
        }
        
        return retVal;
    }
    

private:

    uint8_t TryToSyncStreamFromSerial()
    {
        uint8_t retVal = 0;
        
        uint8_t cont = S0.Available();
        while (cont)
        {
            uint8_t firstByte = S0.Read();
            
            if (firstByte == PREAMBLE_BYTE)
            {
                retVal = 1;
                cont = 0;
                
                // Store data
                bufRx_[0]  = firstByte;
                bufRxSize_ = 1;
                
                // Indicate state change
                state_ = State::LOOKING_FOR_END_OF_MESSAGE;
            }

            if (cont)
            {
                cont = S0.Available();
            }
        }
        
        return retVal;
    }
    
    uint8_t TryToAddMoreDataFromSerial()
    {
        uint8_t bytesAdded = 0;
        
        while (S0.Available() && bufRxSize_ != BUF_CAPACITY)
        {
            uint8_t nextByte = S0.Read();
            
            bufRx_[bufRxSize_] = nextByte;
            ++bufRxSize_;
            
            ++bytesAdded;
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
        else
        {
            // Should never happen.  Zero out the buffer.
            bufRxSize_ = 0;
            state_     = State::LOOKING_FOR_PREAMBLE_BYTE;
        }
    }
    
    uint8_t TryToProcessMessageFromCachedData()
    {
        uint8_t retVal = 0;
        
        // Check if there is enough data to examine the header
        if (bufRxSize_ >= sizeof(SerialLinkHeader))
        {
            // Yes, check the message self-declared data length
            SerialLinkHeader *hdr = (SerialLinkHeader *)bufRx_;
            
            // Check for mangled size or simply oversized message
            uint8_t oversized =
                (uint16_t)((uint16_t)sizeof(SerialLinkHeader) + (uint16_t)hdr->dataLength) !=
                (uint8_t) ((uint8_t) sizeof(SerialLinkHeader) + (uint8_t) hdr->dataLength);
            
            if (oversized)
            {
                // Can be mangled, oversized, or just not the start of an
                // actual message due to a prior mangle.  Either way,
                // let's skip it by resyncing after dropping the preamble byte.
                ClearLeadingBufferBytesAndRecalibrate(1);
            }
            else if (bufRxSize_ >= sizeof(SerialLinkHeader) + hdr->dataLength)
            {
                // We have enough data, confirm checksum before passing up.
                
                // Take a copy of the message checksum so that you can
                // restore it after the checksum calculation, which demands that
                // the checksum field be zero.
                uint8_t checksumTmp = hdr->checksum;
                
                // Zero out the message checksum
                hdr->checksum = 0;
                
                // Calculate checksum of the message
                uint8_t checksum = CRC8(bufRx_, sizeof(SerialLinkHeader) + hdr->dataLength);
                
                // Restore the message checksum
                hdr->checksum = checksumTmp;
                
                // Validate whether checksums match
                if (checksum == hdr->checksum)
                {
                    retVal = sizeof(SerialLinkHeader) + hdr->dataLength;

                    // Call back with complete message
                    cbFn_(hdr,
                          &(bufRx_[sizeof(SerialLinkHeader)]),
                          hdr->dataLength);

                    // Destroy data and move on.
                    ClearLeadingBufferBytesAndRecalibrate(sizeof(SerialLinkHeader) +
                                                          hdr->dataLength);
                }
                else
                {
                    // Checksum failed, but the first byte was a preamble byte.
                    // Could be that it was a false positive or some other
                    // error.
                    // Discard the preamble byte and resync against the next
                    // preamble byte to be found, which may be within what
                    // was within the body of this message.
                    ClearLeadingBufferBytesAndRecalibrate(1);
                }
            }
        }
        
        // Check if the buffer is filled.  This would indicate that the
        // message is corrupted and a resync is required.
        // Drop the first preamble byte and resync.
        if (bufRxSize_ == BUF_CAPACITY)
        {
            ClearLeadingBufferBytesAndRecalibrate(1);
        }
        
        return retVal;
    }

    // Implement callback
    virtual void OnTimedEvent()
    {
        // Look for the first byte to synchronize stream
        if (state_ == State::LOOKING_FOR_PREAMBLE_BYTE)
        {
            TryToSyncStreamFromSerial();
        }
        
        if (state_ == State::LOOKING_FOR_END_OF_MESSAGE)
        {
            TryToAddMoreDataFromSerial();
            
            TryToProcessMessageFromCachedData();
        }
    }
    
    
    // Callback members
    CbFunction cbFn_;
    
    // RX Buffer members
    uint8_t bufRx_[BUF_CAPACITY];
    uint8_t bufRxSize_;
    State   state_;
    
    // TX Buffer members
    uint8_t bufTx_[BUF_CAPACITY];
};




#endif  // __SERIAL_LINK_H__







