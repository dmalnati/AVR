#ifndef __UBX_MESSAGE_H__
#define __UBX_MESSAGE_H__


template <uint8_t MAX_MSG_SIZE>
class UbxMessage
{
    static const uint8_t LEN_HEADER     = 6;
    static const uint8_t LEN_TRAILER    = 2;
    static const uint8_t LEN_RESERVED   = LEN_HEADER + LEN_TRAILER;
    
    static const uint8_t BUF_SIZE = MAX_MSG_SIZE < LEN_RESERVED ?
                                    LEN_RESERVED                :
                                    MAX_MSG_SIZE;
                                    
    static const uint8_t CHECKSUM_START = 2;
    static const uint8_t LENGTH_START   = 4;
    
public:
    UbxMessage()
    {
        Reset();
    }
    
    ~UbxMessage() {}
    
    void Reset()
    {
        bufIdx_ = LEN_HEADER;
        memset(buf_, 0, BUF_SIZE);
        
        buf_[0] = 0xB5; // Sync char 1
        buf_[1] = 0x62; // Sync char 2
    }
    
    void GetBuf(uint8_t **buf, uint8_t *bufLen)
    {
        FinalizeBuffer();
        
        *buf    = buf_;
        *bufLen = bufIdx_ + LEN_TRAILER;
    }
    
    void SetClass(uint8_t ubxClass)
    {
        buf_[2] = ubxClass;
    }
    
    void SetId(uint8_t ubxId)
    {
        buf_[3] = ubxId;
    }
    
    void AddFieldU1(uint8_t val)
    {
        AddData((void *)&val, sizeof(val));
    }
    
    void AddFieldU2(uint16_t val)
    {
        uint16_t valLittleEndian = PAL.htons(PAL.htons(val));
        
        AddData((void *)&valLittleEndian, sizeof(valLittleEndian));
    }
    
    void AddFieldU4(uint32_t val)
    {
        uint32_t valLittleEndian = PAL.htonl(PAL.htonl(val));
        
        AddData((void *)&valLittleEndian, sizeof(valLittleEndian));
    }
    
    void AddFieldI1(int8_t val)
    {
        AddData((void *)&val, sizeof(val));
    }
    
    void AddFieldI4(int32_t val)
    {
        uint32_t valLittleEndian = PAL.htonl(PAL.htonl(val));
        
        AddData((void *)&valLittleEndian, sizeof(valLittleEndian));
    }
    
    void AddFieldX2(uint16_t val)
    {
        AddFieldU2(val);
    }
    
    
private:

    void FinalizeBuffer()
    {
        CalculateAndStoreLength();
        CalculateAndStoreChecksum();
    }
    
    void CalculateAndStoreLength()
    {
        uint16_t length             = bufIdx_ - LEN_HEADER;
        uint16_t lengthLittleEndian = PAL.htons(PAL.htons(length));
        
        memcpy((void *)&(buf_[LENGTH_START]),
               (void *)&lengthLittleEndian,
               sizeof(lengthLittleEndian));
    }
    
    void CalculateAndStoreChecksum()
    {
        uint8_t ckA = 0;
        uint8_t ckB = 0;
        
        for (uint8_t i = CHECKSUM_START; i < bufIdx_; ++i)
        {
            uint8_t b = buf_[i];
            
            ckA += b;
            ckB += ckA;
        }
        
        buf_[bufIdx_ + 0] = ckA;
        buf_[bufIdx_ + 1] = ckB;
    }

    void AddData(void *bufData, uint8_t bufDataLen)
    {
        // Make sure there is space available for checksum
        if ((bufIdx_ + bufDataLen) <= (BUF_SIZE - LEN_TRAILER))
        {
            memcpy((void *)(&(buf_[bufIdx_])), bufData, bufDataLen);
        }
        
        bufIdx_ += bufDataLen;
    }

    uint8_t buf_[BUF_SIZE];
    uint8_t bufIdx_;
};


#endif  // __UBX_MESSAGE_H__