#include <Evm.h>
#include <Function.h>
#include <TimedEventHandler.h>
#include <SerialLink.h>
#include <Utl.h>


class TestSerialLink
: public TimedEventHandler
{
public:
    TestSerialLink()
    : pinOnRxAvailable_(4, LOW)
    , pinOnTimedEvent_(5, LOW)
    {
    }
    ~TestSerialLink() { }

    void Run()
    {
        serialLink_.Init([this](SerialLinkHeader *hdr,
                                uint8_t          *buf,
                                uint8_t           bufSize){
            OnSerialRxAvailable(hdr, buf, bufSize);
        });

        RegisterForTimedEventInterval(1000);

        evm_.MainLoop();
    }


private:
    Evm::Instance<10,10,10> evm_;

private:

    void OnTimedEvent()
    {
        static uint8_t count = 0;

        PAL.DigitalWrite(pinOnTimedEvent_, HIGH);
        
        serialLink_.Send(1, &count, 1);

        PAL.DigitalWrite(pinOnTimedEvent_, LOW);

        ++count;
    }

    void
    OnSerialRxAvailable(SerialLinkHeader *hdr,
                        uint8_t          *buf,
                        uint8_t           bufSize)
    {
        // ping pong
        PAL.DigitalWrite(pinOnRxAvailable_, HIGH);
        
        serialLink_.Send(hdr->protocolId, buf, bufSize);
        
        PAL.DigitalWrite(pinOnRxAvailable_, LOW);
    }

    SerialLink serialLink_;

    Pin pinOnRxAvailable_;
    Pin pinOnTimedEvent_;
};


static TestSerialLink app;


void setup()
{
    app.Run();
}

void loop() { }



