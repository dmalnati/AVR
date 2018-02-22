#ifndef __APP_PICO_TRACKER_1_H__
#define __APP_PICO_TRACKER_1_H__


#include "Evm.h"
#include "LedBlinker.h"
#include "AppPicoTracker1UserConfigManager.h"
#include "SensorGPSUblox.h"


#include "UtlStreamBlob.h"

struct AppPicoTracker1Config
{
    // GPS
    uint8_t pinGpsEnable;
    uint8_t pinGpsSerialRx; // receive GPS data from this pin
    uint8_t pinGpsSerialTx; // send data to the GPS on this pin
    
    // Status LEDs
    uint8_t pinLedRunning;
    uint8_t pinLedGpsLocked;
    uint8_t pinLedTransmitting;
};

class AppPicoTracker1
{
private:
    static const uint32_t DURATION_MS_CHECK_SERIAL_INPUT_ACTIVE = 5000;
    
    static const uint32_t DURATION_MS_LED_RUNNING_OFF_NORMAL = 4950;
    static const uint32_t DURATION_MS_LED_RUNNING_ON_NORMAL  =   50;
    
    static const uint32_t DURATION_MS_LED_RUNNING_OFF_SERIAL = 950;
    static const uint32_t DURATION_MS_LED_RUNNING_ON_SERIAL  =  50;
    
    static const uint8_t PIN_SERIAL_RX = 2;
    
private:
    static const uint8_t C_IDLE  = 10;
    static const uint8_t C_TIMED = 10;
    static const uint8_t C_INTER = 10;
    
public:

    
public:
    AppPicoTracker1(AppPicoTracker1Config &cfg)
    : cfg_(cfg)
    , serIface_(*this)
    , serialInputActive_(0)
    , ledBlinkerRunning_(cfg_.pinLedRunning)
    , gps_(cfg_.pinGpsSerialRx, cfg_.pinGpsSerialTx)
    {
        // Nothing to do
    }
    
    void Run()
    {
        Serial.begin(9600);
        Serial.println(F("Starting"));
        
        // Set up GPS enable pin
        PAL.PinMode(cfg_.pinGpsEnable, OUTPUT);
        
        // Set up status LED pins
        PAL.PinMode(cfg_.pinLedRunning,      OUTPUT);
        PAL.PinMode(cfg_.pinLedGpsLocked,    OUTPUT);
        PAL.PinMode(cfg_.pinLedTransmitting, OUTPUT);
        
        if (AppPicoTracker1UserConfigManager::GetUserConfig(userConfig_))
        {
            Serial.println();
            Serial.println(F("Proceeding with:"));
            userConfig_.Print();
            Serial.println();
            
            
            // Drive state machine
            OnEvent(Event::START);


            
            Serial.println(F("Running."));
            Serial.println();
            PAL.Delay(1000);
            
            // Handle async events
            evm_.MainLoopLowPower();
        }
        else
        {
            Serial.println();
            Serial.println(F("ERR: Invalid configuration, please restart and try again."));
            Serial.println();
            
            // blink error -- cannot continue without config
            while (1)
            {
                const uint32_t delayMs = 500;
                
                PAL.DigitalWrite(cfg_.pinLedRunning, HIGH);
                PAL.Delay(delayMs);
                PAL.DigitalWrite(cfg_.pinLedRunning, LOW);
                PAL.DigitalWrite(cfg_.pinLedGpsLocked, HIGH);
                PAL.Delay(delayMs);
                PAL.DigitalWrite(cfg_.pinLedGpsLocked, LOW);
                PAL.DigitalWrite(cfg_.pinLedTransmitting, HIGH);
                PAL.Delay(delayMs);
                PAL.DigitalWrite(cfg_.pinLedTransmitting, LOW);
            }
        }
    }

private:

    enum class Event : uint8_t
    {
        START,
        SERIAL_INPUT_ACTIVE,
        SERIAL_INPUT_INACTIVE,
    };

