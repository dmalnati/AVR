
#include <Evm.h>
#include <EvmEventHandlerUtils.h>


// may not live here in the end
#include <InterruptEventHandler.h>


// expressed in Arduino pins
const uint8_t PIN_BLOCKING       = 0;   // physical pin  2
const uint8_t PIN_TIMED_INTERVAL = 1;   // physical pin  3
const uint8_t PIN_INT_INPUT_RAW  = 2;   // physical pin  4
const uint8_t PIN_INT_OUTPUT_RAW = 3;   // physical pin  5

// expressed in physical pins

// Testing for individual ports
//const uint8_t PIN_INT_INPUT_EVM = 6;    // Arduino pin 4    (PD4)
//const uint8_t PIN_INT_INPUT_EVM = 28;    // Arduino pin 4    (PC5)
//const uint8_t PIN_INT_INPUT_EVM = 9;    // Arduino pin 4    (PB6)

// Testing for double port registration
//const uint8_t PIN_INT_INPUT_EVM  = 4;   // (PD2)
//const uint8_t PIN_INT_INPUT_EVM2 = 9;   // (PB6)

// Testing for deregistration
//const uint8_t PIN_INT_INPUT_EVM  = 4;   // (PD2)

// Testing for false trigger
const uint8_t PIN_INT_INPUT_EVM  = 4;   // (PD2)


// expressed in Arduino pins
const uint8_t PIN_INT_OUTPUT_EVM = 5;   // physical pin 11



// A function which will block the main thread for long periods of time
void BlockingFunction(void *)
{
    pinMode(PIN_BLOCKING, OUTPUT);

    digitalWrite(PIN_BLOCKING, HIGH);
    delay(750),
    digitalWrite(PIN_BLOCKING, LOW);
}


// Interrupt handler
void INT()
{
    static TimedPinToggler tpt(PIN_INT_OUTPUT_RAW);

    digitalWrite(PIN_INT_OUTPUT_RAW, HIGH);
    digitalWrite(PIN_INT_OUTPUT_RAW, LOW);

    tpt.RegisterForTimedEvent(50);
}


class MyISR
: public InterruptEventHandler
{
public:
    MyISR(uint8_t inPin, MODE mode, uint8_t outPin)
    : InterruptEventHandler(inPin, mode)
    , outPin_(outPin)
    {
        pinMode(outPin_, OUTPUT);

        digitalWrite(outPin_, HIGH);
        digitalWrite(outPin_, LOW);
    }

private:
    void OnInterruptEvent()
    {
        digitalWrite(outPin_, HIGH);
        digitalWrite(outPin_, LOW);
        digitalWrite(outPin_, HIGH);
        digitalWrite(outPin_, LOW);
    }

    uint8_t outPin_;
};

class TimedDeregReg
: public TimedEventHandler
{
public:
    TimedDeregReg(MyISR *isr) : isr_(isr), onOff_(1) { }

    // Implement timeout event
    virtual void OnTimedEvent()
    {
        if (onOff_)
        {
            isr_->DeRegisterForInterruptEvent();

            onOff_ = 0;
        }
        else
        {
            isr_->RegisterForInterruptEvent();

            onOff_ = 1;
        }
    }

private:
    MyISR   *isr_;
    uint8_t  onOff_;
};


void loop()
{
    Evm &evm = Evm::GetInstance();

    // As usual, see the effect on the main thread
    TimedPinToggler tpt(PIN_TIMED_INTERVAL) ; tpt.RegisterForTimedEventInterval(1000);

    // A blocking function to run for long periods, periodically
    TimedEventHandlerFnWrapper bf(BlockingFunction, NULL) ; bf.RegisterForTimedEventInterval(1000);

    // Test raw interrupt handler
    //attachInterrupt(digitalPinToInterrupt(PIN_INT_INPUT_RAW), INT, RISING);

    // Test evm interrupt handler
    MyISR myIsr(PIN_INT_INPUT_EVM,
                InterruptEventHandler::MODE::MODE_RISING,
                PIN_INT_OUTPUT_EVM);
    myIsr.RegisterForInterruptEvent();


    evm.MainLoop();
}



/*

Tests done:
- registration on a pin on each port works (yes)
- intr fires only when its pin is touched (yes)
  - register on a pin on a port
  - toggle two pins on that port


- reg on two ports works (yes)

- double reg same pin doesn't work (yes)

- dereg works (yes)
  - ref counters go back to zero

- re-enable works (yes)

- registering for a transition to high for a pin already high
  doesn't trigger an interrupt.

  Unsure.  Had a hard time making it happen.  I tried and it didn't
  seem to trigger.  However, not sure how saving the initial state
  of the PIN registers won't lead to it.

  If an issue, I'd think I'd consider:
  - On initial use of a port, clear any interrupts
  - On initial use of a pin
    - set the cached "last" state to be the current value.
    - clear interrupts for this pin which are pending

*/


void setup()
{
    // nothing to do
}
