#include <PAL.h>
#include <PCIntEventHandler.h>


class TogglerOnInter
: private PCIntEventHandler
{
public:
    TogglerOnInter(uint8_t pinInter, uint8_t pinToggle)
    : PCIntEventHandler(pinInter)
    , pinToggle_(pinToggle)
    {
        Start();
    }

    void Start()
    {
        RegisterForPCIntEvent();
    }

private:

    virtual void OnPCIntEvent(uint8_t logicLevel)
    {
        PAL.PinMode(pinToggle_, OUTPUT);

        PAL.DigitalWrite(pinToggle_, HIGH);
        PAL.Delay(1000);    // this doesn't work in the raw ISR world
        PAL.DigitalWrite(pinToggle_, LOW);
    }

    uint8_t pinToggle_;
};


void loop()
{
    // Clear button, No LED
    TogglerOnInter toi1(27, 5);

    // No button, Yes LED
    TogglerOnInter toi2(4, 11);


    while (1) { }
}



void setup()
{

}

