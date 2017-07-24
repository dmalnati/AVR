#include "Evm.h"

#include "MIDICommandFromSerial.h"


class Test
{
public:
    void Start()
    {
        Serial.begin(31250);

        Serial.println("Starting");

        // Set up callbacks
        mcfs_.SetCallbackOnMIDICommand([this](MIDICommand c){
            OnMIDICommand(c);
        });

        // Begin listening
        mcfs_.Start();

        // Start the events
        evm_.MainLoop();
    }

private:

    void OnMIDICommand(MIDICommand c)
    {
        Serial.print("MIDICommand: ");
        Serial.print("type(");    Serial.print((uint8_t)c.type); Serial.print("), ");
        Serial.print("channel("); Serial.print(c.channel);       Serial.print("), ");
        Serial.print("param1(");  Serial.print(c.param1);        Serial.print("), ");
        Serial.print("param2(");  Serial.print(c.param2);        Serial.print(")");
        Serial.println();
    }

    Evm::Instance<10,10,10> evm_;

    MIDICommandFromSerial mcfs_;
};

static Test t;


void setup()
{
    t.Start();
}


void loop() {}


















