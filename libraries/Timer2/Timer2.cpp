#include "Timer2.h"


// Storage for static members
function<void()> Timer2::cbFnA_;
function<void()> Timer2::cbFnB_;
function<void()> Timer2::cbFnOvf_;

TimerInterrupt::CbFnRaw Timer2::cbFnRawA_   = Timer2::OnFnRawADefault;
TimerInterrupt::CbFnRaw Timer2::cbFnRawB_   = Timer2::OnFnRawBDefault;
TimerInterrupt::CbFnRaw Timer2::cbFnRawOvf_ = Timer2::OnFnRawOvfDefault;

Timer2::TimerPrescaler  Timer2::timerPrescaler_(GetTimerPrescalerFromRegister());
TimerChannel8Bit        Timer2::channelA_  (&cbFnA_,   &cbFnRawA_,   &TIMSK2, OCIE2A, &TIFR2, OCF2A, &OCR2A, &TCCR2A, COM2A1, COM2A0, Timer2::PIN_CHANNEL_A);
TimerChannel8Bit        Timer2::channelB_  (&cbFnB_,   &cbFnRawB_,   &TIMSK2, OCIE2B, &TIFR2, OCF2B, &OCR2B, &TCCR2A, COM2B1, COM2B0, Timer2::PIN_CHANNEL_B);
TimerInterrupt          Timer2::ovfHandler_(&cbFnOvf_, &cbFnRawOvf_, &TIMSK2, TOIE2,  &TIFR2, TOV2);


ISR(TIMER2_COMPA_vect)
{
    Timer2::cbFnRawA_();
}

ISR(TIMER2_COMPB_vect)
{
    Timer2::cbFnRawB_();
}

ISR(TIMER2_OVF_vect)
{
    Timer2::cbFnRawOvf_();
}

