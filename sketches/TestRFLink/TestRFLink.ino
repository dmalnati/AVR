#include "Evm.h"
#include "RFLink.h"


class TestRFLink
{
public:

    void Run()
    {
        uint8_t realm   = 1;
        uint8_t srcAddr = 1;
        int8_t  rxPin   = 1;
        int8_t  txPin   = 1;
        
        rfLink_.Init(realm, srcAddr, this, rxPin, &TestRFLink::OnRxAvailable, txPin);
        
        evm_.MainLoop();
    }

private:
    void OnRxAvailable(uint8_t /*srcAddr*/,
                       uint8_t /*protocolId*/,
                       uint8_t */*buf*/,
                       uint8_t /*bufSize*/)
    {
        // nothing to do
    }

    Evm::Instance<10,10,10> evm_;

    RFLink<TestRFLink> rfLink_;
};

static TestRFLink app;


void setup() 
{
    app.Run();
}

void loop() {}
