#include "Timer2.h"


// Storage for static members
function<void()> Timer2::cbFnA_;
function<void()> Timer2::cbFnB_;
function<void()> Timer2::cbFnOvf_;


ISR(TIMER2_COMPA_vect)
{
    Timer2::cbFnA_();
}

ISR(TIMER2_COMPB_vect)
{
    Timer2::cbFnB_();
}

ISR(TIMER2_OVF_vect)
{
    Timer2::cbFnOvf_();
}

