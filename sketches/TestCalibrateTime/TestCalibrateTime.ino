#include "Evm.h"
#include "Log.h"
#include "SerialInput.h"


static Evm::Instance<10,10,10> evm;
static TimedEventHandlerDelegate ted;
static SerialAsyncConsoleEnhanced<10>  console;

static const uint8_t PIN_TOGGLE = 25;
static Pin pinToggle(PIN_TOGGLE, LOW);

static uint32_t ms       = 0;
static int32_t  msOffset = 0;
static uint8_t  count    = 10;


enum class Mode : uint8_t
{
    SYNC,
    ASYNC,
};

static Mode mode = Mode::SYNC;


void Start()
{
    if (mode == Mode::SYNC)
    {
        uint32_t msDelay        = ms + msOffset;
        uint16_t countRemaining = count * 2;

        while (countRemaining)
        {
            PAL.DigitalToggle(pinToggle);
            PAL.Delay(msDelay);

            --countRemaining;
        }
    }
    else if (mode == Mode::ASYNC)
    {
        uint32_t msDelay        = ms + msOffset;
        uint16_t countRemaining = count * 2;
        
        ted.SetCallback([&](){
            PAL.DigitalToggle(pinToggle);

            --countRemaining;

            if (!countRemaining)
            {
                ted.DeRegisterForTimedEvent();
                evm.EndMainLoop();
            }
        });

        ted.RegisterForTimedEventInterval(msDelay);

        evm.HoldStackDangerously();
    }
}

void setup()
{
    LogStart(9600);
    Log("Starting");

    console.RegisterCommand("mode", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            const char *p = str.TokenAtIdx(1, ' ');

            if (!strcmp(p, "sync"))
            {
                Log("Mode: PAL.Delay()");

                mode = Mode::SYNC;
            }
            else if (!strcmp(p, "async"))
            {
                Log("Mode: evm TimeoutInterval");

                mode = Mode::ASYNC;
            }
        }
    });

    console.RegisterCommand("offset", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            msOffset = atol(str.TokenAtIdx(1, ' '));

            Log("Offset: ", msOffset);
        }
    });

    // start <ms> [<count=10>]
    console.RegisterCommand("start", [](char *cmdStr){
        Str str(cmdStr);

        if (str.TokenCount(' ') == 2 || str.TokenCount(' ') == 3)
        {
            if (str.TokenCount(' ') == 3)
            {
                count = atoi(str.TokenAtIdx(2, ' '));

                if (count == 0)
                {
                    count = 1;
                }
            }
            
            ms = atol(str.TokenAtIdx(1, ' '));

            Log("Starting: ", ms, " x ", count);

            Start();
        }
    });

    console.RegisterCommand("stop", [](char *){
        Log("Stopping");
    });

    
    console.Start();
    
    evm.MainLoop();
}

void loop() {}



