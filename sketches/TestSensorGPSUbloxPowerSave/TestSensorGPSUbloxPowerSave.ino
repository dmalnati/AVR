#include "Evm.h"
#include "SoftwareSerial.h"
#include "SensorGPSUblox.h"
#include "UtlSerial.h"
#include "UtlStreamBlob.h"


// Not actually used, just needs a concrete instance
// to be here to safely instantiate any objects (gps)
// which use the evm behind the scenes.
Evm::Instance<10,10,10> evm;

static const int8_t PIN_GPS_RX = 5;
static const int8_t PIN_GPS_TX = 6;
static SensorGPSUblox gps(PIN_GPS_RX, PIN_GPS_TX);

static SoftwareSerial &ss = gps.DebugGetSS();

static const uint8_t NUM_COMMANDS = 20;
static SerialShell<NUM_COMMANDS> shell;

static UbxMessage<100> ubxMsg;

static const uint8_t UBX_IN_BUF_SIZE = 150;
static uint8_t ubxInBuf[UBX_IN_BUF_SIZE] = { 0 };


struct ParsedMessage
{
    uint8_t  msgClass;
    uint8_t  msgId;
    uint8_t *buf;
    uint8_t  len;
    uint8_t  bufLen;
    
    const char *failReason;
};


void PrintMessage(ParsedMessage msg)
{
    Serial.print(F("msgClass: 0x")); Serial.println(msg.msgClass, HEX);
    Serial.print(F("msgId   : 0x")); Serial.println(msg.msgId, HEX);
    Serial.print(F("msglen  : ")); Serial.println(msg.len);
    Serial.print(F("bufLen  : ")); Serial.println(msg.bufLen);
    StreamBlob(Serial, msg.buf, msg.bufLen, 1, 1);
}

