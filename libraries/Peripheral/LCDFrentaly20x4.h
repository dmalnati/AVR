#ifndef __LCD_FRENTALY_20X4_H__
#define __LCD_FRENTALY_20X4_H__


#include "LiquidCrystal_I2C.h"


class LCDFrentaly20x4
{
    static const uint8_t I2C_ADDR  = 0x3F;
    static const uint8_t COL_COUNT = 20;
    static const uint8_t ROW_COUNT = 4;
    
public:
    LCDFrentaly20x4(uint8_t i2cAddr = I2C_ADDR)
    : lcd_(i2cAddr, COL_COUNT, ROW_COUNT)
    {
        // Nothing to do
    }
    
    void Init()
    {
        lcd_.begin();
        lcd_.backlight();
    }
    
    template <typename T>
    uint8_t Print(T &&t)
    {
        uint8_t charCount = lcd_.print(t);
        
        return charCount;
    }
    
    template <typename T>
    uint8_t PrintAt(uint8_t col, uint8_t row, T &&t)
    {
        lcd_.setCursor(col, row);
        
        return Print(t);
    }
    
    void Clear()
    {
        lcd_.clear();
    }
    

private:

    LiquidCrystal_I2C lcd_;
};



#endif  // __LCD_FRENTALY_20X4_H__


