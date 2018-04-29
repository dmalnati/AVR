#include "Evm.h"
#include "Utl@fix@Serial.h"


static Evm::Instance<10,10,10> evm;
static SerialAsyncShell<4> shell;
static TimedEventHandlerDelegate ted;
static TimedEventHandlerDelegate tedStart;


void setup()
{
    @fix@Serial.begin(9600);
    @fix@Serial.println("Starting");

    shell.RegisterCommand("one", [](char *cmdStr){
        @fix@Serial.println(cmdStr);
    });

    shell.RegisterCommand("two", [](char *cmdStr){
        @fix@Serial.println(cmdStr);
    });

    shell.RegisterCommand("start", [](char *){
        @fix@Serial.println("Starting");
        shell.Start();
    });

    shell.RegisterCommand("stop", [](char *){
        @fix@Serial.println("Stopping, but coming back in a second");
        shell.Stop();

        tedStart.SetCallback([](){
            @fix@Serial.println("Auto re-starting");
            shell.Start();
        });
        tedStart.RegisterForTimedEvent(1000);
    });

    shell.RegisterErrorHandler([&](char *cmdStr){
        @fix@Serial.print("ERR: \""); @fix@Serial.print(cmdStr); @fix@Serial.print("\"");
    });

    shell.Start();


    uint8_t count = 0;
    ted.SetCallback([&](){
        ++count;
        @fix@Serial.print("ted "); @fix@Serial.println(count);
    });
    ted.RegisterForTimedEventInterval(1000);


    evm.MainLoop();
}

void loop() {}







