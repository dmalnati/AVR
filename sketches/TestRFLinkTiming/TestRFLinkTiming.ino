#include "Evm.h"
#include "Log.h"
#include "LogBlob.h"
#include "SerialInput.h"
#include "RFLink.h"
#include "DurationAuditor.h"


// runs at 434.000 MHz


static Evm::Instance<10,10,10> evm;
static SerialAsyncConsoleEnhanced<5>  console;

static const uint8_t PIN_IRQ = 12;
static const uint8_t PIN_SDN = 13;
static const uint8_t PIN_SEL = 14;
static RFLink r(PIN_IRQ, PIN_SDN, PIN_SEL);
static RFLink_Raw &rr = *r.GetLinkRaw();
static RFSI4463PROPacket &radio = r.GetRadio();

static Pin dbg(6, LOW);


// Seen to get the transmitter and receiver sub-10us of one another
// when the TX callback is in application code and the RX callback
// is in application code.
static uint32_t syncUs = 300;


void setup()
{
    LogStart(9600);
    LogNL();
    Log("Starting timing tester");

    console.RegisterCommand("rtt", [](char *){
        rr.SetOnMessageTransmittedCallback([](){
            // Signal that message is known sent from the application perspective.
            // This can be compared to the receiving application reception signal.
            // By comparing the two you can determine how closely the sender and
            // receiver naturally are at the time where:
            // - sender gets callback tx complete
            // - receiver gets callback rx data
            // and take any synchronization steps necessary at that point.
            PAL.DelayMicroseconds(syncUs);
            PAL.DigitalWrite(dbg, LOW);
            PAL.DigitalWrite(dbg, HIGH);
        });

        rr.SetOnMessageReceivedCallback([](uint8_t *, uint8_t){
            uint32_t timeNowUs = PAL.Micros();

            // Signal reception and completion of round trip
            PAL.DigitalWrite(dbg, LOW);
            
            RFSI4463PROPacket::Measurements m = radio.GetMeasurements();
    
            DurationAuditorMicros<6> auditor;
            auditor.Audit("TX", m.timeUsPacketTxStart);
            auditor.Audit("TXH", m.timeUsPacketTxHandoffComplete);
            auditor.Audit("TXC", m.timeUsPacketTxComplete);
            auditor.Audit("RXC", m.timeUsPacketRxOnChip);
            auditor.Audit("RXH", m.timeUsPacketRxHandoffComplete);
            auditor.Audit("NOW", timeNowUs);

            Log(P("RTT Latency:"));
            auditor.Report();

            LogNL();

            console.Exec("listen");
        });

        char bufReq[] = "REQ_RTT";
        rr.Send((uint8_t *)bufReq, sizeof(bufReq));

        // Signal sent and start of round trip
        PAL.DigitalWrite(dbg, HIGH);
    });

    console.RegisterCommand("listen", [](char *){
        rr.SetOnMessageReceivedCallback([](uint8_t *buf, uint8_t bufLen){
            const char bufReq[] = "REQ_RTT";
            const char bufRep[] = "REP_RTT";

            // Signal reception
            PAL.DigitalWrite(dbg, HIGH);
            PAL.DigitalWrite(dbg, LOW);

            if (bufLen == sizeof(bufReq) && !memcmp(buf, bufReq, bufLen))
            {
                rr.Send((uint8_t *)bufRep, sizeof(bufRep));
            }
        });

        rr.SetOnMessageTransmittedCallback([](){
            RFSI4463PROPacket::Measurements m = radio.GetMeasurements();
    
            DurationAuditorMicros<5> auditor;
            auditor.Audit("RXC", m.timeUsPacketRxOnChip);
            auditor.Audit("RXH", m.timeUsPacketRxHandoffComplete);
            auditor.Audit("TX", m.timeUsPacketTxStart);
            auditor.Audit("TXH", m.timeUsPacketTxHandoffComplete);
            auditor.Audit("TXC", m.timeUsPacketTxComplete);
            
            Log(P("REP Latency:"));
            auditor.Report();

            LogNL();
        });

        r.ModeReceive();
    });

    console.RegisterCommand("sync", [](char *cmdStr){
        Str str(cmdStr);

        if (str.TokenCount(' ') == 2)
        {
            uint32_t val = atol(str.TokenAtIdx(1, ' '));

            syncUs = val;
        }

        Log(P("SyncUs: "), syncUs);
        LogNL();
    });


    console.SetVerbose(0);
    console.Start();
    
    uint8_t initOk = r.Init();
    Log("Init - ", initOk ? "OK" : "ERR");
    console.Exec("listen");
    LogNL();

    evm.MainLoop();
}

void loop() {}
