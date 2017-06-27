#include "PAL.h"
#include "Evm.h"

#include "Timer1.h"
#include "Timer2.h"


static Evm::Instance<10,10,10> evm;
static TimedEventHandlerDelegate ted;

static Timer1 t1;
static TimerChannel   *t1cA  = t1.GetTimerChannelA();
static TimerChannel   *t1cB  = t1.GetTimerChannelB();
static TimerInterrupt *t1Ovf = t1.GetTimerOverflowHandler();


static Timer2 t2;
static TimerChannel   *t2cA  = t2.GetTimerChannelA();
static TimerChannel   *t2cB  = t2.GetTimerChannelB();
static TimerInterrupt *t2Ovf = t2.GetTimerOverflowHandler();



void setup()
{
    Serial.begin(9600);

    
    ted.SetCallback([](){
        Serial.println("Looping again");
        
        TestTimer2SetFrequency();
    });
    ted.RegisterForTimedEventInterval(1000);

    
    evm.MainLoop();
}


void TestTimer2SetFrequency()
{
    TestTimer2SetFrequencyAt(8000);
    TestTimer2SetFrequencyAt(40000);
    TestTimer2SetFrequencyAt(44100);
}

void TestTimer2SetFrequencyAt(uint16_t frequency)
{
    // Stop Timer2 because in Arduino it seems to be running already
    // when the program starts.
    t2.StopTimer();
    
    // Calculate settings
    //TimerHelper<Timer2> th;
    //th.SetInterruptFrequency(frequency);

    TimerHelper<Timer2>{}.SetInterruptFrequency(frequency);

    // Set up an IO pin to toggle so events can be seen on scope
    Pin pinToggleOnInter(14, LOW);

    // Just to show running
    PAL.DigitalToggle(pinToggleOnInter);
    PAL.DigitalToggle(pinToggleOnInter);

    // Set up interrupt handler, this should go off at the given frequency
    volatile uint8_t count = 0;
    volatile uint8_t block = 1;
    
    t2cA->SetInterruptHandler([&](){
        PAL.DigitalToggle(pinToggleOnInter);

        ++count;

        if (count == 10)
        {
            t2.StopTimer();

            // Deregister here, because in the time taken during this
            // ISR, the loop could have gone around again and queued
            // another event.
            t2cA->DeRegisterForInterrupt();

            // unblock the main loop
            block = 0;
        }
    });
    t2cA->RegisterForInterrupt();


    // Actually start the timer
    t2.StartTimer();

    // Block, wait for interrupts to fire
    while (block) {}
}




void loop() {}



