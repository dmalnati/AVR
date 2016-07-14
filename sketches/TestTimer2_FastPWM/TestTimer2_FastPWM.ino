#include "Timer2.h"


static Timer2          t2;
static TimerChannel   *t2cA  = t2.GetTimerChannelA();
static TimerChannel   *t2cB  = t2.GetTimerChannelB();
static TimerInterrupt *t2Ovf = t2.GetTimerOverflowHandler();

static Pin pinTimer2ISROvf(25, LOW);

static const uint8_t TOP = 247;


void setup()
{
    // Set up timer to be FastPWM, 8-bit (natively)
    t2.SetTimerPrescaler(Timer2::TimerPrescaler::DIV_BY_1);
    t2.SetTimerMode(Timer2::TimerMode::FAST_PWM_TOP_OCRNA);
    t2.SetTimerValue(0);

    // Set up TOP value
    t2cA->SetValue(TOP);
    
    // Set 150/TOP duty cycle
    t2cB->SetValue(150);

    // Set up callback to know when overflow
    t2Ovf->SetInterruptHandler([&](){
        PAL.DigitalToggle(pinTimer2ISROvf);
    });
    t2Ovf->RegisterForInterrupt();

    // Begin timer counting
    t2.StartTimer();

    // Begin comparing B to the timer value in order to operate the
    // output level
    t2cB->SetFastPWMModeBehavior(
        TimerChannel::FastPWMModeBehavior::CLEAR
    );
}

void loop() {}





