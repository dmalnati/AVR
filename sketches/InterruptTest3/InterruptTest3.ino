#include <Evm.h>
#include <Utl.h>



// can/should send back userData
// can/should send back pinState which triggered event



class ButtonResponder
{
public:
    ButtonResponder(uint8_t pinA, uint8_t pinB, uint8_t pinC, uint8_t pinSignal)
    : pinSignal_(pinSignal)
    , pinA_(pinA)
    , pinB_(pinB)
    , pinC_(pinC)
    {
        // Assign interrupts to handler functions
        MapAndStartInterrupt(pinA_, this, &ButtonResponder::OnButtonA, LEVEL_RISING);
        MapAndStartInterrupt(pinB_, this, &ButtonResponder::OnButtonB, LEVEL_FALLING);
        MapAndStartInterrupt(pinC_, this, &ButtonResponder::OnButtonC, LEVEL_RISING_AND_FALLING);

        PAL.PinMode(pinSignal_, OUTPUT);
    }

private:
    void TogglePattern(uint8_t countFirst, uint8_t countSecond, uint8_t countThird)
    {
        // Signal the pin number
        for (uint8_t i = 0; i < countFirst; ++i)
        {
            PAL.DigitalWrite(pinSignal_, HIGH);
            PAL.DigitalWrite(pinSignal_, LOW);
        }

        PAL.Delay(1);

        // Signal the number of times you've seen it
        for (uint8_t i = 0; i < countSecond; ++i)
        {
            PAL.DigitalWrite(pinSignal_, HIGH);
            PAL.DigitalWrite(pinSignal_, LOW);
        }
        
        PAL.Delay(1);

        // Signal the number of times you've seen it
        for (uint8_t i = 0; i < countThird; ++i)
        {
            PAL.DigitalWrite(pinSignal_, HIGH);
            PAL.DigitalWrite(pinSignal_, LOW);
        }
    }

    void OnButtonA(uint8_t pinLevel)
    {
        static uint8_t count = 0;
        
        // Update stats
        ++count;

        // Signal
        TogglePattern(pinA_, pinLevel + 1, count);
    }

    void OnButtonB(uint8_t pinLevel)
    {
        static uint8_t count = 0;
        
        // Update stats
        ++count;

        // Signal
        TogglePattern(pinB_, pinLevel + 1, count);
    }
    
    void OnButtonC(uint8_t pinLevel)
    {
        static uint8_t count = 0;
        
        // Update stats
        ++count;

        // Signal
        TogglePattern(pinC_, pinLevel + 1, count);
    }

    uint8_t pinA_;
    uint8_t pinB_;
    uint8_t pinC_;
    uint8_t pinSignal_;
};


void loop()
{
    // Physical pins
    const uint8_t PIN_IDLE_TOGGLER  = 2;
    const uint8_t PIN_TIMED_TOGGLER = 3;
    const uint8_t PIN_BUTTON_1      = 4;
    const uint8_t PIN_BUTTON_2      = 5;
    const uint8_t PIN_BUTTON_3      = 6;
    const uint8_t PIN_SIGNAL        = 11;
    
    // The usual actors
    IdlePinToggler ipt(PIN_IDLE_TOGGLER); ipt.RegisterForIdleTimeEvent();
    TimedPinToggler tpt(PIN_TIMED_TOGGLER); tpt.RegisterForTimedEventInterval(1000);

    // Monitor buttons
    ButtonResponder br(PIN_BUTTON_1,
                       PIN_BUTTON_2,
                       PIN_BUTTON_3,
                       PIN_SIGNAL);

    Evm::GetInstance().MainLoop();
}



void setup()
{
}

