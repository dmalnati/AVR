#ifndef __AX_25_UI_MESSAGE_H__
#define __AX_25_UI_MESSAGE_H__


#include "PAL.h"


// Designed to work within a buffer that it does not own.
// Must be used in a certain specific way, since data is appended to the
// buffer and field reordering won't be taken into consideration.
// That way is to fill out the fields in the order they appear in the message.
//
//
// Sizing:
// Minimum size possible:  18 bytes  (dst, src, no route paths, control, pid, no info, checksum)
// Max legal size       : 330 bytes  (max frame payload plus checksum of HDLC)
//
// Sizing determined by:
// - Number of Routing Paths (each takes 7 bytes)
// - Size of Information Field
//
// Usage:
// - Init (can be done once regardless of number of times messages constructed)
// - Reset (each time a new message is to be created)
// - SetAddress
// - AppendInfo (as many times as you want)
// - Finalize (buffer now a valid AX.25 UI message)
//            (provided the buffer was large enough)
class AX25UIMessage
{
public:
    static const uint8_t MIN_BUF_SIZE = 18;

private:
    
    // We are sending a UI message, with no P/F request-for-response
    static const uint8_t AX25_CHAR_CONTROL = 0x03;
    
    // No Layer 3 protocol
    static const uint8_t AX25_CHAR_PID = 0xF0;
    
    static const uint16_t CRC16_INITIAL_VALUE    = 0xFFFF;
    static const uint16_t CRC16_CCITT_POLYNOMIAL = 0x8408;
    
public:
    AX25UIMessage()
    : buf_(NULL)
    , bufIdxNextByte_(1)
    , bufIdxControl_(0)
    , bufIdxPID_(0)
    {
        // Nothing to do
    }
    
    void Init(uint8_t *buf, uint8_t bufSize)
    {
        SetBuf(buf, bufSize);
    }
    
    void SetBuf(uint8_t *buf, uint8_t bufSize)
    {
        buf_     = buf;
        bufSize_ = bufSize;
        
        Reset();
    }
    
    void Reset()
    {
        // get ready for a new message to be created
        bufIdxNextByte_ = 0;
        
        // These aren't correct, they will need to be calculated later.
        bufIdxControl_ = 0;
        bufIdxPID_     = 0;
    }
    
    void SetDstAddress(const char *addr, uint8_t addrSSID)
    {
        AppendAddress(addr, addrSSID);
    }
    
    void SetSrcAddress(const char *addr, uint8_t addrSSID)
    {
        AppendAddress(addr, addrSSID);
    }
    
    void AddRepeaterAddress(const char *addr, uint8_t addrSSID)
    {
        AppendAddress(addr, addrSSID);
    }
    
    uint8_t GetUnsafePtrInfo(uint8_t **buf, uint8_t *bufSize)
    {
        uint8_t retVal = 0;
        
        if (buf && bufSize)
        {
            retVal = 1;
            
            *buf     = &(buf_[bufIdxNextByte_]);
            *bufSize = bufSize_ - (&(buf_[bufIdxNextByte_]) - buf_);
        }
        
        return retVal;
    }
    
    void AssertInfoBytesUsed(uint8_t bytesUsed)
    {
        bufIdxNextByte_ += bytesUsed;
    }

    void AppendInfo(uint8_t *buf, uint8_t bufSize)
    {
        memcpy((void *)&(buf_[bufIdxNextByte_]), buf, bufSize);
        bufIdxNextByte_ += bufSize;
    }
    
    // Returns number of bytes used
    uint8_t Finalize()
    {
        SetControl(AX25_CHAR_CONTROL);
        SetPID(AX25_CHAR_PID);
        CalcFCS();
        
        return bufIdxNextByte_;
    }


private:

    void AppendAddress(const char *addr, uint8_t addrSSID)
    {
        // undo any stop-bit from prior address append.
        // cope with situation where this is the first.
        if (bufIdxNextByte_ != 0)
        {
            // reverse idxNextByte by the control and pid bytes from last time.
            bufIdxNextByte_ -= 2;
            
            // undo the stop-bit of the prior address
            buf_[bufIdxNextByte_ - 1] &= 0b11111110;
        }
        
        // encode this address
        EncodeAddress(addr, addrSSID);
        
        // set stop bit of this address
        buf_[bufIdxNextByte_ - 1] |= 0b00000001;
        
        // calculate location of control field
        // (already pointed to by bufIdxNextByte_)
        bufIdxControl_ = bufIdxNextByte_;
        
        // calculate location of pid field
        ++bufIdxNextByte_;
        bufIdxPID_ = bufIdxNextByte_;
        
        // calculate starting location of data
        ++bufIdxNextByte_;
    }
    
