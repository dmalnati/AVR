#include "Evm.h"

#include "Synthesizer.h"
#include "SynthesizerSerialInterface.h"


class Controller
{
public:
    Controller()
    : ssi_(&s_)
    {
        // Nothing to do
    }

    void Start()
    {
        @fix@Serial.begin(9600);

        @fix@Serial.println("Starting");

        // Start up serial controller
        ssi_.Init();

        // Set up and start synthesizer
        s_.Init();
        s_.Start();

        // Start the events
        evm_.MainLoop();
    }

private:

    Evm::Instance<10,10,10> evm_;

    Synthesizer s_;
    SynthesizerSerialInterface ssi_;
};

static Controller c;


void setup()
{
    c.Start();
}


void loop() {}


















