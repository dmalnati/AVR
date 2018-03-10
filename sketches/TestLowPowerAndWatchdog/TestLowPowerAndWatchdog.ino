#include "PAL.h"
#include "Evm.h"
#include "UtlSerial.h"


/*
 * Objective, confirm good working behavior between: 
 * - watchdog-based deep sleep wakeup
 * - actual watchdog behavior
 * 
 * 
 * Test1
 * - LowPower evm, set watchdog during callback, don't exceed, end during callback
 *   - expect evm timeouts still work fine
 *     - yes
 * 
 * 
 * Test2
 * - LowPower evm, set watchdog during callback, do exceed, end during callback
 *   - expect reboot
 *     - yes
 * 
 * 
 * Test3
 * - LowPower evm, start watchdog, return to evm, wait for async event, never cancel watchdog
 *   - expect watchdog never fires because low-power evm will have re-purposed it
 *     - yes (this is something you'll need to watch out for)
 *     
 *     
 * Test4 (same as Test3 but solved by going full power)
 * - LowPower evm disabled, start watchdog, return to evm, wait for async event, never cancel watchdog
 *   - expect watchdog does fire, as evm never repurposes the watchdog
 *     - yes (this is the solution to Test3 for now)
 *     
 *  
 *  Though, reading some articles, watchdogs may not be applicable for applications which sleep, since the
 *  thought process is that they can't malfunction in sleep.  Only when awake.
 *  
 *  So probably just enable/disable watchdog in any code which gets called back from evm.  Protect each function.
 *  
 */

static Evm::Instance<10,10,10>    evm;
static TimedEventHandlerDelegate  ted1;
static TimedEventHandlerDelegate  ted2;
static SerialAsyncConsole<10>     console;


void Test1()
{
    evm.LowPowerEnable();  Serial.println("LPE");  PAL.Delay(50);
    
    ted1.SetCallback([](){
        PAL.Delay(50); Serial.println("Test1"); PAL.Delay(50);
        
        PAL.WatchdogEnable(WatchdogTimeout::TIMEOUT_250_MS);
        PAL.WatchdogReset();
        PAL.WatchdogDisable();
    });
    ted1.RegisterForTimedEvent(50);

    ted2.SetCallback([](){
        evm.LowPowerDisable(); Serial.println("LPD");  PAL.Delay(50);
    });
    ted2.RegisterForTimedEvent(55);
}

void Test2()
{
    evm.LowPowerEnable();  Serial.println("LPE");  PAL.Delay(50);
    
    ted1.SetCallback([](){
        PAL.Delay(50); Serial.println("Test2"); PAL.Delay(50);
        
        PAL.WatchdogEnable(WatchdogTimeout::TIMEOUT_250_MS);
        
        while (1) {}
        
        PAL.WatchdogDisable();
    });
    ted1.RegisterForTimedEvent(50);

    ted2.SetCallback([](){
        evm.LowPowerDisable(); Serial.println("LPD");  PAL.Delay(50);
    });
    ted2.RegisterForTimedEvent(55);
}

void Test3()
{
    evm.LowPowerEnable();  Serial.println("LPE");  PAL.Delay(50);

    PAL.WatchdogEnable(WatchdogTimeout::TIMEOUT_250_MS);

    ted1.SetCallback([](){
        PAL.WatchdogDisable();
        
        PAL.Delay(50); Serial.println("Test3"); PAL.Delay(50);
    });
    ted1.RegisterForTimedEvent(500);

    ted2.SetCallback([](){
        evm.LowPowerDisable(); Serial.println("LPD");  PAL.Delay(50);
    });
    ted2.RegisterForTimedEvent(505);
}

void Test4()
{
    PAL.WatchdogEnable(WatchdogTimeout::TIMEOUT_250_MS);

    ted1.SetCallback([](){
        PAL.WatchdogDisable();
        
        PAL.Delay(50); Serial.println("Test4"); PAL.Delay(50);
    });
    ted1.RegisterForTimedEvent(500);
}






void setup()
{
    Serial.begin(9600);

    if (PAL.GetStartupMode() == PlatformAbstractionLayer::StartupMode::RESET_WATCHDOG)
    {
        Serial.println();
        Serial.println();
        Serial.println("-- Watchdog Reset --");
        Serial.println();
        Serial.println();
    }
    
    Serial.println("Starting");

    console.RegisterCommand("test1", [](char *){ Test1(); });
    console.RegisterCommand("test2", [](char *){ Test2(); });
    console.RegisterCommand("test3", [](char *){ Test3(); });
    console.RegisterCommand("test4", [](char *){ Test4(); });

    console.RegisterErrorHandler([](char *){
        Serial.println("ERR");
    });

    console.Start();

    // start out with low power mode disabled, so serial can be monitored
    evm.LowPowerDisable();

    // process events
    evm.MainLoopLowPower();
}

void loop() {}




