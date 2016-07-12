#include "Timer1.h"
#include "Timer2.h"


static Timer1 t1;
static TimerChannel   *t1cA  = t1.GetTimerChannelA();
static TimerChannel   *t1cB  = t1.GetTimerChannelB();
static TimerInterrupt *t1Ovf = t1.GetTimerOverflowHandler();

static Timer2 t2;
static TimerChannel   *t2cA  = t2.GetTimerChannelA();
static TimerChannel   *t2cB  = t2.GetTimerChannelB();
static TimerInterrupt *t2Ovf = t2.GetTimerOverflowHandler();


void setup() {}
void loop() {}



