

#include "PAL.h"




class APRSMessage
{
public:


private:
};




// Designed to work within a buffer that it does not own
class AX25InformationMessage
{
    static const uint8_t AX25_CHAR_FLAG = 0x7E;
    
public:
    AX25InformationMessage()
    : buf_(NULL)
    , bufSize_(0)
    {
        // Nothing to do
    }
    
    ~AX25InformationMessage() {}

    void Init(uint8_t buf, uint8_t bufSize)
    {
        SetBuf(buf, bufSize);
    }
    
    void SetBuf(uint8_t buf, uint8_t bufSize)
    {
        buf_     = buf;
        bufSize_ = bufSize;
        
        Reset();
    }
    
    void Reset()
    {
        // Does this reset the msg seqno, or just get ready for a new
        // message to be created?
        
        buf_[0] = AX25_CHAR_FLAG;
        
        
    }
    
    uint8_t FinalizeBuf()
    {
        uint8_t retVal = 0;
        
        // calc FCS (before or after bit stuffing?)
        // bit stuff
        // set end flag
        
        return retVal;
    }
    
    
    void SetAddress(char *addrSrc, char *addrDst)
    {
        // affects where Info data goes?
        
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
    void AppendInfo(uint8_t buf, uint8_t bufSize)
    {
        // affects where Info data goes
    }
    
    
    


private:

    
    void SetControl(uint8_t control)
    {
        // Too complex for caller to operate?
        // Should get set automatically in Finalize or Reset.
        
        // Possible to get away with using 0b00000000?
            // need to figure out correct P/F value to mean I'm not looking
            // for a reply.
    }
    
    void SetPID(uint8_t pid)
    {
        // whatever the "none" representation is
    }
    

    void BitStuff()
    {
        
    }

    void CalcFCS()
    {
        // ISO 3309 (HDLC)
        
        // reverse bits when done for transmission
        
        // 2 bytes.  reverse each byte I guess?
    }



    uint8_t buf_;
    uint8_t bufSize_;
};
























// Physical layer handler

// Need to handle being configured to:
// - send LSB or MSB first for a given byte
// - encoding - NRZI

template <uint8_t MAX_MSG_SIZE>
class ModemBell202
{
    static const uint8_t BUF_SIZE = MAX_MSG_SIZE ? MAX_MSG_SIZE : 1;
    
    static const uint16_t DEFAULT_BAUD = 1200;
    
    static const uint16_t BELL_202_TONE_MARK  = 1200;
    static const uint16_t BELL_202_TONE_SPACE = 2200;
    
    
public:
    ModemBell202(uint8_t pinTx, uint8_t pinEnable)
    : pinTx_(pinTx)
    , pinEnable_(pinEnable)
    {
        PAL.PinMode(pinEnable_, OUTPUT);
        PAL.DigitalWrite(pinEnable_, LOW);
    }
    
    ~ModemBell202() {}
    
    void Init()
    {
        
    }
    
    uint8_t Send(uint8_t* buf, uint8_t len)
    {
        uint8_t retVal = 0;


        return retVal;
    }
    
    uint8_t GetTxBuf(uint8_t **buf, uint8_t *bufLen)
    {
        *buf    = buf_;
        *bufLen = BUF_SIZE;
        
        return 1;
    }
    
    uint8_t SendFromTxBuf(uint8_t bufLen)
    {
        return Send(buf_, bufLen);
    }
    
private:



    uint8_t pinTx_;
    uint8_t pinEnable_;
    
    uint8_t buf_[BUF_SIZE];
};



























