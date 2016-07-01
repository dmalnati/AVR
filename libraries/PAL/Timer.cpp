#include "Timer.h"


// Storage for static members
TimerChannel *Timer1::channelAPtr_ = NULL;
TimerChannel *Timer1::channelBPtr_ = NULL;


ISR(TIMER1_COMPA_vect)
{
    if (Timer1::channelAPtr_)
    {
        Timer1::channelAPtr_->OnISR();
    }
}

ISR(TIMER1_COMPB_vect)
{
    if (Timer1::channelBPtr_)
    {
        Timer1::channelBPtr_->OnISR();
    }
}

