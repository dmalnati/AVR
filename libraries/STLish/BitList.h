#ifndef __BIT_LIST_H__
#define __BIT_LIST_H__


template <uint8_t BIT_COUNT>
class BitList
{
public:
    BitList()
    {
        Reset();
    }

    void Reset()
    {
        for (uint8_t i = 0; i < BYTE_COUNT; ++i)
        {
            byteList_[i] = 0;
        }
        
        bitMask_     = 0x80;
        byteListIdx_ = 0;
    }
    
    inline uint8_t PushBack(uint8_t bitVal)
    {
        uint8_t retVal = 0;
        
        if (byteListIdx_ < BYTE_COUNT)
        {
            retVal = 1;
            
            if (bitVal)
            {
                byteList_[byteListIdx_] |= _BV(bitMask_);
            }
            else
            {
                byteList_[byteListIdx_] &= ~_BV(bitMask_);
            }
            
            bitMask_ >>= 1;
            
            if (!bitMask_)
            {
                bitMask_ = 0x80;
                
                ++byteListIdx_;
            }
        }
        
        return retVal;
    }
    
    inline uint8_t operator[](uint8_t bitIdx)
    {
        uint8_t retVal = 0;
        
        if (bitIdx < BIT_COUNT)
        {
            uint8_t byteListIdxTmp = (bitIdx >> 3);
            uint8_t bitMaskTmp     = (0x80 >> (bitIdx - (byteListIdxTmp << 3)));
            
            retVal = (byteList_[byteListIdxTmp] & bitMaskTmp) ? 1 : 0;
        }
        
        return retVal;
    }
    
    inline static uint8_t Size()
    {
        return BIT_COUNT;
    }

private:
    static const uint8_t BYTE_COUNT =
        (BIT_COUNT % 8 == 0 ?
         (BIT_COUNT / 8)    :
         (BIT_COUNT / 8) + 1);

    uint8_t byteList_[BYTE_COUNT];
    
    uint8_t bitMask_;
    uint8_t byteListIdx_;
};


#endif  // __BIT_LIST_H__



















