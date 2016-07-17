#ifndef __AX_25_UI_MESSAGE_H__
#define __AX_25_UI_MESSAGE_H__


#include "PAL.h"


// Designed to work within a buffer that it does not own.
// Must be used in a certain specific way, since data is appended to the
// buffer and field reordering won't be taken into consideration.
// That way is to fill out the fields in the order they appear in the message.
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
    
    ~AX25UIMessage() {}

    void Init(uint8_t *buf)
    {
        SetBuf(buf);
    }
    
    void SetBuf(uint8_t *buf)
    {
        buf_ = buf;
        
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
    

    // Does not try to set stop bit.
    void EncodeAddress(const char *addr, uint8_t ssid)
    {
        // should be of form: ascii string, 6 char, space padded on right
        
        // validate anyway
        uint8_t addrLen = strlen(addr);
        
        // Assume well formed
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

    void SetAddress(const char *addrDst,
                    uint8_t     addrDstSSID,
                    const char *addrSrc,
                    uint8_t     addrSrcSSID)
    {
        EncodeAddress(addrDst, addrDstSSID);
        EncodeAddress(addrSrc, addrSrcSSID);
        
        // set stop-bit on final byte of address
        buf_[bufIdxNextByte_ - 1] |= 0x01;
        
        
        // calculate location of control field
        // (already pointed to by bufIdxNextByte_)
        bufIdxControl_ = bufIdxNextByte_;
        
        // calculate location of pid field
        ++bufIdxNextByte_;
        bufIdxPID_ = bufIdxNextByte_;
        
        // calculate starting location of data
        ++bufIdxNextByte_;
        
        
        
        // each byte needs to be shifted left 1 bit to indicate either:
        // 0 - another byte follows
        // 1 - end of bytes
        
        // Indicate 1 only after dst
        
        // 14 bytes for non-repeater point-to-point transmission.
        // online doc shows typical format.
        
        // each callsign must space separate itself from the SSID.
        // (consider using 4 params to this fn?)
        
        // Unclear how repeater-based transmission should look
        // (such as how they know the end of the repeater vs src
        //  since the field widths aren't 7 and 7 for dst and src anymore)
        
        /*
        
          Wait, clearer now.
          Unconditionally encode the 14 bytes as described above.
          If repeaters are being used, they are appended to the end of the
          address field contiguously.
          This also means that the stop-bit on the end of the src address is not
          used in this case, and instead is used at the end of the repeater
          addressing data.
          
          The most-significant bit in the final octet of the SSID of the last
          repeater is set to 0 on initial transmission, and 1 once repeated.
          (this doesn't seem problematic since numeric ASCII characters don't
           make use of that bit even when shifted left 1 bit, as is necessary
           for the stop-bit encoding)
        
         */
    }

    // max 256 bytes pre-stuffing
    void AppendInfo(uint8_t *buf, uint8_t bufSize)
    {
        memcpy((void *)&(buf_[bufIdxNextByte_]), buf, bufSize);
        bufIdxNextByte_ += bufSize;
    }
    
    
    // Returns number of bytes used or 0 on error.
    uint8_t Finalize()
    {
        SetControl(AX25_CHAR_CONTROL);
        SetPID(AX25_CHAR_PID);
        CalcFCS();
        
        return bufIdxNextByte_;
    }
    


private:

    
    void SetControl(uint8_t control)
    {
        buf_[bufIdxControl_] = control;
    }
    
    void SetPID(uint8_t pid)
    {
        buf_[bufIdxPID_] = pid;
    }
    
    // Taken from:
    // https://github.com/tcort/va2epr-tnc/blob/master/firmware/aprs.c
    static uint16_t CRC16NextByte(uint16_t crc, uint8_t byte)
    {
        uint8_t i;

        crc ^= byte;

        /* for each bit in 'byte' */
        for (i = 0; i < 8; i++) {

            /* if LSB of 'crc' is HIGH (1) */
            if (crc & 0x0001) {
                /*
                 * Optimization Note:
                 * avr-gcc generates fewer instructions for
                 * "if (crc & 0x0001)" than for "if (crc << 15)"
                 */

                /* Shift Right by 1 and XOR the result with the polynomial */
                crc = (crc>>1) ^ CRC16_CCITT_POLYNOMIAL;
            } else {

                /* Shift Right  by 1 */
                crc >>= 1;
            }
        }

        return crc;
    }
    


    void CalcFCS()
    {
        // ISO 3309 (HDLC)
        
        // reverse bits when done for transmission
        // Is the number supposed to be big-endian on the wire
        // (not considering the bit switching)?

        
        // So...
        // calc a 16 bit int
        // Make it BigEndian in RAM
        // Reverse bits of each byte
        // That way, when streamed least-significant-bit first, the end result
        // will be what the spec calls for.
        //    This relies on the transmission of the binary data to uniformally
        //    transmit all bytes lsb first.
        
        
        
        // EX:
        // BigEndian          : 0x95D3 -- 1001 0101  1101 0011
        // Swapped 8-bit bytes: 0xA9CB -- 1010 1001  1100 1011
        
        
        // Apply to Address, Control, PID, Info
        
        uint16_t crc = CRC16_INITIAL_VALUE;
        
        for (uint8_t i = 0; i < bufIdxNextByte_; ++i)
        {
            uint8_t b = buf_[i];
            
            crc = CRC16NextByte(crc, b);
        }
        
        // Get BigEndian representation
        uint16_t crcBigEndian = PAL.htons(crc);
        
        // Get copy to swap the bits of each byte around within
        uint16_t crcBigEndianBitSwap = crcBigEndian;
        
        // Reverse the bits of each byte
        uint8_t *p = (uint8_t *)&crcBigEndianBitSwap;
        
        for (uint8_t i = 0; i < sizeof(crcBigEndianBitSwap); ++i)
        {
            // Get the byte as-is
            uint8_t b = p[i];
            
            // Get container for new byte
            uint8_t bNew = 0;
            
            // Transfer bits
            for (uint8_t j = 0; j < 8; ++j)
            {
                bNew <<= 1;
                
                bNew |= (uint8_t)(b & 0x01);
                
                b >>= 1;
            }
            
            // Store new byte
            p[i] = bNew;
        }
        
        // Put the bytes into our buffer
        memcpy((void *)&(buf_[bufIdxNextByte_]),
               (void *)&crcBigEndianBitSwap,
               sizeof(crcBigEndianBitSwap));
               
        
        // Move to next position
        bufIdxNextByte_ += sizeof(crcBigEndianBitSwap);
    }

    

    uint8_t *buf_;
    uint8_t  bufIdxNextByte_;
    uint8_t  bufIdxControl_;
    uint8_t  bufIdxPID_;
};



#endif  // __AX_25_UI_MESSAGE_H__