    void OnEvent(Event evt)
    {
        if (evt == Event::START)
        {
            // set up blinking
            ledBlinkerRunning_.SetDurationOffOn(DURATION_MS_LED_RUNNING_OFF_NORMAL,
                                                DURATION_MS_LED_RUNNING_ON_NORMAL);
            ledBlinkerRunning_.Start();
            
            // Watch for serial input, check immediately
            tedSerialMonitor_.SetCallback([this](){
                OnCheckForSerialInputActive();
            });
            tedSerialMonitor_.RegisterForTimedEventInterval(DURATION_MS_CHECK_SERIAL_INPUT_ACTIVE);
            tedSerialMonitor_();
        }
        else if (evt == Event::SERIAL_INPUT_ACTIVE)
        {
            Serial.println(F("Serial active"));
            
            // as long as serial input active, we need to be able to read from
            // it, so disable power saving
            evm_.LowPowerDisable();
            
            // run at different interval
            ledBlinkerRunning_.SetDurationOffOn(DURATION_MS_LED_RUNNING_OFF_SERIAL,
                                                DURATION_MS_LED_RUNNING_ON_SERIAL);
                                                
            // since uart active, can monitor for input
            serIface_.Start();
        }
        else if (evt == Event::SERIAL_INPUT_INACTIVE)
        {
            Serial.println(F("Serial inactive"));
            PAL.Delay(50);
            
            // serial input no longer active, enable power saving
            evm_.LowPowerEnable();
            
            // run at different interval
            ledBlinkerRunning_.SetDurationOffOn(DURATION_MS_LED_RUNNING_OFF_NORMAL,
                                                DURATION_MS_LED_RUNNING_ON_NORMAL);
            
            
            // no point in this polling anymore, no serial data will be received
            // since the uart isn't active in low power mode
            serIface_.Stop();
        }
    }
    
    
    void TrackerSequence()
    {
        // async
        // blink GPS led until locked
        
        
        // enable GPS power
        // enable high altitude mode
        
        
        // async
        // wait for GPS lock
        
        // keep GPS led lit
        
        // command GPS to save state
        // cut off power to GPS
        
        
        // build message
        
        
        // tx message
        
        
        // turn off GPS LED
        
        
    }
    
    
    
    void OnReportIntervalTimeout()
    {
        // blink 
        
        
        
    }
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    


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
    SoftwareSerial &ss = gps_.DebugGetSS();
    static const uint8_t UBX_IN_BUF_SIZE = 100;
    static uint8_t ubxInBuf[UBX_IN_BUF_SIZE];
    
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
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    void StartGPS()
    {
        // enable power supply to GPS
        PAL.DigitalWrite(cfg_.pinGpsEnable, HIGH);
        
        // re-init to begin cycle again
        gps_.Init();
        
        // assert this is a high-altitude mode
        gps_.SetHighAltitudeMode();
        
        GetMessageOrErr();
    }
    
    
    void StopGPS()
    {
        // stop interrups from firing in underlying code
        gps_.DisableSerialInput();
        
        // cause the gps module to store the metadata is has learned from
        // the satellites it can see and used to get a lock.
        // this will be read again automatically by the module on startup.
        gps_.SaveConfiguration();
        
        // disable power supply to GPS
        // (battery backup for module-stored data supplied through other pin)
        PAL.DigitalWrite(cfg_.pinGpsEnable, LOW);
    }
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    


    void OnCheckForSerialInputActive()
    {
        // found 2 empirically, added 1 for margin
        const uint32_t delayMsToWaitForPinToSettle = 2 + 1;
        
        PAL.Delay(delayMsToWaitForPinToSettle);
        
        uint8_t serialInputActive = PAL.DigitalRead(PIN_SERIAL_RX);
        
        if (serialInputActive == 1 && serialInputActive_ == 0)
        {
            // active now but wasn't before
            OnEvent(Event::SERIAL_INPUT_ACTIVE);
        }
        else if (serialInputActive == 0 && serialInputActive_ == 1)
        {
            // not active but was before
            OnEvent(Event::SERIAL_INPUT_INACTIVE);
        }
        
        serialInputActive_ = serialInputActive;
    }


    
    void ToDo()
    {
        
        // Change SSID to 11, not 9.
            // I think I set the icon display based on other characters in the
            // aprs message.
                // easy to expose to settings?  dad might like that.
        
        // APRS route should be WIDE2-1 not WIDE1-1, right?
        
        
        // wtf were the bugs I had before?
            // Something about the deg min sec not accounting for 00 and
            // turning into just spaces right?
            // internal voltage wasn't working correctly
            
        // GPS geofencing to change frequency
        
        // Make sure to manually enable high-altitude mode
        
        // use LEDs as status
            // different schemes for config vs low-altitude status
            // disable when flying
        
        // set serial active checking frequency more appropriately
        
        // watchdog
        
        // RTC
        
        // dump flash stored locations if found later
        
        
        // restore fuse setting to erase EEPROM on re-program
            // temporarily disabled for development
            
        // restore user config being sensitive to RX-in
    }
    
    
    
private:

    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;

    AppPicoTracker1Config &cfg_;
    
    #include "AppPicoTracker1SerialInterface.h"
    AppPicoTracker1SerialInterface serIface_;
    
    AppPicoTracker1UserConfigManager::UserConfig userConfig_;
    
    TimedEventHandlerDelegate tedSerialMonitor_;
    uint8_t                   serialInputActive_;
    
    LedBlinker  ledBlinkerRunning_;
    
    
    
    
    
    SensorGPSUblox               gps_;
    SensorGPSUblox::Measurement  gpsMeasurement_;
    
    
    
    
    
};









#endif  // __APP_PICO_TRACKER_1_H__




























