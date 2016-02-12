
#include <Evm.h>
#include <EvmEventHandlerUtils.h>


// may not live here in the end
#include <InterruptEventHandler.h>


const uint8_t PIN_BLOCKING       = 0;
const uint8_t PIN_TIMED_INTERVAL = 1;
const uint8_t PIN_INT_INPUT_RAW  = 2;
const uint8_t PIN_INT_OUTPUT_RAW = 3;
const uint8_t PIN_INT_INPUT_EVM  = 4;
const uint8_t PIN_INT_OUTPUT_EVM = PD5;



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
    MyISR(uint8_t pin, uint8_t mode, uint8_t outPin)
    : InterruptEventHandler(pin, mode)
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
    }

    uint8_t outPin_;
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
    //MyISR myIsr(PIN_INT_INPUT_EVM, RISING, PIN_INT_OUTPUT_EVM); myIsr.RegisterForInterruptEvent();
    MyISR myIsr(PIN_INT_INPUT_RAW, RISING, PIN_INT_OUTPUT_EVM); myIsr.RegisterForInterruptEvent();


    evm.MainLoop();
}








void setup()
{
    // nothing to do
}
