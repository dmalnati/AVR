#include <PAL.h>
#include <Container.h>


class Blinker1
{
public:
    Blinker1(uint8_t pin1)
    : pin1_(pin1)
    {
        Start();
    }

    ~Blinker1()
    {
        Stop();
    }

    void Start()
    {
        PAL.PinMode(pin1_, OUTPUT);

        for (uint8_t i = 0; i < 2; ++i)
        {
            PAL.DigitalWrite(pin1_, HIGH);
            PAL.Delay(300);
            PAL.DigitalWrite(pin1_, LOW);
            PAL.Delay(300);
        }
    }

    void Stop()
    {
        for (uint8_t i = 0; i < 5; ++i)
        {
            PAL.DigitalWrite(pin1_, HIGH);
            PAL.Delay(100);
            PAL.DigitalWrite(pin1_, LOW);
            PAL.Delay(100);
        }
    }

    uint8_t pin1_;
};

class Blinker2
{
public:
    Blinker2(uint8_t pin1, uint8_t pin2)
    : pin1_(pin1)
    , pin2_(pin2)
    {
        Start();
    }

    ~Blinker2()
    {
        Stop();
    }

    void Start()
    {
        PAL.PinMode(pin1_, OUTPUT);
        PAL.PinMode(pin2_, OUTPUT);

        for (uint8_t i = 0; i < 2; ++i)
        {
            PAL.DigitalWrite(pin1_, HIGH);
            PAL.Delay(300);
            PAL.DigitalWrite(pin1_, LOW);
            PAL.Delay(300);
        }
            
        for (uint8_t i = 0; i < 2; ++i)
        {
            PAL.DigitalWrite(pin2_, HIGH);
            PAL.Delay(300);
            PAL.DigitalWrite(pin2_, LOW);
            PAL.Delay(300);
        }
    }

    void Stop()
    {
        for (uint8_t i = 0; i < 5; ++i)
        {
            PAL.DigitalWrite(pin1_, HIGH);
            PAL.DigitalWrite(pin2_, HIGH);
            PAL.Delay(100);
            PAL.DigitalWrite(pin1_, LOW);
            PAL.DigitalWrite(pin2_, LOW);
            PAL.Delay(100);
        }
    }

    uint8_t pin1_;
    uint8_t pin2_;
};


class PinHolder
{
public:
    PinHolder(uint8_t pin) : pin_(pin) { }

    uint8_t GetPin() const { return pin_; }

private:
    uint8_t pin_;
};

class Blinker2WithObjects
: private Blinker2
{
public:
    Blinker2WithObjects(PinHolder &phLed1, uint8_t led2)
    : Blinker2(phLed1.GetPin(), led2)
    {
        // Nothing to do
    }
};


void TestPlacementNewDirectly(uint8_t led1, uint8_t led2)
{
    // This works.  Blinks each LED twice each on Construction, again on Start().
    uint8_t buf[sizeof(Blinker2)];
    new ((void *)buf) Blinker2(led1, led2);
    ((Blinker2 *)buf)->~Blinker2();
}

void TestBlinker1(uint8_t pin)
{
    ListInPlace<Blinker1, 1> lip;

    lip.PushNew(pin);
}

void TestBlinker2(uint8_t pin1, uint8_t pin2)
{
    ListInPlace<Blinker2, 1> lip;

    lip.PushNew(pin1, pin2);
}

void TestBlinker2WithObjects(PinHolder &phLed1, uint8_t led2)
{
    ListInPlace<Blinker2WithObjects, 1> lip;

    lip.PushNew(phLed1, led2);
}


void loop()
{
    uint8_t ledNo    = 5;   // No LED
    uint8_t ledYes   = 11;  // Yes LED
    uint8_t ledFTT   = 12;  // FTT LED
    uint8_t ledABlue = 17;  // Attention Blue LED

    

    //TestPlacementNewDirectly(ledNo, ledYes);
    //TestBlinker1(ledFTT);
    //TestBlinker1(ledABlue);
    //TestBlinker2(ledNo, ledYes);

    PinHolder phLedNo(ledNo);
    TestBlinker2WithObjects(phLedNo, ledYes);


    while (1) { }
}




void setup()
{
}

