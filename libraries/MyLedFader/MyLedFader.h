#ifndef __MY_LED_FADER_H__
#define __MY_LED_FADER_H__

#include <Arduino.h>


class MyLedFader
{
public:
    MyLedFader(uint8_t pin, uint8_t cycleTimeInSecs, PinMode mode) {}
    ~MyLedFader() {}
    
    void Start();
    void Stop();
    
    enum PinMode {
        PWM = 0,
        ADC = 1
    }

private:
    uint8_t pin_;
};



#endif  // __MY_LED_FADER_H__