#include <Container.h>
#include <Evm.h>
#include <TimedEventHandler.h>
#include <IdleTimeEventHandler.h>
#include <InterruptEventHandler.h>


class MyTimedEvent
: public TimedEventHandler
{
public:
    MyTimedEvent(uint8_t pin) : pin_(pin) { }

private:
    virtual void OnTimedEvent()
    {
        PAL.PinMode(pin_, OUTPUT);

        PAL.DigitalWrite(pin_, HIGH);
        PAL.Delay(500);
        PAL.DigitalWrite(pin_, LOW);
    }

    uint8_t pin_;
};




class MyIdleEvent
: public IdleTimeEventHandler
{
public:
    MyIdleEvent(uint8_t pin) : pin_(pin) { }

private:
    virtual void OnIdleTimeEvent()
    {
        PAL.PinMode(pin_, OUTPUT);

        PAL.DigitalWrite(pin_, HIGH);
        PAL.DigitalWrite(pin_, LOW);
    }

    uint8_t pin_;
};



class MyPCInterEvent
: public InterruptEventHandler
{
public:
    MyPCInterEvent(uint8_t pinInter, uint8_t pinToggle)
    : InterruptEventHandler(pinInter)
    , pinToggle_(pinToggle)
    {
        PAL.PinMode(pinToggle_, OUTPUT);
    }

    void OnInterruptEvent(uint8_t logicLevel)
    {
        PAL.DigitalWrite(pinToggle_, HIGH);
        PAL.Delay(500);
        PAL.DigitalWrite(pinToggle_, LOW);

    }

private:
    uint8_t pinToggle_;
};



// This app would normally live in a separate file.
// Testing purposes only.
class App
{
public:
    App()
    : evm_(Evm::CreateInstance<C_IDLE, C_TIMED, C_INTER>())
    , mte1_(5)  // No LED
    , mte2_(11) // Yes LED
    , mie1_(15) // Attention Red LED
    , mie2_(17) // Attention Blue LED
    , mpe1_(27, 5) // Watch Clear button, toggle No LED
    , mpe2_(4, 17) // Watch No button, toggle Attention Blue LED
    {
        // Nothing to do

        // Yes LED on error
        if (!mte1_.RegisterForTimedEventInterval(1000))
        {
            PAL.PinMode(11, OUTPUT);
            PAL.DigitalWrite(11, HIGH);
            //PAL.Delay(500);   // can't do this when app is static, I guess Arduino hasn't started Timer0 yet...
            PAL.DigitalWrite(11, LOW);
        }

        // FTT LED on error
        if (!mte2_.RegisterForTimedEventInterval(1000))
        {
            PAL.PinMode(12, OUTPUT);
            PAL.DigitalWrite(12, HIGH);
            //PAL.Delay(500);
            PAL.DigitalWrite(12, LOW);
        }


        PAL.PinMode(11, OUTPUT);
        PAL.DigitalWrite(11, HIGH);

        // FTT LED on error
        if (!mie1_.RegisterForIdleTimeEvent())
        {
            PAL.PinMode(12, OUTPUT);
            PAL.DigitalWrite(12, HIGH);
            //PAL.Delay(500);
            PAL.DigitalWrite(12, LOW);
        }

        // FTT LED on error
        if (!mie2_.RegisterForIdleTimeEvent())
        {
            PAL.PinMode(12, OUTPUT);
            PAL.DigitalWrite(12, HIGH);
            PAL.Delay(500);
            PAL.DigitalWrite(12, LOW);
        }
        

        PAL.DigitalWrite(11, LOW);



        // FTT LED on error
        if (!mpe1_.RegisterForInterruptEvent())
        {
            PAL.PinMode(12, OUTPUT);
            PAL.DigitalWrite(12, HIGH);
            //PAL.Delay(500);
            PAL.DigitalWrite(12, LOW);
        }
        
        // FTT LED on error
        if (!mpe2_.RegisterForInterruptEvent())
        {
            PAL.PinMode(12, OUTPUT);
            PAL.DigitalWrite(12, HIGH);
            //PAL.Delay(500);
            PAL.DigitalWrite(12, LOW);
        }



    }
 
    void Run()
    {
        evm_.MainLoop();
    }


    // Accounting work
    static const uint8_t C_IDLE = 
        1 + // mte1_
        1;  // mte2_
    static const uint8_t C_TIMED =
        1 + // mie1_
        1;  // mie2_
    static const uint8_t C_INTER = 
        0 + // mpe1_
        0;  // mpe2_

        // interestingly the above works for the interrupt timer count.
        // - if 0, then the container in Evm defaults to size 1.
        // - however, we instantiate two interrupt handlers here.
        //   - but, they actually are placed in Ivm memory, not Evm
        //     - and, when one fires, it is transferred to Evm,
        //       but only one of them at a time given how quick they are
        //     - during that time, the one queued event is occupying
        //       the 0-becomes-1 default container value.

        
    
private:
    Evm &evm_;



    MyTimedEvent mte1_;
    MyTimedEvent mte2_;

    MyIdleEvent mie1_;
    MyIdleEvent mie2_;

    MyPCInterEvent mpe1_;
    MyPCInterEvent mpe2_;

    RingBuffer<void *, 25> rb_;




    




};
 




//////////////////////////////
//
// Testing placement new
//
//////////////////////////////

class Blinker
{
public:
    Blinker(uint8_t pin)
    {
        PAL.PinMode(pin, OUTPUT);

        PAL.DigitalWrite(pin, HIGH);
        PAL.Delay(500);
        PAL.DigitalWrite(pin, LOW);
    }
};


#if 1
void* operator new(size_t, void* const buf){
  return buf;
}
#endif



void TestPN2()
{
    
    char buf[sizeof(Blinker)];
    //Blinker *ptr = static_cast<Blinker *>((void *)buf);   // this works
    Blinker *ptr = (Blinker *)buf;

    new(ptr)Blinker(5);
}

static App app;



void loop()
{
    TestPN2();
    
    app.Run();
}


/*

Are templates necessary in order to get predefined sram sizing?

In c++, when you use a variable to size an array, where does that go?
- doesn't work on constructor arguments
  - except maybe constexpr ones?
    - not sure how that all works.

- plus what is the story with placement new?
  - can this help?
  - ie, I know I can get the number of events necessary for Evm to handle up
    front.
    - can I pass them as arguments to Evm?
      - if yes, then
        - Templated Evm everywhere goes away
        - it becomes a race to get to Evm first
          - do I automatically win if app is the only object which has children, and
            I have a reference to Evm as my first member and initilizer?

*/




void setup()
{
}