    // Does not set stop bit, left for higher-level logic
    void EncodeAddress(const char *addr, uint8_t ssid)
    {
        // This will fill the next 7 bytes of the buffer.
        // The format is <SIGN>[spaces]<SSID>
        // The callsign is left-aligned, occupying up to 6 characters.
        // Any callsign less than 6 characters is space-padded on the right.
        // The 7th byte is for the SSID.
        // The formatting of the actual data is:
        // SIGN bytes
        // - each ascii chater is shifted 1 bit to the left, allowing for the
        //   least-significant-bit to represent flag data.
        // SSID byte
        // - HRRSSIDS
        //   - H    - "has been repeated" - set to 0 as we're doing initial send
        //   - RR   - Reserved.  Set each bit to 1.
        //   - SSID - The actual binary SSID, 0-15.
        //   - S    - Stop bit.  Set to 0 here, we aren't in charge of this.
        
        // validate length
        uint8_t addrLen = strlen(addr);
        
        // Initialize to max length supported
        uint8_t useByteCount = 6;
        uint8_t padByteCount = 0;
        
        if (addrLen < 6)
        {
            // will need to pad with spaces
            useByteCount = addrLen;
            padByteCount = 6 - useByteCount;
        }
        
        // Use the bytes we can
        const char *p = addr;
        for (uint8_t i = 0; i < useByteCount; ++i)
        {
            buf_[bufIdxNextByte_] = (*p << 1);
            
            ++bufIdxNextByte_;
            ++p;
        }
        
        // Pad if necessary
        for (uint8_t i = 0; i < padByteCount; ++i)
        {
            buf_[bufIdxNextByte_] = (' ' << 1);
            
            ++bufIdxNextByte_;
        }
        
        // Encode SSID
        buf_[bufIdxNextByte_] =
            (uint8_t)(0b01100000 | ((uint8_t)(ssid & 0x0F) << 1));
        
        ++bufIdxNextByte_;
    }
    
    void SetControl(uint8_t control)
    {
        buf_[bufIdxControl_] = control;
    }
    
    void SetPID(uint8_t pid)
    {
        buf_[bufIdxPID_] = pid;
    }
    
    
public:

    // Taken from:
    // http://digitalcommons.calpoly.edu/cgi/viewcontent.cgi?article=2449&context=theses
    // (light reformatting and removal of compiler warnings about comparing
    //  signed and unsigned values)
    
    // Calculate the CRC-16-CCITT of a given array of a given length
    // NOTE : Operates completely in reverse - bit order
    static uint16_t calc_crc (uint8_t frame [], uint8_t frame_len)
    {
        uint8_t i, j;
        
        // Preload the CRC register with ones
        uint16_t crc = 0xffff;
        
        // Iterate over every octet in the frame
        for (i = 0; i < frame_len; i++)
        {
            // Iterate over every bit, LSb first
            for (j = 0; j < 8; j++)
            {
                uint8_t bit = (frame[i] >> j) & 0x01;
                
                // Divide by a bit - reversed 0x1021
                if ((crc & 0x0001) != bit)
                {
                    crc = (crc >> 1) ^ 0x8408;
                }
                else
                {
                    crc = crc >> 1;
                }
            }
        }
        
        // Take the one's compliment of the calculated CRC
        crc = crc ^ 0xffff;
        
        return crc;
    }    
    

private:

    void CalcFCS()
    {
        // The CRC Function below already takes into account the special
        // requirements for transmitting the FCS, and the output of that
        // is suitable for placing in the byte stream and sending each byte
        // least-significant-bit first along with everything else.
        
        // Apply to Address, Control, PID, Info
        uint16_t crc = calc_crc(buf_, bufIdxNextByte_);
        
        // Put the bytes into our buffer
        memcpy((void *)&(buf_[bufIdxNextByte_]), (void *)&crc, sizeof(crc));
        
        // Move to next position
        bufIdxNextByte_ += sizeof(crc);
    }

    

    uint8_t *buf_;
    uint8_t  bufSize_;
    uint8_t  bufIdxNextByte_;
    uint8_t  bufIdxControl_;
    uint8_t  bufIdxPID_;
};



#endif  // __AX_25_UI_MESSAGE_H__









