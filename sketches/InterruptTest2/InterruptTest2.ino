#include <Evm.h>
#include <EvmEventHandlerUtils.h>

class ButtonEventHandler
{
public:
    virtual void OnButtonEvent(void *userData) = 0;
};

class ButtonEventMonitor
: private InterruptEventHandler
{
public:
    ButtonEventMonitor(uint8_t pin, ButtonEventHandler *beh, void *userData)
    : InterruptEventHandler(pin)
    , beh_(beh)
    , userData_(userData)
    {
        // nothing to do
    }

    void Start()
    {
        RegisterForInterruptEvent();
    }

    void Stop()
    {
        DeRegisterForInterruptEvent();
    }

private:
    virtual void OnInterruptEvent()
    {
        beh_->OnButtonEvent(userData_);
    }

    ButtonEventHandler *beh_;
    void               *userData_;
};


class ButtonResponder
: private ButtonEventHandler
{
public:
    ButtonResponder(uint8_t pinA, uint8_t pinB, uint8_t pinC, uint8_t pinSignal)
    : pinSignal_(pinSignal)
    {
        // Create button listeners, store some data to use when they fire
        buttonDataList_[0].bem_   = new ButtonEventMonitor(pinA, this, (void *)0);
        buttonDataList_[0].pin_   = pinA;
        buttonDataList_[0].count_ = 0;
        
        buttonDataList_[1].bem_   = new ButtonEventMonitor(pinB, this, (void *)1);
        buttonDataList_[1].pin_   = pinB;
        buttonDataList_[1].count_ = 0;
        
        buttonDataList_[2].bem_   = new ButtonEventMonitor(pinC, this, (void *)2);
        buttonDataList_[2].pin_   = pinC;
        buttonDataList_[2].count_ = 0;

        // Start them
        buttonDataList_[0].bem_->Start();
        buttonDataList_[1].bem_->Start();
        buttonDataList_[2].bem_->Start();

        // Set up signalling pin
        PAL.PinMode(pinSignal_, OUTPUT);
    }

    ~ButtonResponder()
    {
        delete buttonDataList_[0].bem_;
        delete buttonDataList_[1].bem_;
        delete buttonDataList_[2].bem_;
    }

private:
    struct ButtonData
    {
        ButtonEventMonitor *bem_;
        uint8_t             pin_;
        uint8_t             count_;
    };

    virtual void OnButtonEvent(void *userData)
    {
        uint8_t buttonDataListIdx = (uint8_t)(uint16_t)userData;

        ButtonData &bd = buttonDataList_[buttonDataListIdx];

        // Update stats
        ++bd.count_;

        // Signal the pin number
        for (uint8_t i = 0; i < bd.pin_; ++i)
        {
            PAL.DigitalWrite(pinSignal_, HIGH);
            PAL.DigitalWrite(pinSignal_, LOW);
        }

        PAL.Delay(1);

        // Signal the number of times you've seen it
        for (uint8_t i = 0; i < bd.count_; ++i)
        {
            PAL.DigitalWrite(pinSignal_, HIGH);
            PAL.DigitalWrite(pinSignal_, LOW);
        }
    }

    ButtonData buttonDataList_[3];
    uint8_t    pinSignal_;
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
    
    Evm &evm = Evm::GetInstance();

    // The usual actors
    IdlePinToggler ipt(PIN_IDLE_TOGGLER); ipt.RegisterForIdleTimeEvent();
    TimedPinToggler tpt(PIN_TIMED_TOGGLER); tpt.RegisterForTimedEventInterval(1000);

    // Monitor buttons
    ButtonResponder br(PIN_BUTTON_1,
                       PIN_BUTTON_2,
                       PIN_BUTTON_3,
                       PIN_SIGNAL);

    evm.MainLoop();
}



void setup()
{
}

