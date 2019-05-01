#include "PAL.h"
#include "SerialInput.h"
#include "Evm.h"
#include "Log.h"
#include "LogBlob.h"
#include "UbxMessage.h"
#include "SensorGPSUblox.h"


/*
 * Intention is to reduce code size on runtime code by pre-processing messages to the GPS.
 * 
 * The UbxMessage class is good for safely constructing messages.
 * However, the code size can be reduced at runtime by extracting raw bytes to send here.
 * 
 */

static Evm::Instance<0,5,0> evm;
static SerialAsyncConsoleEnhanced<20,20>  console;

static const uint8_t  MAX_UBX_MESSAGE_SIZE  = 44;
static UbxMessage<MAX_UBX_MESSAGE_SIZE> ubxMessage_;





void GenCpp(const char *function, uint8_t *buf, uint8_t bufLen)
{
    uint8_t  bufLenRemaining = bufLen;
    uint8_t *p               = buf;
    
    Log(function);
    LogNL();

    Log(P("uint8_t bufLen = "), bufLen, ";");
    Log(P("uint8_t buf[] = {"));

    while (bufLenRemaining)
    {
        uint8_t bytesToPrint = bufLenRemaining >= 8 ? 8 : bufLenRemaining;
        
        LogNNL("    ");
        for (uint8_t i = 0; i < bytesToPrint; ++i)
        {
            printf("0x%02X, ", *p);
            
            ++p;
        }
        LogNL();
        
        bufLenRemaining -= bytesToPrint;
    }
    
    Log(P("};"));
    LogNL(2);
}


void SetHighAltitudeMode()
{
    ubxMessage_.Reset();
    
    // Create CFG-NAV5 msg
    // Set altitude mode to Airborne < 1g
    // Indicate this is the only parameter changing via bitmap
    ubxMessage_.SetClass(0x06);
    ubxMessage_.SetId(0x24);
    
    ubxMessage_.AddFieldX2(0x0001); // mask     = dynModel change only
    ubxMessage_.AddFieldU1(6);      // dynModel = airborne < 1g
    ubxMessage_.AddFieldU1(0);      // fixMode
    ubxMessage_.AddFieldI4(0);      // fixedAlt
    ubxMessage_.AddFieldU4(0);      // fixedAltVar
    ubxMessage_.AddFieldI1(0);      // minElev
    ubxMessage_.AddFieldU1(0);      // drLimit
    ubxMessage_.AddFieldU2(0);      // pDop
    ubxMessage_.AddFieldU2(0);      // tDop
    ubxMessage_.AddFieldU2(0);      // pAcc
    ubxMessage_.AddFieldU2(0);      // tAcc
    ubxMessage_.AddFieldU1(0);      // staticHoldThresh
    ubxMessage_.AddFieldU1(0);      // dgpsTimeOut
    ubxMessage_.AddFieldU4(0);      // reserved2
    ubxMessage_.AddFieldU4(0);      // reserved3
    ubxMessage_.AddFieldU4(0);      // reserved4
    
    uint8_t *buf;
    uint8_t  bufLen;
    ubxMessage_.GetBuf(&buf, &bufLen);
    
    GenCpp("SetHighAltitudeMode", buf, bufLen);
}

#if 0
// purposefully don't support this anymore, what's the need?
void SetMessageInterval(uint16_t intervalMs)
{
    ubxMessage_.Reset();
    
    // Create CFG-RATE msg
    ubxMessage_.SetClass(0x06);
    ubxMessage_.SetId(0x08);
    
    ubxMessage_.AddFieldU2(intervalMs); // measRate
    ubxMessage_.AddFieldU2(1);          // navRate
    ubxMessage_.AddFieldU2(1);          // timeRef
    
    uint8_t *buf;
    uint8_t  bufLen;
    ubxMessage_.GetBuf(&buf, &bufLen);
    
    GenCpp("SetMessageInterval", buf, bufLen);
}
#endif

#if 0
// purposefully don't support, just let defaults rule
void EnableOnlyGGAAndRMC()
{
    // These are the messages broadcast by default.
    // Specifically enable and disable to be certain.
    uint8_t ubxClassUbxIdRateList[] = {
        0xF0, 0x00, 1,  // GGA
        0xF0, 0x01, 0,  // GLL
        0xF0, 0x02, 0,  // GSA
        0xF0, 0x03, 0,  // GSV
        0xF0, 0x04, 1,  // RMC
        0xF0, 0x05, 0   // VTG
    };
    
    uint8_t ubxClassUbxIdRateListLen = sizeof(ubxClassUbxIdRateList);
    
    for (uint8_t i = 0; i < ubxClassUbxIdRateListLen; i += 3)
    {
        uint8_t ubxClass = ubxClassUbxIdRateList[i + 0];
        uint8_t ubxId    = ubxClassUbxIdRateList[i + 1];
        uint8_t rate     = ubxClassUbxIdRateList[i + 2];
        
        SetMessageRate(ubxClass, ubxId, rate);
    }
}
#endif

void SaveConfiguration()
{
    ubxMessage_.Reset();

    // CFG-CFG (0x06 0x09)
    ubxMessage_.SetClass(0x06);
    ubxMessage_.SetId(0x09);

    ubxMessage_.AddFieldX4(0);           // clearMask  - clear nothing
    ubxMessage_.AddFieldX4(0x0000FFFF);  // saveMask   - save everything
    ubxMessage_.AddFieldX4(0);           // loadMask   - load nothing
    ubxMessage_.AddFieldX1(1);           // deviceMask - save to batter-backed ram (internal to chip)

    uint8_t *buf;
    uint8_t  bufLen;
    ubxMessage_.GetBuf(&buf, &bufLen);

    GenCpp("SaveConfiguration", buf, bufLen);
}

// Default to the hardest reset possible, complete expunging of all data
void ResetModule(SensorGPSUblox::ResetType resetType = SensorGPSUblox::ResetType::COLD,
                 SensorGPSUblox::ResetMode resetMode = SensorGPSUblox::ResetMode::HW)
{
    ubxMessage_.Reset();

    // CFG-RST (0x06 0x04)
    ubxMessage_.SetClass(0x06);
    ubxMessage_.SetId(0x04);

    ubxMessage_.AddFieldX2((uint16_t)resetType);
    ubxMessage_.AddFieldU1((uint8_t)resetMode);
    ubxMessage_.AddFieldU1(0);   // reserved

    uint8_t *buf;
    uint8_t  bufLen;
    ubxMessage_.GetBuf(&buf, &bufLen);

    GenCpp("ResetModule", buf, bufLen);
}

#if 0
// purposefully not supporting, let defaults rule
void SetMessageRate(uint8_t ubxClass, uint8_t ubxId, uint8_t rate)
{
    ubxMessage_.Reset();
    
    // Create CFG-MSG msg
    ubxMessage_.SetClass(0x06);
    ubxMessage_.SetId(0x01);
    
    ubxMessage_.AddFieldU1(ubxClass);   // msgClass
    ubxMessage_.AddFieldU1(ubxId);      // msgID
    ubxMessage_.AddFieldU1(rate);       // rate
    
    uint8_t *buf;
    uint8_t  bufLen;
    ubxMessage_.GetBuf(&buf, &bufLen);
}
#endif




void setup()
{
    LogStart(9600);
    Log("Started");
    
    LogNL(3);

    SetHighAltitudeMode();
    SaveConfiguration();
    ResetModule();  // purposefully only supporting COLD : HW reset

    //console.Start();
    evm.MainLoop();
}




void loop() {}





































