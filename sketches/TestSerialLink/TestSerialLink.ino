#include <Evm.h>
#include <TimedEventHandler.h>
#include <SerialLink.h>
#include <Utl.h>


class TestSerialLink
: public TimedEventHandler
{
public:
    TestSerialLink() { }
    ~TestSerialLink() { }

    void Run()
    {
        serialLink_.Init(this, &TestSerialLink::OnSerialRxAvailable);

        RegisterForTimedEventInterval(1000);

        evm_.MainLoop();
    }


private:
    Evm::Instance<10,10,10> evm_;

private:

    void OnTimedEvent()
    {
        static uint8_t count = 0;

        PinToggle(5, 100);
        
        serialLink_.Send(1, &count, 1);

        ++count;
    }

    void
    OnSerialRxAvailable(SerialLink<TestSerialLink>::Header *hdr,
                        uint8_t *buf,
                        uint8_t  bufSize)
    {
        // ping pong
        PinToggle(4, 100);
        serialLink_.Send(hdr->protocolId, buf, bufSize);
    }

    SerialLink<TestSerialLink> serialLink_;
};


static TestSerialLink app;


void setup()
{
    app.Run();
}

void loop() { }



