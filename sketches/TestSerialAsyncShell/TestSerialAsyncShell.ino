#include "Evm.h"
#include "UtlSerial.h"


static Evm::Instance<10,10,10> evm;
static SerialAsyncShell<4> shell;
static TimedEventHandlerDelegate ted;
static TimedEventHandlerDelegate tedStart;


void setup()
{
    Serial.begin(9600);
    Serial.println("Starting");

    shell.RegisterCommand("one", [](char *cmdStr){
        Serial.println(cmdStr);
    });

    shell.RegisterCommand("two", [](char *cmdStr){
        Serial.println(cmdStr);
    });

    shell.RegisterCommand("start", [](char *){
        Serial.println("Starting");
        shell.Start();
    });

    shell.RegisterCommand("stop", [](char *){
        Serial.println("Stopping, but coming back in a second");
        shell.Stop();

        tedStart.SetCallback([](){
            Serial.println("Auto re-starting");
            shell.Start();
        });
        tedStart.RegisterForTimedEvent(1000);
    });

    shell.RegisterErrorHandler([&](char *cmdStr){
        Serial.print("ERR: \""); Serial.print(cmdStr); Serial.print("\"");
    });

    shell.Start();


    uint8_t count = 0;
    ted.SetCallback([&](){
        ++count;
        Serial.print("ted "); Serial.println(count);
    });
    ted.RegisterForTimedEventInterval(1000);


    evm.MainLoop();
}

void loop() {}







