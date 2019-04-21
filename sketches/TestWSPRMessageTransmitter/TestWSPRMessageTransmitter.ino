#include "Evm.h"
#include "Log.h"
#include "SerialInput.h"
#include "WSPRMessageTransmitter.h"


static Evm::Instance<10,10,10> evm;
static TimedEventHandlerDelegate ted;
static SerialAsyncConsoleEnhanced<20>  console;

static const uint8_t pinB = 25;

static WSPRMessage                          m;
static WSPRMessageTransmitter               mt;
static WSPRMessageTransmitter::Calibration  mtc;
static uint32_t freqInHundredths = mt.GetCalculatedFreqHundredths();

static uint8_t onOff = 0;


void PrintCurrentValues()
{
    const char *callsign = NULL;
    const char *grid     = NULL;
    uint8_t     powerDbm = 0;
    
    m.GetData(callsign, grid, powerDbm);
    
    Log(P("Current Values"));
    Log(P("--------------"));
    Log(P("freq                   : "), freqInHundredths / 100.0);
    Log(P("callsign               : "), callsign);
    Log(P("grid                   : "), grid);
    Log(P("powerDbm               : "), powerDbm);
    Log(P("crystalCorrectionFactor: "), mtc.crystalCorrectionFactor);
    Log(P("systemClockOffsetMs    : "), mtc.systemClockOffsetMs);
}

void PrintMenu()
{
    LogNL(4);
    Log(P("WSPR Calibrator"));
    Log(P("---------------"));
    LogNL();
    Log(P("Time Related"));
    LogX('-', 12);
    Log(P("timeCalibrate             - run time calibration"));
    Log(P("systemClockOffsetMs <val> - apply time calibration"));
    LogNL();
    //Log(P("Data Related"));
    //LogX('-', 20);
    //Log(P("c <callsign> - set callsign"));
    //Log(P("g <grid> - set grid"));
    //Log(P("p <power> - set power"));
    //LogNL();
    Log(P("Frequency Related"));
    LogX('-', 17);
    Log(P("chan <num>                    - change WSPR frequency channel"));
    Log(P("crystalCorrectionFactor <val> - apply frequency calibration"));
    LogNL();
    Log(P("on  - Turn on radio"));
    Log(P("off - Turn off radio"));
    LogNL();
    Log(P("Other"));
    LogX('-', 5);
    //Log(P("test - test the configuration of the message to be sent"));
    Log(P("send - send message whose contents were configured"));
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

        mt.SetFreqHundredths(freqInHundredths);

        onOff = 1;
    });

    console.RegisterCommand("freq", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint32_t val = atol(str.TokenAtIdx(1, ' '));

            freqInHundredths = val * 100;
            
            Log("Setting freq to \"", freqInHundredths / 100.0, '"');

            if (onOff)
            {
                mt.SetFreqHundredths(freqInHundredths);
            }

            PrintCurrentValues();
        }
    });
    
    console.RegisterCommand("chan", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint32_t chan = atol(str.TokenAtIdx(1, ' '));

            freqInHundredths =
                (WSPRMessageTransmitter::WSPR_DEFAULT_DIAL_FREQ * 100UL) +
                (WSPRMessageTransmitter::WSPR_OFFSET_FROM_DIAL_TO_USABLE_HZ * 100UL) +
                (chan * WSPRMessageTransmitter::WSPR_CHANNEL_BANDWIDTH_HUNDREDTHS_HZ);

            Log("Setting channel to ", chan, ", freq now ", freqInHundredths / 100.0);

            if (onOff)
            {
                mt.SetFreqHundredths(freqInHundredths);
            }

            PrintCurrentValues();
        }
    });
    
    console.RegisterCommand("send", [](char *){
        Log("Sending");

        if (!onOff)
        {
            console.Exec("on");
        }

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

            if (onOff)
            {
                console.Exec("on");
            }
        }
    });

    console.RegisterCommand("timeCalibrate", [](char *){
        PAL.PinMode(pinB, OUTPUT);

        Log(P("Testing system clock"));
        Log(P("Current systemClockOffsetMs = "), mtc.systemClockOffsetMs);
        Log(P("Measure with scope, get pulse widths to "), WSPRMessageTransmitter::WSPR_DELAY_MS, P(" ms"));
        Log(P("Positive adjustments make pulses smaller"));
        Log(P("negative adjustments make pulses larger"));
        Log(P("Starting test"));

        for (uint8_t i = 0; i < 3; ++i)
        {
            PAL.DigitalWrite(pinB, HIGH);
            PAL.Delay(WSPRMessageTransmitter::WSPR_DELAY_MS - mtc.systemClockOffsetMs);
            PAL.DigitalWrite(pinB, LOW);
            PAL.Delay(WSPRMessageTransmitter::WSPR_DELAY_MS - mtc.systemClockOffsetMs);
        }

        Log("Done");
    });
    
    console.RegisterCommand("help", [](char *){
        PrintMenu();
    });


    // Set some defaults
    m.SetCallsign("KD2KDD");
    m.SetGrid("AB12");
    m.SetPower(27);

    // Toggle pinB whenever a bit changes
    mt.SetCallbackOnBitChange([](){
        PAL.PinMode(pinB, OUTPUT);
        PAL.DigitalToggle(pinB);
    });

    console.SetVerbose(0);
    console.Start();
    console.Exec("pin set 15 1");
    console.Exec("help");
    
    evm.MainLoop();
}

void loop() {}



