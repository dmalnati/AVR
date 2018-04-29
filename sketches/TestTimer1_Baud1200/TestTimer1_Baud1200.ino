#include "PAL.h"
#include "Timer1.h"
#include "Evm.h"


static Evm::Instance<10,10,10> evm;

static Timer1          t;
static TimerChannel   *ta = NULL;
static TimerChannel   *tb = NULL;
static TimerInterrupt *ti = NULL;

Pin pinTimer1ISRA(14, LOW);
Pin pinTimer1ISROvf(19, LOW);



void setup()
{
    @fix@Serial.begin(9600);

    ta  = t.GetTimerChannelA();
    tb  = t.GetTimerChannelB();
    ti =  t.GetTimerOverflowHandler();

    TestBaud1200();

    evm.MainLoop();
}

void TestBaud1200()
{
    // Using 16-bit FastPWM, can I achieve the right 1200 baud bit spacing?

    // Funcntion of Timer:
    // - 16-bit of range FastPWM (65536 values)
    // - Channel A - interrupt handler at desired interval
    // - Channel B - nothing
    // - Overflow  - nothing

    // Approach:
    // - determine number of ticks for the given period (what if not whole?)
    // - remove any number of ticks required for important code to actually start functioning

    // A few constants for our 8MHz, no-prescaler, 16-bit timer
    double FUDGE_TIME_US   =     0;
    double SCALING_FACTOR  =     1.005;
    double US_PER_TICK     =     0.125;
    double TICKS_PER_CYCLE = 65536;
    

    // Start with the actual period you want
    double periodLogicalUs = 1000.0 / 1200.0 * 1000.0;

    // Now account for how long the function will take to start executing useful code
    //double periodUs = periodLogicalUs - FUDGE_TIME_US;
    double periodUs = periodLogicalUs * SCALING_FACTOR;

    // Convert the duration in us into ticks of the timer
    double ticksPerPeriod = periodUs / US_PER_TICK;

    uint16_t top = ticksPerPeriod - 1;

    // Let's see what we calculated
    @fix@Serial.println();
    @fix@Serial.print("FUDGE_TIME_US       : ");
    @fix@Serial.println(FUDGE_TIME_US);
    @fix@Serial.print("SCALING_FACTOR      : ");
    @fix@Serial.println(SCALING_FACTOR);
    @fix@Serial.print("US_PER_TICK         : ");
    @fix@Serial.println(US_PER_TICK);
    @fix@Serial.print("TICKS_PER_CYCLE     : ");
    @fix@Serial.println(TICKS_PER_CYCLE);
    @fix@Serial.print("periodLogicalUs     : ");
    @fix@Serial.println(periodLogicalUs);
    @fix@Serial.print("periodUs            : ");
    @fix@Serial.println(periodUs);
    @fix@Serial.print("ticksPerPeriod      : ");
    @fix@Serial.println(ticksPerPeriod);
    @fix@Serial.print("top                 : ");
    @fix@Serial.println(top);
    @fix@Serial.println();

    delay(1000);

    // Set up timer
    t.SetTimerPrescaler(Timer1::TimerPrescaler::DIV_BY_1);
    t.SetTimerMode(Timer1::TimerMode::FAST_PWM_TOP_OCRNA);
    t.SetTimerValue(0);

    // Set up top value
    ta->SetValue(top);

    // Set up interrupt handler
    ta->SetInterruptHandler([&](){
        PlatformAbstractionLayer::DigitalToggle(pinTimer1ISRA);

        // Do some user thing
    });
    ta->RegisterForInterrupt();
    
    // Begin
    t.StartTimer();

    // Don't let Timer0 interfere with our precise timing efforts
    PAL.PowerDownTimer0();

    // Also prevent Evm from looping and trying to handle interrupts
    // (which block ISRs)
    while (1)
    {
    }
}


void loop() {}









