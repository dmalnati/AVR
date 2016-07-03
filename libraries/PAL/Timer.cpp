#include "Timer.h"


// Storage for static members
function<void()> Timer1::cbFnA_;
function<void()> Timer1::cbFnB_;


ISR(TIMER1_COMPA_vect)
{
    Timer1::cbFnA_();
}

ISR(TIMER1_COMPB_vect)
{
    Timer1::cbFnB_();
}

