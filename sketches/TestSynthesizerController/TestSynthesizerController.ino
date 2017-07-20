#include "Evm.h"

#include "Synthesizer.h"
#include "SynthesizerVoiceSerialInterface.h"


class Controller
{
public:
    Controller()
    : svsi_(&s_)
    {
        // Nothing to do
    }

    void Start()
    {
        Serial.begin(9600);

        Serial.println("Starting");

        // Start up serial controller
        svsi_.Init();

        // Set up and start synthesizer
        s_.Init();
        s_.Start();

        // Start the events
        evm_.MainLoop();
    }

private:

    Evm::Instance<10,10,10> evm_;

    Synthesizer s_;
    SynthesizerVoiceSerialInterface svsi_;
};

static Controller c;


void setup()
{
    c.Start();
}


void loop() {}


















