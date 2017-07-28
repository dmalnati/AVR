#include "Evm.h"
#include "PinInputAnalog.h"
#include "MuxAnalogDigitalCD74HC4067.h"



static const uint8_t PIN_BIT_0 = 15;
static const uint8_t PIN_BIT_1 = 16;
static const uint8_t PIN_BIT_2 = 17;
static const uint8_t PIN_BIT_3 = 18;
static const uint8_t PIN_MUX   = 23;

static Evm::Instance<10,10,10> evm;

static PinInputAnalog pia1(PIN_MUX);
static PinInputAnalog pia2(PIN_MUX);

static MuxAnalogDigitalCD74HC4067 mux(PIN_BIT_0,
                                      PIN_BIT_1,
                                      PIN_BIT_2,
                                      PIN_BIT_3,
                                      PIN_MUX);

void setup()
{
    Serial.begin(9600);
    Serial.println("Starting");

    PinInputAnalog *piaList[] = { &pia1, &pia2 };

    uint8_t channel = 0;
    for (auto pia : piaList)
    {
        pia->SetAnalogReadFunction([=channel, &mux](uint8_t pin){
            mux.ConnectToChannel(channel);
    
            return PAL.AnalogRead(pin);
        });

        pia->SetCallback([=channel, &mux](uint16_t val){
            Serial.print("channel: ");
            Serial.print(channel);
            Serial.print(", val: ");
            Serial.print(val);
        });
        
        ++channel;
    }

    evm.MainLoop();
}


void loop() {}


















