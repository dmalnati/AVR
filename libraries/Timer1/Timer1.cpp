#include "Timer1.h"


// Storage for static members
function<void()> Timer1::cbFnA_;
function<void()> Timer1::cbFnB_;
function<void()> Timer1::cbFnOvf_;

TimerInterrupt::CbFnRaw Timer1::cbFnRawA_   = Timer1::OnFnRawADefault;
TimerInterrupt::CbFnRaw Timer1::cbFnRawB_   = Timer1::OnFnRawBDefault;
TimerInterrupt::CbFnRaw Timer1::cbFnRawOvf_ = Timer1::OnFnRawOvfDefault;

Timer1::TimerPrescaler  Timer1::timerPrescaler_(Timer1::GetTimerPrescalerFromRegister());
TimerChannel16Bit       Timer1::channelA_  (&cbFnA_,   &cbFnRawA_,   &TIMSK1, OCIE1A, &TIFR1, OCF1A, &OCR1A, &TCCR1A, COM1A1, COM1A0, Timer1::PIN_CHANNEL_A);
TimerChannel16Bit       Timer1::channelB_  (&cbFnB_,   &cbFnRawB_,   &TIMSK1, OCIE1B, &TIFR1, OCF1B, &OCR1B, &TCCR1A, COM1B1, COM1B0, Timer1::PIN_CHANNEL_B);
TimerInterrupt          Timer1::ovfHandler_(&cbFnOvf_, &cbFnRawOvf_, &TIMSK1, TOIE1,  &TIFR1, TOV1);


ISR(TIMER1_COMPA_vect)
{
    Timer1::cbFnRawA_();
}

ISR(TIMER1_COMPB_vect)
{
    Timer1::cbFnRawB_();
}

ISR(TIMER1_OVF_vect)
{
    Timer1::cbFnRawOvf_();
}