uint8_t GetMessage(ParsedMessage &msg)
{
    enum class State : uint8_t
    {
        LOOKING_FOR_HEADER = 0,
        LOOKING_FOR_CLASS,
        LOOKING_FOR_ID,
        LOOKING_FOR_LEN,
        LOOKING_FOR_CHECKSUM,
    };

    State state = State::LOOKING_FOR_HEADER;

    uint8_t retVal = 0;

    static const uint16_t GIVE_UP_AFTER = 5000;
    uint16_t tryCount = 0;

    uint8_t idx = 0;

    uint8_t msgClass = 0;
    uint8_t msgId    = 0;
    uint8_t bufLen   = 0;

    uint16_t len = 0;

    uint8_t idxStopAt = 0;

    uint8_t found = 0;

    uint8_t cont = 1;
    while (cont)
    {
        if (ss.available())
        {
            //Serial.println(F("Available"));
            
            uint8_t b = (uint8_t)ss.read();

            if (state == State::LOOKING_FOR_HEADER)
            {
                //Serial.println(F("LOOKING_FOR_HEADER"));
                
                // store this byte
                ubxInBuf[idx] = b;
                ++idx;

                // check if we received a byte previously
                // need 2 to match header
                if (idx == 1)
                {
                    // nope, leave that byte stored and carry on
                }
                else
                {
                    // yup, check if this is a valid header now
                    if (ubxInBuf[0] == 0xB5 && ubxInBuf[1] == 0x62)
                    {
                        // yup, move to next state
                        state = State::LOOKING_FOR_CLASS;
                    }
                    else
                    {
                        //Serial.print(F("    discarding ")); Serial.println(ubxInBuf[0], HEX);
                        
                        // nope, maybe the last byte that came in is the start,
                        // shift it to the start and carry on
                        ubxInBuf[0] = ubxInBuf[1];
                        --idx;
                    }
                }
            }
            else if (state == State::LOOKING_FOR_CLASS)
            {
                //Serial.println(F("LOOKING_FOR_CLASS"));

                ubxInBuf[idx] = b;
                ++idx;
                
                msgClass = b;

                //Serial.print(F("    class: "));  Serial.println(b, HEX);

                state = State::LOOKING_FOR_ID;
            }
            else if (state == State::LOOKING_FOR_ID)
            {
                //Serial.println("LOOKING_FOR_ID");

                ubxInBuf[idx] = b;
                ++idx;
                
                msgId = b;

                //Serial.print(F("    id: "));  Serial.println(b, HEX);

                state = State::LOOKING_FOR_LEN;
            }
            else if (state == State::LOOKING_FOR_LEN)
            {
                //Serial.println(F("LOOKING_FOR_LEN"));
                
                ubxInBuf[idx] = b;
                ++idx;

                if (idx == 6)
                {
                    // we have the full size
                    // we want to go from little endian wire format to host endian
                    // start by making a network-byte-order 16-bit int, aka big endian
                    uint16_t lenBigEndian;
                    char *p = (char *)&lenBigEndian;
                    p[0] = ubxInBuf[5];
                    p[1] = ubxInBuf[4];

                    len = PAL.ntohs(lenBigEndian);

                    //Serial.print(F("    lenBigEndian: "));  Serial.println(lenBigEndian);
                    //Serial.print(F("    len         : "));  Serial.println(len);

                    // length does not include the header, class, id, length, or checksum fields.
                    if (idx + len + 2 <= UBX_IN_BUF_SIZE)
                    {
                        idxStopAt = idx + len + 2;

                        state = State::LOOKING_FOR_CHECKSUM;
                    }
                    else
                    {
                        // Can't fit
                        cont = 0;

                        //Serial.println(F("Message too large"));
                    }
                }
                else
                {
                    // Nothing to do, keep collecting
                }
            }
            else if (state == State::LOOKING_FOR_CHECKSUM)
            {
                //Serial.println(F("LOOKING_FOR_CHECKSUM"));
                
                ubxInBuf[idx] = b;
                ++idx;

                if (idx == idxStopAt)
                {
                    // Time to calculate and compare the checksum

                    uint8_t idxChecksumStart = idxStopAt - 2;

                    uint8_t ckA = 0;
                    uint8_t ckB = 0;

                    // did store the header (2 bytes total)
                    // did store the class and id (2 bytes total)
                    // did store the size (2 bytes total)
                    // supposed to checksum from class and id forward, 
                    
                    for (uint8_t i = 2; i < idxChecksumStart; ++i)
                    {
                        uint8_t b = ubxInBuf[i];
                        
                        ckA += b;
                        ckB += ckA;
                    }

                    // extract the message checksum
                    uint8_t msgCkA = ubxInBuf[idxChecksumStart + 0];
                    uint8_t msgCkB = ubxInBuf[idxChecksumStart + 1];

                    //Serial.print("ckA, ckB: ");       Serial.print(ckA);    Serial.print(" "); Serial.print(ckB);    Serial.println();
                    //Serial.print("msgCkA, msgCkB: "); Serial.print(msgCkA); Serial.print(" "); Serial.print(msgCkB); Serial.println();

                    //StreamBlob(Serial, ubxInBuf, UBX_IN_BUF_SIZE, 1, 1);
                    bufLen = idxStopAt;

                    if (ckA == msgCkA && ckB == msgCkB)
                    {
                        // success
                        found = 1;
                    }
                    else
                    {
                        //Serial.println(F("Checksum failed"));
                        msg.failReason = "Checksum failed";
                    }

                    cont = 0;
                }
                else
                {
                    // Nothing to do, just pile on bytes until reaching the checksum
                }
            }
        }
        else
        {
            PAL.Delay(1);
        }

        ++tryCount;
        if (tryCount == GIVE_UP_AFTER)
        {
            cont = 0;

            //Serial.println(F("Too many attempts, giving up"));
            msg.failReason = "Too many attempts";
        }
    }

    if (found)
    {
        retVal = 1;

        msg.msgClass = msgClass;
        msg.msgId    = msgId;
        msg.buf      = ubxInBuf;
        msg.len      = len;
        msg.bufLen   = bufLen;
        
        msg.failReason = "";
    }

    return retVal;
}

