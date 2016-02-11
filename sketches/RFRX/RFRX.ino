#include <Evm.h>
#include <EvmEventHandler.h>
#include <MyRadioController.h>




class MyTimedRadioReceiver
: public MyRadioControllerRxCallbackIface
{
public:
    MyTimedRadioReceiver(int8_t pin)
    : rc_(pin, this, -1, NULL)
    {
        digitalWrite(3, HIGH);
        digitalWrite(3, LOW);
    }

private:
    // Implement the MyRadioControllerCallbackIface callback
    virtual void OnRxAvailable(uint8_t *buf, uint8_t bufSize)
    {
        uint8_t msgAsCount = *buf;
        
        for (uint8_t i = 0; i < msgAsCount; ++i)
        {
            digitalWrite(3, HIGH);
            digitalWrite(3, LOW);
        }

        // really light the LED for a visible period of time
        digitalWrite(3, HIGH);
        delay(500);
        digitalWrite(3, LOW);
    }

    MyRadioController rc_;
};



class TimedPinToggler : public TimedEventHandler
{
public:
    TimedPinToggler(uint8_t pin) : pin_(pin) { pinMode(pin_, OUTPUT); }
    
    void OnTimedEvent()
    {
        digitalWrite(pin_, HIGH);
        digitalWrite(pin_, LOW);
    }

private:
    uint8_t pin_;
};


void loop()
{
    Evm &evm = Evm::GetInstance();

    // hold down pins we don't care about for salea purposes
    pinMode(3, OUTPUT); digitalWrite(3, LOW);
    pinMode(4, OUTPUT); digitalWrite(4, LOW);

    // set up some debug pins
    pinMode(0, OUTPUT);
    pinMode(1, OUTPUT);

    // deal with RX pin?  set low?
    //pinMode(2, INPUT_PULLDOWN);


    // Set up radio listener
    const uint8_t PIN_RF_RX = 2;
    MyTimedRadioReceiver mts(PIN_RF_RX);

    // Just an idle event to be sure Evm working properly
    TimedPinToggler tpt1(1) ; tpt1.RegisterForTimedEventInterval(100);

    
    evm.MainLoop();
}









void setup()
{
    // nothing to do
}












