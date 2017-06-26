#include "Timer2.h"


// Want to be able to scope the speed of the clock of an AVR
// in order to know how far off correct it is.
//
// An 8-bit timer, at 8MHz, no prescaler, will loop (in theory)
// every 32us.
//
// Setting the A channel to toggle-on-match will show an
// alternating pulse whose width can be measured on a scope.
//
// The ATmega328P Timer2 Channel A pin is 17


static Timer2        t;
static TimerChannel *ta = t.GetTimerChannelA();


void setup()
{
    t.SetTimerPrescaler(Timer2::TimerPrescaler::DIV_BY_1);
    t.SetTimerMode(Timer2::TimerMode::FAST_PWM_TOP_OCRNA);

    ta->SetFastPWMModeBehavior(TimerChannel::FastPWMModeBehavior::SPECIAL_TOP_VALUE);
    ta->SetValue(255);

    t.StartTimer();
}

void loop() {}


