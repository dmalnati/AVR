#include "ThinSoftwareSerial.h"
#include "SerialInput.h"
#include "PAL.h"
#include "LogBlob.h"

static Evm::Instance<10,10,10> evm;
static SerialAsyncConsoleEnhanced<25>  console;
static TimedEventHandlerDelegate ted;

static const int8_t PIN_GPS_RX = 23;
static const int8_t PIN_GPS_TX = 24;
static ThinSoftwareSerial ss(PIN_GPS_RX, PIN_GPS_TX);

static uint16_t SS_BAUD = 9600;
static uint8_t MAX_BYTES = ThinSoftwareSerial::BUF_SIZE;
static uint32_t INTERVAL_MS = 10;

/*
 * As per usual there is a conflict between ingesting serial data and replicating it
 * back to the console.
 * 
 * If you try to push a lot of data, the program tries to spit it back to the screen,
 * which takes time, preventing interrupts, making you miss bytes.
 * 
 * An effective technique is having a long interval, pushing up to BUF_SIZE of data, and
 * then seeing it come back.  You may get hiccups if you're transmitting and an interval
 * timer fires, because then writing begins again, and messes up reading.
 * 
 * Basically if you write your programs to ingest the data but not spend lots of time
 * spitting it back to the screen, or otherwise preventing interrupts, then you should
 * be able to ingest at a reasonable rate.
 * 
 * Tested both on 8MHz internal and 16MHz crystal with no corrections at 9600 baud.
 * 
 */

void setup()
{
    LogStart(9600);
    Log(P("Starting"));

    ss.begin(9600);
    Log(P("SS Starting"));
    

    console.RegisterCommand("speed", 1, [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            double val = atof(str.TokenAtIdx(1, ' '));

            Log(P("SetRunAtSpeedFactor("), val, ')');

            ss.SetRunAtSpeedFactor(val);

            console.Exec("begin");
        }
    });
        
    console.RegisterCommand("begin", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint16_t val = atoi(str.TokenAtIdx(1, ' '));

            SS_BAUD = val;
        }

        Log(P("Begin("), SS_BAUD, ')');

        ss.begin(SS_BAUD);
    });

    console.RegisterCommand("listen", [](char *){
        Log("Listen");
        ss.listen();
    });
    
    console.RegisterCommand("stop", [](char *){
        Log("stopListening");
        ss.stopListening();
    });

    console.RegisterCommand("write", [](char *cmdStr){
        const char *data = &cmdStr[6];
        uint8_t dataLen = strlen(data);
        
        Log(P("send["), dataLen, P("]("), data, ')');

        ss.write((uint8_t *)data, dataLen);
        ss.write('\n');
    });

    // hook up my ss out to my ss in, I will validate
    console.RegisterCommand("selftest", [](char *cmdStr){
        const char *data = &cmdStr[6];
        uint8_t dataLen = strlen(data);
        
        Log(P("send["), dataLen, P("]("), data, ')');

        ss.write((uint8_t *)data, dataLen);
        ss.write('\n');
    });

    console.RegisterCommand("int", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint32_t val = atoi(str.TokenAtIdx(1, ' '));

            INTERVAL_MS = val;
        }

        Log(P("IntervalMs "), INTERVAL_MS);

        ted.RegisterForTimedEventInterval(INTERVAL_MS);
    });

    console.RegisterCommand("max", [](char *cmdStr){
        Str str(cmdStr);
        
        if (str.TokenCount(' ') == 2)
        {
            uint8_t val = atoi(str.TokenAtIdx(1, ' '));

            MAX_BYTES = val;
        }
        
        Log(P("MAX_BYTES "), MAX_BYTES);
    });



    ted.SetCallback([](){
        uint8_t bytesRemaining = MAX_BYTES;
        while (ss.available() && bytesRemaining)
        {
            uint8_t b = ss.read();
            
            if (isprint(b) || b == '\n')
            {
                LogNNL((char)b);
            }
            else
            {
//                LogNL();
//                LogNNL(LogHEX(b));
//                LogNL();
            }

            --bytesRemaining;
        }
    });
    ted.RegisterForTimedEventInterval(INTERVAL_MS);


    
    console.Start();
    console.Exec("max");
    console.Exec("int");
    console.Exec("begin");

    evm.MainLoop();
}

void loop() {}
