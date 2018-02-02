#ifndef __EEPROM_H__
#define __EEPROM_H__


#include <avr/eeprom.h>

#include "crc.h"


class EepromAddressManager
{
public:
    static uint16_t GetBlock(uint16_t size)
    {
        uint16_t retVal = addrNext_;
        
        addrNext_ += size;
        
        return retVal;
    }
    
private:
    static uint16_t addrNext_;
};



template <typename T, uint8_t CAPACITY = 1>
class EepromAccessor
{
public:
    EepromAccessor()
    : addrStart_(EepromAddressManager::GetBlock(sizeof(Wrapper) * CAPACITY))
    {
        // Nothing to do
    }
    
    uint8_t GetCapacity()
    {
        return CAPACITY;
    }
    
    uint8_t Read(T &val)
    {
        return ReadFromIdx(val, 0);
    }
    
    uint8_t ReadFromIdx(T &val, uint8_t idx)
    {
        uint8_t retVal = 0;
        
        if (idx < CAPACITY)
        {
            Wrapper eObj;
            
            uint16_t addr = GetAddrAtIdx(idx);
            
            eeprom_read_block((void *)&eObj, (const void *)addr, sizeof(Wrapper));
            val = eObj.val;
            
            uint8_t crc = CRC8((const uint8_t *)&eObj.val, sizeof(T));
            
            retVal = (crc == eObj.crc);
        }
        
        return retVal;
    }
    
    uint8_t Write(T &val)
    {
        return WriteToIdx(val, 0);
    }
    
    uint8_t WriteToIdx(T &val, uint8_t idx)
    {
        uint8_t retVal = 0;
        
        if (idx < CAPACITY)
        {
            retVal = 1;
            
            Wrapper eObj;
            
            eObj.val = val;
            eObj.crc = CRC8((const uint8_t *)&eObj.val, sizeof(T));
            
            uint16_t addr = GetAddrAtIdx(idx);
            
            eeprom_update_block((const void *)&eObj, (void *)addr, sizeof(Wrapper));
        }
        
        return retVal;
    }
    
    uint8_t Delete()
    {
        return DeleteAtIdx(0);
    }
    
    uint8_t DeleteAtIdx(uint8_t idx)
    {
        uint8_t retVal = 0;
        
        if (idx < CAPACITY)
        {
            retVal = 1;
            
            uint8_t buf[sizeof(Wrapper)];
            
            Wrapper *eObj = (Wrapper *)buf;
            
            memset((void *)buf, 0xFF, sizeof(Wrapper));
            eObj->crc = 0xFF;
            
            uint16_t addr = GetAddrAtIdx(idx);
            
            eeprom_update_block((const void *)eObj, (void *)addr, sizeof(Wrapper));
        }
        
        return retVal;
    }
    
    
private:

    struct Wrapper
    {
        T       val;
        uint8_t crc;
    };
    
    uint16_t GetAddrAtIdx(uint8_t idx)
    {
        return addrStart_ + (sizeof(Wrapper) * idx);
    }

    uint16_t addrStart_;
};


#endif  // __EEPROM_H__









