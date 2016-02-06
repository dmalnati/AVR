#ifndef __MY_SHIFT_REGISTER_H__
#define __MY_SHIFT_REGISTER_H__


#include <Arduino.h>

class MyShiftRegister
{
public:
    MyShiftRegister(uint8_t pinLoad,
                    uint8_t pinClock,
                    uint8_t pinClockEnable,
                    uint8_t pinSerial);
    ~MyShiftRegister() { }
    
    uint8_t ShiftIn();

private:
    void Setup();

    uint8_t pinLoad_;
    uint8_t pinClock_;
    uint8_t pinClockEnable_;
    uint8_t pinSerial_;
};



#endif // __MY_SHIFT_REGISTER_H__