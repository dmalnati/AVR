#define IVM_DISABLE

#include "Evm.h"
#include "PeripheralOpenLog.h"



static const int8_t PIN_LOGGER_TX = 25;

static const uint32_t DEBUG_INTERVAL_MS = 1000;

static Evm::Instance<10,10,10> evm;

static TimedEventHandlerDelegate ted;
static PeripheralOpenLog<2> logger(PIN_LOGGER_TX);


void setup()
{
    Serial.begin(9600);
    Serial.println("Starting");
    logger.Init();

    PeripheralOpenLogFileHandle *fhOne = logger.GetFileHandle("one.txt");
    PeripheralOpenLogFileHandle *fhTwo = logger.GetFileHandle("two.txt");

    uint16_t count = 0;
    
    ted.SetCallback([&]() {
        ++count;

        if ((count % 2))
        {
            Serial.println("Appending ONE");
            fhOne->Append(count);
            fhOne->Append(" ");
        }
        else
        {
            Serial.println("Appending TWO");
            fhTwo->Append(count);
            fhTwo->Append(" ");
        }
    });
    
    ted.RegisterForTimedEventInterval(DEBUG_INTERVAL_MS);
    
    evm.MainLoop();
}

void loop() {}


