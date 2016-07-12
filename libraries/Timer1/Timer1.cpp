#include "Timer1.h"


// Storage for static members
function<void()> Timer1::cbFnA_;
function<void()> Timer1::cbFnB_;
function<void()> Timer1::cbFnOvf_;


ISR(TIMER1_COMPA_vect)
{
    Timer1::cbFnA_();
}

ISR(TIMER1_COMPB_vect)
{
    Timer1::cbFnB_();
}

ISR(TIMER1_OVF_vect)
{
    Timer1::cbFnOvf_();
}

