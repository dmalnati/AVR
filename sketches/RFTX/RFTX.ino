#include <Evm.h>
#include <EvmCallback.h>
#include <MyRadioController.h>




class MyTimedSender
: private TimedCallback
, public MyRadioControllerTxCallbackIface
{
public:
    MyTimedSender(int8_t pin, uint16_t interval)
    : rc_(-1, NULL, pin, this)
    {
        digitalWrite(3, HIGH);
        ScheduleInterval(interval);
        digitalWrite(3, LOW);
    }

private:
    constexpr static const char *MSG = "TEXT";

    // Implement the TimedCallback interface
    virtual void OnCallback()
    {
        digitalWrite(3, HIGH);
        rc_.Send((uint8_t *)MSG, strlen(MSG));
    }

    // Implement the MyRadioControllerCallbackIface callback
    virtual void OnTxComplete()
    {
        digitalWrite(3, LOW);
    }

    MyRadioController rc_;
};



class TimedPinToggler : public TimedCallback
{
public:
    TimedPinToggler(uint8_t pin) : pin_(pin) { pinMode(pin_, OUTPUT); }
    
    void OnCallback()
    {
        digitalWrite(pin_, HIGH);
        digitalWrite(pin_, LOW);
    }

private:
    uint8_t pin_;
};


// This sketch doesn't work on ATtiny85 since it seems that
// VirtualWire takes over Timer0 on that platform (only)
// (despite saying Timer1, which is accurate elsewhere.)
//
// As a result, use of micros()/millis() doesn't work.
// That affects:
// - Evm's TimedEvents (but not IdleEvents)
// - LedFader, etc
//
void loop()
{
    Evm &evm = Evm::GetInstance();

    // Just enable and toggle some bits to make debugging easier
    // both to show where the start of the program is but also
    // to set up some pins for signaling elsewhere deeper in libs.
    delay(200);
    for (uint8_t i = 0; i < 5; ++i)
    {
        pinMode(i, OUTPUT);
    }
    delay(200);
    for (uint8_t i = 0; i < 5; ++i)
    {
        digitalWrite(i, LOW);  delay(1);
        digitalWrite(i, HIGH); delay(1);
        digitalWrite(i, LOW);  delay(1);
    }
    delay(200);

    // Invoke, it tries to send periodically.
    const uint8_t PIN_RF_TX = 2;
    const uint16_t SEND_INTERVAL = 1000;
    MyTimedSender mts(PIN_RF_TX, SEND_INTERVAL);

    // Run this guy just to keep track of how events unfold
    // during transmission.  AKA make sure things don't get
    // blocked.
    TimedPinToggler tpt1(1) ; tpt1.ScheduleInterval(100);

    evm.MainLoop();
}








void setup()
{
    // nothing to do
}
















