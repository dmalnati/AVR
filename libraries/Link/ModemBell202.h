

#include "PAL.h"




class APRSMessage
{
public:


private:
};




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
    static const uint8_t AX25_CHAR_FLAG = 0x7E;
    
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
        bufIdxNextByte_ = 1;
        
        // These aren't correct, they will need to be calculated later.
        bufIdxControl_ = 0;
        bufIdxPID_     = 0;
    }

    // src and dst are 7 char, space padded between callsign and SSID
    void SetAddress(const char *addrDst7char,
                    const char *addrSrc7char,
                    const char *addrRepeaterStr = NULL)
    {
        // affects where Control, PID, Info data goes
        
        const char *p = NULL;
        
        // encode dst
        p = addrDst7char;
        while (*p)
        {
            buf_[bufIdxNextByte_] = (*p << 1);
            
            ++bufIdxNextByte_;
            ++p;
        }
        
        // encode src
        p = addrSrc7char;
        while (*p)
        {
            buf_[bufIdxNextByte_] = (*p << 1);
            
            ++bufIdxNextByte_;
            ++p;
        }
        
        // encode repeater (optional)
        if (addrRepeaterStr)
        {
            p = addrRepeaterStr;
            while (*p)
            {
                buf_[bufIdxNextByte_] = (*p << 1);
                
                ++bufIdxNextByte_;
                ++p;
            }            
        }
        
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
        SetFlags();
        
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
        
        
        // Apply to Address, Control, PID, Info
        
        uint16_t crc = CRC16_INITIAL_VALUE;
        
        for (uint8_t i = 1; i < bufIdxNextByte_; ++i)
        {
            uint8_t b = buf_[i];
            
            crc = CRC16NextByte(crc, b);
        }
        
        uint16_t crcBigEndian = PAL.htons(crc);
        
        memcpy((void *)&(buf_[bufIdxNextByte_]),
               (void *)&crcBigEndian,
               sizeof(crcBigEndian));
               
        
        // Move to next position
        ++bufIdxNextByte_;
    }


    void SetFlags()
    {
        buf_[0]               = AX25_CHAR_FLAG;
        buf_[bufIdxNextByte_] = AX25_CHAR_FLAG;
        
        // Move to next position
        ++bufIdxNextByte_;
    }

    
    

    uint8_t *buf_;
    uint8_t  bufIdxNextByte_;
    uint8_t  bufIdxControl_;
    uint8_t  bufIdxPID_;
};
























// Physical layer handler

// Need to handle being configured to:
// - send LSB or MSB first for a given byte
// - encoding - NRZI
// - Bit stuffing might best be done here...

class ModemBell202
{
    static const uint8_t BUF_SIZE = MAX_MSG_SIZE ? MAX_MSG_SIZE : 1;
    
    static const uint16_t DEFAULT_BAUD = 1200;
    
    static const uint16_t BELL_202_TONE_MARK  = 1200;
    static const uint16_t BELL_202_TONE_SPACE = 2200;
    
    static const uint16_t TONE_LIST[2] = {
        BELL_202_TONE_MARK,
        BELL_202_TONE_SPACE
    };
    
    
public:
    ModemBell202(uint8_t pinTx, uint8_t pinEnable)
    : pinTx_(pinTx)
    , pinEnable_(pinEnable)
    , toneListIdx_(0)
    {
        PAL.PinMode(pinEnable_, OUTPUT);
        PAL.DigitalWrite(pinEnable_, LOW);
    }
    
    ~ModemBell202() {}
    
    void Init()
    {
        Reset();
    }
    
    void Reset()
    {
        toneListIdx_ = 0;
    }
    
    uint8_t Send(uint8_t* buf, uint8_t bufLen)
    {
        uint8_t retVal = 0;
        
        Reset();
        
        for (uint8_t i = 0; i < bufLen; ++i)
        {
            SendByte(buf[i]);
        }
        
        return retVal;
    }

    
private:

    void SendByte(uint8_t b)
    {
        for (uint8_t i = 0; i < 8; ++i)
        {
            // LSB vs MSB
            // Bit stuffing
                // how to deal with first and last byte?
        }
    }
    
    void SendBit(uint8_t val)
    {
        // NRZI
        
        if (val)
        {
            // do transition
            toneListIdx_ = !toneListIdx_;
        }
        else
        {
            // no transition
        }
        
        SendTone(TONE_LIST[toneListIdx_]);
    }
    
    void SendTone(uint16_t frequency)
    {
        tone(frequency);
        delayMicroseconds();
    }


    uint8_t pinTx_;
    uint8_t pinEnable_;
    
    uint8_t toneListIdx_;
};



























