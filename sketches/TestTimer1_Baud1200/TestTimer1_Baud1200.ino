#include "PAL.h"
#include "Timer1.h"
#include "Evm.h"


static Evm::Instance<10,10,10> evm;

static Timer1          t;
static TimerChannel   *ta = NULL;
static TimerChannel   *tb = NULL;
static TimerInterrupt *ti = NULL;

static const uint8_t PIN_DEBUG_A = 27;
static const uint8_t PIN_DEBUG_B = 28;

Pin pinDebugA(PIN_DEBUG_A, LOW);
Pin pinDebugB(PIN_DEBUG_B, LOW);


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
    Serial.println();
    Serial.print("FUDGE_TIME_US       : ");
    Serial.println(FUDGE_TIME_US);
    Serial.print("SCALING_FACTOR      : ");
    Serial.println(SCALING_FACTOR);
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
    Serial.print("top                 : ");
    Serial.println(top);
    Serial.println();

    delay(1000);

    // Set up timer
    t.SetTimerPrescaler(Timer1::TimerPrescaler::DIV_BY_1);
    t.SetTimerMode(Timer1::TimerMode::FAST_PWM_TOP_OCRNA);
    t.SetTimerValue(0);

    // Set up channel
    ta->SetInterruptHandler([&](){
        PlatformAbstractionLayer::DigitalWrite(pinDebugA, HIGH);

        // Do some user thing

        PlatformAbstractionLayer::DigitalWrite(pinDebugA, LOW);
    });
    ta->SetValue(top);
    ta->RegisterForInterrupt();
    
    // Begin
    t.StartTimer();

    PAL.PowerDownTimer0();
    while (1) {}            // Disable Evm
}


void loop() {}









