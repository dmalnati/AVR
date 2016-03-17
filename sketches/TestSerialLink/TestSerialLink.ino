#include <Evm.h>
#include <SerialLink.h>

class TestSerialLink
{
public:
    TestSerialLink() { }
    ~TestSerialLink() { }

    void Run()
    {
        serialLink_.Init(this, &TestSerialLink::OnSerialRxAvailable);

        evm_.MainLoop();
    }


private:
    Evm::Instance<10,10,10> evm_;

private:
    void
    OnSerialRxAvailable(SerialLink<TestSerialLink>::Header *hdr,
                        uint8_t *buf,
                        uint8_t  bufSize)
    {
        // ping pong
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