uint8_t GetMessageOrErr(uint8_t printMessage = 1)
{
    uint8_t retVal = 0;
    
    ParsedMessage msg;

    Serial.print("Waiting for msg... ");
    if (GetMessage(msg))
    {
        retVal = 1;
        
        Serial.println("YES");

        if (printMessage)
        {
            PrintMessage(msg);
        }
    }
    else
    {
        Serial.print("NO - ");
        Serial.print(msg.failReason);
        Serial.println();
    }
    Serial.println();

    return retVal;
}

void setup()
{
    Serial.begin(9600);
    Serial.println("Starting");

    gps.Init();
    gps.SetHighAltitudeMode();
    GetMessageOrErr();



    shell.RegisterCommand("interval", [](char *cmdStr) {
        Str str(cmdStr);

        if (str.TokenCount(' ') == 2)
        {
            uint32_t intervalMs = atol(str.TokenAtIdx(1, ' '));

            Serial.print(F("Setting interval to "));  Serial.println(intervalMs);
            
            gps.SetMessageInterval(intervalMs);
            
            GetMessageOrErr();
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

        Serial.print(F("Resetting resetType: "));
        Serial.print(resetType);
        Serial.print(F(", resetMode: "));
        Serial.print(resetMode);
        Serial.println();

        // Fill out UBX Message
        ubxMsg.Reset();

        // CFG-RST (0x06 0x04)
        ubxMsg.SetClass(0x06);
        ubxMsg.SetId(0x04);

        ubxMsg.AddFieldX2(resetType);
        ubxMsg.AddFieldU1(resetMode);
        ubxMsg.AddFieldU1(0);   // reserved

        uint8_t *buf;
        uint8_t  bufLen;
        ubxMsg.GetBuf(&buf, &bufLen);

        StreamBlob(Serial, buf, bufLen, 0, 1);
        ss.write(buf, bufLen);
        
        GetMessageOrErr();  // Didn't see this work ever
    });

    shell.RegisterCommand("alm", [](char *) {
        ubxMsg.Reset();

        // AID-ALM (0x0B 0x30)
        ubxMsg.SetClass(0x0B);
        ubxMsg.SetId(0x30);

        uint8_t *buf;
        uint8_t  bufLen;
        ubxMsg.GetBuf(&buf, &bufLen);

        StreamBlob(Serial, buf, bufLen, 0, 1);
        ss.write(buf, bufLen);

        // Get all messages
        uint8_t count = 0;
        while (GetMessageOrErr(0))
        {
            ++count;
        }
        Serial.print(F("Got ")); Serial.print(count); Serial.println(F(" AID-ALM responses"));
    });

    shell.RegisterCommand("eph", [](char *) {
        ubxMsg.Reset();

        // AID-EPH (0x0B 0x31)
        ubxMsg.SetClass(0x0B);
        ubxMsg.SetId(0x31);

        uint8_t *buf;
        uint8_t  bufLen;
        ubxMsg.GetBuf(&buf, &bufLen);

        StreamBlob(Serial, buf, bufLen, 0, 1);
        ss.write(buf, bufLen);

        // Get all messages
        uint8_t count = 0;
        while (GetMessageOrErr(0))
        {
            ++count;
        }
        Serial.print(F("Got ")); Serial.print(count); Serial.println(F(" AID-EPH responses"));
    });

    
    shell.RegisterCommand("save", [](char *) {
        ubxMsg.Reset();

        // CFG-CFG (0x06 0x09)
        ubxMsg.SetClass(0x06);
        ubxMsg.SetId(0x09);

        ubxMsg.AddFieldX4(0);           // clearMask  - clear nothing
        ubxMsg.AddFieldX4(0x0000FFFF);  // saveMask   - save everything
        ubxMsg.AddFieldX4(0);           // loadMask   - load nothing
        ubxMsg.AddFieldX1(1);           // deviceMask - save to batter-backed ram (internal to chip)

        uint8_t *buf;
        uint8_t  bufLen;
        ubxMsg.GetBuf(&buf, &bufLen);

        StreamBlob(Serial, buf, bufLen, 0, 1);
        ss.write(buf, bufLen);

        GetMessageOrErr();
    });



    
    shell.RegisterCommand("wake", [](char *) {
        ss.write(0xFF);
    });

    
    //////////////////////////////////////////////////////////
    //
    // Log
    //
    //////////////////////////////////////////////////////////


    
    shell.RegisterCommand("log-info", [](char *) {
        Serial.println(F("LOG-INFO (0x21 0x08)"));
        
        ubxMsg.Reset();

        // LOG-INFO (0x21 0x08)
        ubxMsg.SetClass(0x21);
        ubxMsg.SetId(0x08);

        uint8_t *buf;
        uint8_t  bufLen;
        ubxMsg.GetBuf(&buf, &bufLen);

        StreamBlob(Serial, buf, bufLen, 0, 1);
        ss.write(buf, bufLen);

        GetMessageOrErr();
        GetMessageOrErr();
    });

    

    //////////////////////////////////////////////////////////
    //
    // Power Saving
    //
    //////////////////////////////////////////////////////////



    // turns out this doesn't work on my NEO-6M, but does on NEO-7M
    shell.RegisterCommand("noglonass", [](char *) {
        Serial.println(F("Disabling GLONASS"));
        // Disable GLONASS mode
        // (CFG-GNSS 0x06 0x3E)
        static uint8_t disable_glonass[20] = {0xB5, 0x62, 0x06, 0x3E, 0x0C, 0x00, 0x00, 0x00, 0x20, 0x01, 0x06, 0x08, 0x0E, 0x00, 0x00, 0x00, 0x01, 0x01, 0x8F, 0xB2};

        ss.write(disable_glonass, 20);

        GetMessageOrErr();
    });


    shell.RegisterCommand("gpsonly", [](char *) {
        Serial.println(F("Enabling GPS only"));
        // Enable power saving
        uint8_t setGPSonly[28] = {0xB5, 0x62, 0x6, 0x3E, 0x14, 0x0, 0x0, 0x0, 0xFF, 0x2, 0x0, 0x8, 0xFF, 0x0, 0x1, 0x0, 0x1, 0x0, 0x6, 0x8, 0xFF, 0x0, 0x0, 0x0, 0x0, 0x0, 0x6F, 0xCC};

        // Analysis
        // size = 0x14 (20 bytes), so 4 header plus 2 config blocks

        // 0x0, 0x0, 0xFF, 0x2,
        // version 0
        // num hw channels (readonly) 0
        // num track channels to use 255
        // num config blocks 2

        
        // 0x0, 0x8, 0xFF, 0x0, 0x1, 0x0, 0x1, 0x0,
        // gnssId = 0 (GPS)
        // reserve this many channels = 8
        // max channels to use = 255
        // reservedField = 0
        // flags = enable this system (though bitfield has an extra 1 in there for some reason)

        
        // 0x6, 0x8, 0xFF, 0x0, 0x0, 0x0, 0x0, 0x0,
        // gnssId = 6 (GLONASS)
        // reserve this many channels = 8
        // max channels to use = 255
        // reservedField = 0
        // flags = disable this system
        
        
        ss.write(setGPSonly, 28);

        GetMessageOrErr();
    });

    shell.RegisterCommand("lowpower", [](char *) {
        Serial.println(F("Enabling power save"));

        // CFG-RXM (0x06 0x11)
        uint8_t enable_powersave[10] = {0xB5, 0x62, 0x06, 0x11, 0x02, 0x00, 0x08, 0x01, 0x22, 0x92};

        ss.write(enable_powersave, 10);

        GetMessageOrErr();
    });

    shell.RegisterCommand("cfg-gnss", [](char *) {
        Serial.println(F("CFG-GNSS (0x06 0x3E)"));

        ubxMsg.Reset();
        
        // CFG-GNSS (0x06 0x3E)
        ubxMsg.SetClass(0x06);
        ubxMsg.SetId(0x3E);

        ubxMsg.AddFieldU1(0);   // msgVer          - 0 (required)
        ubxMsg.AddFieldU1(0);   // numTrkChcHw     - (read-only, so set to 0?)
        ubxMsg.AddFieldU1(255); // numTrkChUse     - use this many of the available channels
        ubxMsg.AddFieldU1(4);   // numConfigBlocks - how many repeating sections follow

        // Satellite Numbering
        // gnssId  GNSS Type
        // 0       GPS
        // 1       SBAS
        // 5       QZSS
        // 6       GLONASS

        // Do repeating sections of these
        // ubxMsg.AddFieldU1(?);   // gnssId    - GNSS identifier (see Satellite Numbering)
        // ubxMsg.AddFieldU1(?);   // resTrkCh  - Number of reserved (minimum) tracking channels for this GNSS system
        // ubxMsg.AddFieldU1(?);   // maxTrkCh  - Maximum number of tracking channels used for this GNSS system (>=resTrkChn)
        // ubxMsg.AddFieldU1(?);   // reserved1 - Reserved
        // ubxMsg.AddFieldX4(?);   // flags     - bitfield of flags (see graphic below)

        // GPS
        ubxMsg.AddFieldU1(0);   // gnssId    - GNSS identifier (see Satellite Numbering)
        ubxMsg.AddFieldU1(8);   // resTrkCh  - Number of reserved (minimum) tracking channels for this GNSS system
        ubxMsg.AddFieldU1(255); // maxTrkCh  - Maximum number of tracking channels used for this GNSS system (>=resTrkChn)
        ubxMsg.AddFieldU1(0);   // reserved1 - Reserved
        ubxMsg.AddFieldX4(1);   // flags     - enable

        // SBAS
        ubxMsg.AddFieldU1(1);   // gnssId    - GNSS identifier (see Satellite Numbering)
        ubxMsg.AddFieldU1(0);   // resTrkCh  - Number of reserved (minimum) tracking channels for this GNSS system
        ubxMsg.AddFieldU1(0);   // maxTrkCh  - Maximum number of tracking channels used for this GNSS system (>=resTrkChn)
        ubxMsg.AddFieldU1(0);   // reserved1 - Reserved
        ubxMsg.AddFieldX4(0);   // flags     - enable
        
        // QZSS
        ubxMsg.AddFieldU1(5);   // gnssId    - GNSS identifier (see Satellite Numbering)
        ubxMsg.AddFieldU1(0);   // resTrkCh  - Number of reserved (minimum) tracking channels for this GNSS system
        ubxMsg.AddFieldU1(0);   // maxTrkCh  - Maximum number of tracking channels used for this GNSS system (>=resTrkChn)
        ubxMsg.AddFieldU1(0);   // reserved1 - Reserved
        ubxMsg.AddFieldX4(0);   // flags     - enable
        
        // GLONASS
        ubxMsg.AddFieldU1(6);   // gnssId    - GNSS identifier (see Satellite Numbering)
        ubxMsg.AddFieldU1(0);   // resTrkCh  - Number of reserved (minimum) tracking channels for this GNSS system
        ubxMsg.AddFieldU1(0);   // maxTrkCh  - Maximum number of tracking channels used for this GNSS system (>=resTrkChn)
        ubxMsg.AddFieldU1(0);   // reserved1 - Reserved
        ubxMsg.AddFieldX4(0);   // flags     - enable
        

        uint8_t *buf;
        uint8_t  bufLen;
        ubxMsg.GetBuf(&buf, &bufLen);

        StreamBlob(Serial, buf, bufLen, 0, 1);
        ss.write(buf, bufLen);

        GetMessageOrErr();
    });

    shell.RegisterCommand("cfg-pm2", [](char *cmdStr) {
        Serial.println(F("CFG-PM2 (0x06 0x3B)"));
 
        // updateEPH     will be enabled
        //
        // decide if cyclic or on/off (default to on/off)

                        // 33222222222211111111110000000000
                        // 10987654321098765432109876543210
        uint32_t flags = 0b00000000000000000001000000000000;    // ON/OFF mode, updateEPH enabled

        uint8_t mode = 0;
        
        Str str(cmdStr);

        if (str.TokenCount(' ') >= 2)
        {
            const char *resetTypeStr = str.TokenAtIdx(1, ' ');
            
            if (!strcmp(resetTypeStr, "cyclic"))
            {
                       // 33222222222211111111110000000000
                       // 10987654321098765432109876543210
                flags = 0b00000000000000100001000000000000;    // Cyclic tracking mode, updateEPH enabled
                mode = 1;
            }
        }

        if (mode == 0)
        {
            Serial.println(F("    ON/OFF"));
        }
        else
        {
            Serial.println(F("    Cyclic"));
        }
        
        
        uint32_t updatePeriod = 30000;
        uint32_t searchPeriod = 10000;
        

        ubxMsg.Reset();
       
        // CFG-PM2 (0x06 0x3B)
        ubxMsg.SetClass(0x06);
        ubxMsg.SetId(0x3B);

        ubxMsg.AddFieldU1(1);   // version, must set to 1
        ubxMsg.AddFieldU1(0);   // reserved1
        ubxMsg.AddFieldU1(0);   // reserved2
        ubxMsg.AddFieldU1(0);   // reserved3
        ubxMsg.AddFieldX4(flags);   // flags
        ubxMsg.AddFieldU4(updatePeriod);   // updatePeriod - Position update period. If set to 0, the receiver will never retry a fix
        ubxMsg.AddFieldU4(searchPeriod);   // searchPeriod - Acquisition retry period. If set to 0, the receiver will never retry a startup
        ubxMsg.AddFieldU4(0);   // gridOffset   - Grid offset relative to GPS start of week
        ubxMsg.AddFieldU2(0);   // onTime       - on time after first successful fix
        ubxMsg.AddFieldU2(0);   // minAcqTime   - minimal search time
        ubxMsg.AddFieldU2(0);   // reserved4
        ubxMsg.AddFieldU2(0);   // reserved5
        ubxMsg.AddFieldU4(0);   // reserved6
        ubxMsg.AddFieldU4(0);   // reserved7
        ubxMsg.AddFieldU1(0);   // reserved8
        ubxMsg.AddFieldU1(0);   // reserved9
        ubxMsg.AddFieldU2(0);   // reserved10
        ubxMsg.AddFieldU4(0);   // reserved11

        uint8_t *buf;
        uint8_t  bufLen;
        ubxMsg.GetBuf(&buf, &bufLen);

        StreamBlob(Serial, buf, bufLen, 0, 1);
        ss.write(buf, bufLen);

        GetMessageOrErr();



        // Compare against TT7
        /* ONOFF 30s, doNotEnterOff, updateEPH. */
        //static uint8_t setONOFFoperation_30s[52] = {0xB5, 0x62, 0x06, 0x3B, 0x2C, 0x00, 0x01, 0x06, 0x00, 0x00, 0x00, 0x90, 0x01, 0x01, 0x30, 0x75, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x4F, 0xC1, 0x03, 0x00, 0x87, 0x02, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x64, 0x40, 0x01, 0x00, 0x4F, 0x5E};

        // 0x2C, 0x00, - 44 bytes
        // version - 1
        // reserved1 - 6 (??)
        // reserved2 - 0
        // reserved3 - 0
                                                  // 33222222222211111111110000000000
                                                  // 10987654321098765432109876543210
        // flags        - 0x00, 0x90, 0x01, 0x01 - 0b00000001000000011001000000000000
            // so 24=?, 18=mode=on/off, 16=doNotEnterOff, 15=?, 12=updateEPH
        // updatePeriod - 0x30, 0x75, 0x00, 0x00 = 30,000
        // searchPeriod - 0x10, 0x27, 0x00, 0x00 = 10,000
        // gridOffset   - 0x00, 0x00, 0x00, 0x00 = 0
        // onTime       - 0x00, 0x00 = 0
        // minAcqTime   - 0x00, 0x00 = 0
        // the rest should be reserved, but his code shows differently... different module?
            // 0x2C, 0x01, 0x00, 0x00, 0x4F, 0xC1, 0x03, 0x00, 0x87, 0x02, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x64, 0x40, 0x01, 0x00, 0x4F, 0x5E};
    });

    shell.RegisterCommand("cfg-sbas", [](char *) {
        Serial.println(F("CFG-SBAS (0x06 0x16)"));
        
        ubxMsg.Reset();
        
        // CFG-SBAS (0x06 0x16)
        ubxMsg.SetClass(0x06);
        ubxMsg.SetId(0x16);

        ubxMsg.AddFieldX1(0);   // mode = disabled
        ubxMsg.AddFieldX1(0);   // usage = unclear, probably disabled
        ubxMsg.AddFieldU1(0);   // maxSBAS = no sbas?
        ubxMsg.AddFieldX1(0xFF);   // scanmode2
        ubxMsg.AddFieldX4(0xFFFFFFFF);   // scanmode1

        uint8_t *buf;
        uint8_t  bufLen;
        ubxMsg.GetBuf(&buf, &bufLen);

        StreamBlob(Serial, buf, bufLen, 0, 1);
        ss.write(buf, bufLen);

        GetMessageOrErr();
    });

    shell.RegisterCommand("cfg-rxm", [](char *cmdStr) {
        Serial.println(F("CFG-RXM (0x06 0x11)"));

        ubxMsg.Reset();

        // CFG-RXM (0x06 0x11)
        ubxMsg.SetClass(0x06);
        ubxMsg.SetId(0x11);

        uint8_t lowPowerMode = 1;

        Str str(cmdStr);

        if (str.TokenCount(' ') >= 2)
        {
            const char *lowPowerModeStr = str.TokenAtIdx(1, ' ');

            lowPowerMode = atoi(lowPowerModeStr);
        }

        if (lowPowerMode == 1)
        {
            Serial.println(F("    Power Save Mode"));
        }
        else
        {
            Serial.println(F("    Other Mode"));
        }

        ubxMsg.AddFieldU1(8);              // reserved1 - always set to 8
        ubxMsg.AddFieldU1(lowPowerMode);   // LowPowerMode, 0 = Continuous; 1 = PowerSaveMode

        uint8_t *buf;
        uint8_t  bufLen;
        ubxMsg.GetBuf(&buf, &bufLen);

        StreamBlob(Serial, buf, bufLen, 0, 1);
        ss.write(buf, bufLen);

        GetMessageOrErr();
    });


    shell.RegisterCommand("cfg-rxm-poll", [](char *) {
        Serial.println(F("CFG-RXM (0x06 0x11) POLL"));

        ubxMsg.Reset();

        // CFG-RXM (0x06 0x11)
        ubxMsg.SetClass(0x06);
        ubxMsg.SetId(0x11);

        uint8_t *buf;
        uint8_t  bufLen;
        ubxMsg.GetBuf(&buf, &bufLen);

        StreamBlob(Serial, buf, bufLen, 0, 1);
        ss.write(buf, bufLen);

        GetMessageOrErr();
    });




    
    shell.RegisterCommand("rxm-pmreq", [](char *) {
        Serial.println(F("RXM-PMREQ (0x02 0x41)"));
        
        ubxMsg.Reset();

        // RXM-PMREQ (0x02 0x41)
        ubxMsg.SetClass(0x02);
        ubxMsg.SetId(0x41);

        ubxMsg.AddFieldU4(0);   // duration, 0 = infinite
        ubxMsg.AddFieldX4(2);   // bitfield, backup enabled

        uint8_t *buf;
        uint8_t  bufLen;
        ubxMsg.GetBuf(&buf, &bufLen);

        StreamBlob(Serial, buf, bufLen, 0, 1);
        ss.write(buf, bufLen);

        GetMessageOrErr();
    });

    

    shell.RegisterErrorHandler([](char *cmdStr) {
        Serial.print("ERR: Unrecognized \"");
        Serial.print(cmdStr);
        Serial.print("\"");
        Serial.println();
    });
    
    while (1)
    {
        shell.Run();
    }
}

void loop() {
  // put your main code here, to run repeatedly:

}
