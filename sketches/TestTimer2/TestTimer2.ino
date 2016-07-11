#include "PAL.h"
#include "Timer.h"
#include "Evm.h"


static Evm::Instance<10,10,10> evm;

static Timer1          t;
static TimerChannel   *ta = NULL;
static TimerChannel   *tb = NULL;
static TimerInterrupt *ti = NULL;

static const uint8_t PIN_ISR_A1 = 27;
static const uint8_t PIN_ISR_A2 = 28;

Pin pinIsrA1(PIN_ISR_A1, LOW);
Pin pinIsrA2(PIN_ISR_A2, LOW);


void setup()
{
    Serial.begin(9600);

    ta  = t.GetTimerChannelA();
    tb  = t.GetTimerChannelB();
    ti =  t.GetTimerOverflowHandler();

    TestBaud1200();

    evm.MainLoop();
}

void TestBaud1200()
{
    // Using 8-bit FastPWM, can I achieve the right 1200 baud bit spacing?

    // Funcntion of Timer:
    // - 8-bit full range FastPWM (256 values)
    // - Channel A - nothing
    // - Channel B - represent duty cycle
    // - Overflow:
    //   - changes duty cycle for B
    //   - counts down user callback (for baud)


    // Approach:
    // - determine number of ticks for the given period (what if not whole?)
    // - remove any number of ticks required for important code to actually start functioning
    // - set first loop timeout to be the overflow
    // - re-set for every subsequent loop
    // - when final countdown over, fire
    // - don't forget to have a term which scales the "duration" of a tick to offset the wrongness
    //   of the avr clock

    // A few constants for our 8MHz, no-prescaler, 8-bit timer
    double FUDGE_TIME_US   =  34;
    double US_PER_TICK     =   0.125;
    double TICKS_PER_CYCLE = 256;

    // Start with the actual period you want
    double periodLogicalUs = 1000.0 / 1200.0 * 1000.0;

    // Now account for how long the function will take to start executing useful code
    double periodUs = periodLogicalUs - FUDGE_TIME_US;

    // Todo -- scale for actual clock skew

    // Convert the duration in us into ticks of the timer
    double ticksPerPeriod = periodUs / US_PER_TICK;

    // Now convert ticks into number of cycles.
    double cyclesPerPeriod = ticksPerPeriod / TICKS_PER_CYCLE;
    
    // Now break into whole cycles, and ticks for the fractional cycle part
    uint32_t cyclesPerPeriodWhole = (uint32_t)cyclesPerPeriod;
    double   ticksRemainder       = (cyclesPerPeriod - cyclesPerPeriodWhole) * TICKS_PER_CYCLE;
    uint32_t ticksRemainderWhole  = (uint32_t)ticksRemainder;
    
    Serial.println();
    Serial.print("FUDGE_TIME_US       : ");
    Serial.println(FUDGE_TIME_US);
    Serial.print("US_PER_TICK         : ");
    Serial.println(US_PER_TICK);
    Serial.print("TICKS_PER_CYCLE     : ");
    Serial.println(TICKS_PER_CYCLE);
    Serial.print("periodLogicalUs     : ");
    Serial.println(periodLogicalUs);
    Serial.print("periodUs            : ");
    Serial.println(periodUs);
    Serial.print("ticksPerPeriod      : ");
    Serial.println(ticksPerPeriod);
    Serial.print("cyclesPerPeriod     : ");
    Serial.println(cyclesPerPeriod);
    Serial.print("cyclesPerPeriodWhole: ");
    Serial.println(cyclesPerPeriodWhole);
    Serial.print("ticksRemainder      : ");
    Serial.println(ticksRemainder);
    Serial.print("ticksRemainderWhole : ");
    Serial.println(ticksRemainderWhole);
    Serial.println();

    delay(1000);



    // Set up user-specified callback period
    // (This is A's first value, so just count overflows)
    struct UserPeriodCallbackData
    {
        uint8_t tickIncrement;
        uint8_t overflowsExpected;
        uint8_t overflowsRemaining;
        uint8_t aValueLast;
    };

    UserPeriodCallbackData d;
    d.tickIncrement      = ticksRemainderWhole;
    d.overflowsExpected  = cyclesPerPeriodWhole + 1; // since the remainder part is going to count
    d.overflowsRemaining = d.overflowsExpected;
    d.aValueLast         = d.tickIncrement - 1;

    UserPeriodCallbackData *dp = &d;

    // todo -- consider what happens when remainder == 0

    // Set up timer
    ta->SetInterruptHandler([&](){
        PlatformAbstractionLayer::DigitalWrite(pinIsrA2, HIGH);

        // Do some user thing
        

        // This only goes off once, and gets re-enabled by the main overflow interrupt
        ta->DeRegisterForInterruptNonAtomic();

        PlatformAbstractionLayer::DigitalWrite(pinIsrA2, LOW);
    });
    ta->SetValue(d.aValueLast);
    ta->RegisterForInterrupt();

    ti->SetInterruptHandler([&](){
        PlatformAbstractionLayer::DigitalWrite(pinIsrA1, HIGH);


        
        --dp->overflowsRemaining;
        if (!dp->overflowsRemaining)
        {
            // Prepare for next cycle
            // Adjust next target ahead, 8-bit overflow on purpose
            dp->aValueLast += dp->tickIncrement;
            
            ta->SetValue(dp->aValueLast);
            ta->RegisterForInterruptNonAtomic();

            dp->overflowsRemaining = dp->overflowsExpected;
        }
        



        
        PlatformAbstractionLayer::DigitalWrite(pinIsrA1, LOW);
    });
    ti->RegisterForInterrupt();

    t.SetTimerPrescaler(Timer1::TimerPrescaler::DIV_BY_1);
    t.SetTimerMode(Timer1::TimerMode::FAST_PWM_8_BIT);
    t.SetTimerValue(0);
    
    t.StartTimer();

    TCCR0B &= 0b1111100;    // Disable Timer0
    while (1) {}            // Disable Evm
}


void loop() {}









