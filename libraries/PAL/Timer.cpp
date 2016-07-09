#include "Timer.h"


// Storage for static members
function<void()> Timer1::cbFnA_;
function<void()> Timer1::cbFnB_;
function<void()> Timer1::cbFnOvf_;


// Just for debugging
/*
uint8_t pinA   = 27;
uint8_t pinB   = 28;
uint8_t pinOvf =  4;

Pin debugChannelA(pinA,     LOW);
Pin debugChannelB(pinB,     LOW);
Pin debugChannelOvf(pinOvf, LOW);
*/


ISR(TIMER1_COMPA_vect)
{
    //PAL.DigitalWrite(debugChannelA, HIGH);

    Timer1::cbFnA_();
    
    //PAL.DigitalWrite(debugChannelA, LOW);
}

ISR(TIMER1_COMPB_vect)
{
    //PAL.DigitalWrite(debugChannelB, HIGH);
    
    Timer1::cbFnB_();
    
    //PAL.DigitalWrite(debugChannelB, LOW);
}

ISR(TIMER1_OVF_vect)
{
    //PAL.DigitalWrite(debugChannelOvf, HIGH);
    
    Timer1::cbFnOvf_();
    
    //PAL.DigitalWrite(debugChannelOvf, LOW);
}



