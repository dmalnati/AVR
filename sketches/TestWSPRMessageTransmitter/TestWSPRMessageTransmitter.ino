#include "Evm.h"
#include "Log.h"
#include "SerialInput.h"
#include "WSPRMessageTransmitter.h"


static Evm::Instance<10,10,10> evm;
static TimedEventHandlerDelegate ted;
static SerialAsyncConsoleEnhanced<10>  console;

static WSPRMessage                          m;
static WSPRMessageTransmitter               mt;
static WSPRMessageTransmitter::Calibration  mtc;

static uint8_t onOff = 0;


void PrintCurrentValues()
{
    const char *callsign = NULL;
    const char *grid     = NULL;
    uint8_t     powerDbm = 0;
    
    m.GetData(callsign, grid, powerDbm);
    
    Log(P("Current Values"));
    Log(P("--------------"));
    Log(P("callsign               : "), callsign);
    Log(P("grid                   : "), grid);
    Log(P("powerDbm               : "), powerDbm);
    Log(P("crystalCorrectionFactor: "), mtc.crystalCorrectionFactor);
    Log(P("systemClockOffsetMs    : "), mtc.systemClockOffsetMs);
}

void PrintMenu()
{
    LogNL(4);
    Log(P("WSPR Message Transmitter"));
    Log(P("------------------------"));
    LogNL();
    Log(P("Tuned to 14.095600 MHz"));
    LogNL();
    Log(P("c <callsign> - set callsign"));
    Log(P("g <grid> - set grid"));
    Log(P("p <power> - set power"));
    LogNL();
    Log(P("crystalCorrectionFactor - "));
    Log(P("systemClockOffsetMs - apply time calibration"));
    LogNL();
    Log(P("on - Turn on radio"));
    Log(P("off - Turn off radio"));
    LogNL();
    Log(P("test - test the configuration of the message to be sent"));
    Log(P("send - send message whose contents were configured"));
    LogNL();
    Log(P("help - this menu"));
    LogNL();
    LogNL();
    PrintCurrentValues();
    LogNL();
}

void setup()
{
    LogStart(9600);

    console.RegisterCommand("c", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            const char *p = str.TokenAtIdx(1, ' ');
            
            Log("Setting callsign to \"", p, '"');

            m.SetCallsign((char *)p);

            PrintCurrentValues();
        }
    });

    console.RegisterCommand("g", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            const char *p = str.TokenAtIdx(1, ' ');
            
            Log("Setting grid to \"", p, '"');

            m.SetGrid((char *)p);

            PrintCurrentValues();
        }
    });
    
    console.RegisterCommand("p", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint8_t val = atoi(str.TokenAtIdx(1, ' '));
            
            Log("Setting power to \"", val, '"');

            m.SetPower(val);

            PrintCurrentValues();
        }
    });

    console.RegisterCommand("on", [](char *){
        Log("Radio On");

        mt.SetCalibration(mtc);

        mt.RadioOn();

        onOff = 1;
    });
    
    console.RegisterCommand("send", [](char *){
        Log("Sending");

        PrintCurrentValues();
        
        mt.Send(&m);

        Log(P("Send complete"));
    });
    
    console.RegisterCommand("off", [](char *){
        Log("Radio Off");

        mt.RadioOff();

        onOff = 0;
    });

    console.RegisterCommand("test", [](char *){
        LogNNL("Testing settings - ");

        PrintCurrentValues();

        uint8_t retVal = mt.Test(&m);

        Log(retVal ? "OK" : "ERR");
    });

    console.RegisterCommand("crystalCorrectionFactor", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            mtc.crystalCorrectionFactor = atol(str.TokenAtIdx(1, ' '));
            
            Log("Setting crystalCorrectionFactor to ", mtc.crystalCorrectionFactor);

            PrintCurrentValues();

            if (onOff)
            {
                console.Exec("on");
            }
        }
    });
    
    console.RegisterCommand("systemClockOffsetMs", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            mtc.systemClockOffsetMs = atol(str.TokenAtIdx(1, ' '));
            
            Log("Setting systemClockOffsetMs to ", mtc.systemClockOffsetMs);

            PrintCurrentValues();
        }
    });
    
    console.RegisterCommand("help", [](char *){
        PrintMenu();
    });


    // Set some defaults
    m.SetCallsign("KD2KDD");
    m.SetGrid("AB12");
    m.SetPower(27);

    console.SetVerbose(0);
    console.Start();
    console.Exec("help");
    
    evm.MainLoop();
}

void loop() {}



