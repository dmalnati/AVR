#include <Evm.h>
#include <PCIntEventHandler.h>


class TogglerOnIvmInter
: private PCIntEventHandler
{
public:
    TogglerOnIvmInter(uint8_t pinInter, uint8_t pinToggle)
    : pinToggle_(pinToggle)
    {
        Start(pinInter);
    }

    void Start(uint8_t pin)
    {
        PAL.PinMode(pinToggle_, OUTPUT);
        RegisterForPCIntEvent(pin);
    }

private:

    virtual void OnPCIntEvent(uint8_t)
    {
        PAL.DigitalWrite(pinToggle_, HIGH);
        PAL.Delay(1000);    // this doesn't work in the raw ISR world
        PAL.DigitalWrite(pinToggle_, LOW);
    }

    uint8_t pinToggle_;
};


class TogglerOnEvmInter
: private InterruptEventHandler
{
public:
    TogglerOnEvmInter(uint8_t pinInter, uint8_t pinToggle)
    : pinToggle_(pinToggle)
    {
        Start(pinInter);
    }

    void Start(uint8_t pin)
    {
        PAL.PinMode(pinToggle_, OUTPUT);
        RegisterForInterruptEvent(pin);
    }

private:

    virtual void OnInterruptEvent(uint8_t)
    {
        PAL.DigitalWrite(pinToggle_, HIGH);
        PAL.Delay(3000);    // this should work in the Evm ISR world
        PAL.DigitalWrite(pinToggle_, LOW);
    }

    uint8_t pinToggle_;
};





static const uint8_t C_IDLE  = 0;
static const uint8_t C_TIMED = 0;
static const uint8_t C_INTER = 2;

static Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm;

void loop()
{
    // Clear button, No LED
    TogglerOnIvmInter toi1(27, 5);

    // No button, Yes LED
    TogglerOnIvmInter toi2(4, 11);

    // Yes button, FTT LED
    TogglerOnEvmInter toe1(6, 12);

    // FTT button, Attention Blue LED
    TogglerOnEvmInter toe2(13, 17);


    evm.MainLoop();
}



void setup()
{

}

