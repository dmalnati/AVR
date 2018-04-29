#include "Evm.h"

#include "MIDICommandFrom@fix@Serial.h"


class Test
{
public:
    void Start()
    {
        @fix@Serial.begin(31250);

        @fix@Serial.println("Starting");

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
        @fix@Serial.print("MIDICommand: ");
        @fix@Serial.print("type(");    @fix@Serial.print((uint8_t)c.type); @fix@Serial.print("), ");
        @fix@Serial.print("channel("); @fix@Serial.print(c.channel);       @fix@Serial.print("), ");
        @fix@Serial.print("param1(");  @fix@Serial.print(c.param1);        @fix@Serial.print("), ");
        @fix@Serial.print("param2(");  @fix@Serial.print(c.param2);        @fix@Serial.print(")");
        @fix@Serial.println();
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


















