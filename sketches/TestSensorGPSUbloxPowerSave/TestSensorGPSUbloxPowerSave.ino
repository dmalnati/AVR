#include "Evm.h"
#include "SoftwareSerial.h"
#include "SensorGPSUblox.h"
#include "UtlSerial.h"


// Not actually used, just needs a concrete instance
// to be here to safely instantiate any objects (gps)
// which use the evm behind the scenes.
Evm::Instance<10,10,10> evm;

static const int8_t PIN_GPS_RX = 5;
static const int8_t PIN_GPS_TX = 6;
static SensorGPSUblox gps(PIN_GPS_RX, PIN_GPS_TX);

static SoftwareSerial &ss = gps.DebugGetSS();

static const uint8_t NUM_COMMANDS = 10;
static SerialShell<NUM_COMMANDS> shell;

static UbxMessage<100> ubxMsg;


void setup()
{
    Serial.begin(9600);
    Serial.println("Starting");

    gps.Init();
    gps.SetHighAltitudeMode();

    shell.RegisterCommand("lowpower", [](char *) {
        Serial.println("Enabling low power mode");
        
        // Disable GLONASS mode
        static uint8_t disable_glonass[20] = {0xB5, 0x62, 0x06, 0x3E, 0x0C, 0x00, 0x00, 0x00, 0x20, 0x01, 0x06, 0x08, 0x0E, 0x00, 0x00, 0x00, 0x01, 0x01, 0x8F, 0xB2};
    
        //gps_sendubx(disable_glonass, 20);
        ss.write(disable_glonass, 20);
    
        // Enable power saving
        uint8_t enable_powersave[10] = {0xB5, 0x62, 0x06, 0x11, 0x02, 0x00, 0x08, 0x01, 0x22, 0x92};
        
        //gps_sendubx(enable_powersave, 10);
        ss.write(enable_powersave, 10);
    });

    shell.RegisterCommand("interval", [](char *cmdStr) {
        Str str(cmdStr);

        if (str.TokenCount(' ') == 2)
        {
            uint32_t intervalMs = atol(str.TokenAtIdx(1, ' '));

            Serial.print("Setting interval to ");  Serial.println(intervalMs);
            
            gps.SetMessageInterval(intervalMs);
        }
    });

    shell.RegisterCommand("reset", [](char *cmdStr) {
        Str str(cmdStr);

        uint16_t resetType = 0x0000;
        if (str.TokenCount(' ') >= 2)
        {
            const char *resetTypeStr = str.TokenAtIdx(1, ' ');
            
            if      (!strcmp(resetTypeStr, "hot"))  { resetType = 0x0000; }
            else if (!strcmp(resetTypeStr, "warm")) { resetType = 0x0001; }
            else if (!strcmp(resetTypeStr, "cold")) { resetType = 0xFFFF; }
        }

        uint8_t resetMode = 0x00;
        if (str.TokenCount(' ') >= 3)
        {
            const char *resetModeStr = str.TokenAtIdx(2, ' ');
            
            if      (!strcmp(resetModeStr, "hw"))                  { resetMode = 0x00; }
            else if (!strcmp(resetModeStr, "sw"))                  { resetMode = 0x01; }
            else if (!strcmp(resetModeStr, "sw_gps_only"))         { resetMode = 0x02; }
            else if (!strcmp(resetModeStr, "hw_after_sd"))         { resetMode = 0x04; }
            else if (!strcmp(resetModeStr, "controlled_gps_down")) { resetMode = 0x08; }
            else if (!strcmp(resetModeStr, "controlled_gps_up"))   { resetMode = 0x09; }
        }

        Serial.print("Resetting resetType: ");
        Serial.print(resetType);
        Serial.print(", resetMode: ");
        Serial.print(resetMode);
        Serial.println();

        // Fill out UBX Message
        ubxMsg.Reset();

        ubxMsg.SetClass(0x06);
        ubxMsg.SetId(0x04);

        ubxMsg.AddFieldX2(resetType);
        ubxMsg.AddFieldU1(resetMode);
        ubxMsg.AddFieldU1(0);   // reserved

        uint8_t *buf;
        uint8_t  bufLen;
        ubxMsg.GetBuf(&buf, &bufLen);

        ss.write(buf, bufLen);
    });

    
    
    while (1)
    {
        shell.Run();
    }
}

void loop() {
  // put your main code here, to run repeatedly:

}
