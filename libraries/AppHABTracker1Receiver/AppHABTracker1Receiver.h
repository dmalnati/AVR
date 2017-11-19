#ifndef __APP_HAB_TRACKER_1_RECEIVER_H__
#define __APP_HAB_TRACKER_1_RECEIVER_H__


#include "Container.h"
#include "Evm.h"
#include "TimedEventHandler.h"
#include "IdleTimeEventHandler.h"
#include "RFLink.h"
#include "LCDFrentaly20x4.h"


struct AppHABTracker1ReceiverConfig
{
    // APRS Modem configuration
    uint32_t baud;
    
    const char *callsign;
    
    // 433MHz configuration
    uint8_t pinRfRx;
    
    // Circuit configuration
    uint8_t  i2cAddrLcd;
};

class AppHABTracker1Receiver
{
    static const uint8_t C_IDLE  = 10;
    static const uint8_t C_TIMED = 10;
    static const uint8_t C_INTER = 0;
    
    static const uint8_t RF_REALM    = 2;
    static const uint8_t RF_SRC_ADDR = 2;
    static const uint8_t RF_DST_ADDR = 1;
    static const uint8_t PROTOCOL_ID = 2;
    
    static const uint32_t ONE_SECOND_IN_MS = 1000;
    
    static const uint8_t BUF_BYTES = 200;

    struct PayloadBuffer
    {
        char timeStr[6] = { 0 };    // 191148
        char latStr[8]  = { 0 };     // 4044.23N
        char lngStr[9]  = { 0 };     // 07402.04W
        char altStr[6]  = { 0 };     // 000123
    };
    
    struct SourceState
    {
        SourceState(const char *srcNameInput)
        {
            srcName = srcNameInput;
        }
        
        const char *srcName;
        
        PayloadBuffer payloadBuffer;
        
        uint32_t secsSinceLast = 0;
        uint32_t filteredCount = 0;
    };
    
    enum class ActiveSource : uint8_t
    {
        PRIMARY,
        SECONDARY
    };


public:
    
    AppHABTracker1Receiver(AppHABTracker1ReceiverConfig &cfg)
    : cfg_(cfg)
    , lcd_(cfg_.i2cAddrLcd)
    , activeSource_(ActiveSource::PRIMARY)
    , sourceStateSerial_("PRI")
    , sourceStateRf_("SEC")
    {
        // Nothing to do
    }
    
    void Run()
    {
        Serial.begin(cfg_.baud);
        Serial.println("Starting");
        
        // Set up LCD
        lcd_.Init();
        lcd_.PrintAt( 8, 1, ",");
        lcd_.PrintAt(13, 2, "(0    )");
        lcd_.PrintAt( 0, 3, "(0    )");
        lcd_.PrintAt(13, 3, "(0    )");
        OnPrimaryDataRefreshed();
        
        // Set up primary source
        ied_.SetCallback([this](){ OnSerialPoll(); });
        ied_.RegisterForIdleTimeEvent();
        
        // Set up secondary source
        rfLink_.SetRealm(RF_REALM);
        rfLink_.SetSrcAddr(RF_SRC_ADDR);
        rfLink_.SetDstAddr(RF_DST_ADDR);
        rfLink_.SetOnMessageReceivedCallback([this](RFLinkHeader *hdr,
                                                    uint8_t      *buf,
                                                    uint8_t       bufLen) {
            OnMsgRcv(hdr, buf, bufLen);
        });
        rfLink_.Init(cfg_.pinRfRx, -1);
        
        // Set up time keeping
        ted_.SetCallback([this](){ OnSecondInterval(); });
        ted_.RegisterForTimedEventInterval(ONE_SECOND_IN_MS);
        
        // Begin
        evm_.MainLoop();
    }
    


private:


    ////////////////////////////////////////////////////////////////////////////
    //
    // Primary data sourcing
    //
    ////////////////////////////////////////////////////////////////////////////



    // String you're looking for:
    // SRC: [KD2KDD-9] DST: [APZ001-0] PATH: [WIDE1-1] DATA: /001221h2400.00S/18000.00EO359/999/A=999999      -& %,. ! ! )$,#"  "$
    //
    // The part we care about is:
    // 001221h2400.00S/18000.00E
    // 001221h   = time
    // 2400.00S  = latitude
    // 18000.00E = longitude
    // A=999999  = altitude
    
    void OnSerialPoll()
    {
        while (Serial.available())
        {
            uint8_t b = Serial.read();
            Serial.write(b);
            AddByteToQueue(b);
            
            if (b == '\n')
            {
                static const uint8_t BUF_SIZE = 20;
                char buf[BUF_SIZE] = { 0 };
                
                // Now look for whether this line looks like what we want
                GetBytesFromQueueAsString(0, buf, 3);
                if (!strcmp(buf, "SRC"))
                {
                    // this is the right line, confirm from me
                    
                    GetBytesFromQueueAsString(6, buf, strlen(cfg_.callsign));
                    if (!strcmp(buf, cfg_.callsign))
                    {
                        GetBytesFromQueueAsString(55, buf, 6);
                        memcpy(sourceStateSerial_.payloadBuffer.timeStr, buf, 6);
                        
                        GetBytesFromQueueAsString(62, buf, 8);
                        memcpy(sourceStateSerial_.payloadBuffer.latStr, buf, 8);
                        
                        GetBytesFromQueueAsString(71, buf, 9);
                        memcpy(sourceStateSerial_.payloadBuffer.lngStr, buf, 9);
                        
                        GetBytesFromQueueAsString(91, buf, 6);
                        memcpy(sourceStateSerial_.payloadBuffer.altStr, buf, 6);
                        
                        sourceStateSerial_.secsSinceLast = 0;
                        
                        SetSourcePrimary();
                    }
                    else
                    {
                        ++sourceStateSerial_.filteredCount;
                        
                        OnPrimaryDataRefreshed();
                    }
                    
                }
                
                // blow away buffer, either not the line we want or we just used
                // it.
                q_.Clear();
            }
        }
    }
    
    void GetBytesFromQueueAsString(uint8_t  idxStart,
                                   char    *buf,
                                   uint8_t  bufSize)
    {
        for (uint8_t i = 0; i < bufSize; ++i)
        {
            buf[i] = q_[idxStart + i];
        }
        
        buf[bufSize] = '\0';
    }
    
    void AddByteToQueue(uint8_t b)
    {
        if (!q_.Push(b))
        {
            uint8_t tmp;
            
            q_.Pop(tmp);
            q_.Push(b);
        }
    }
    
    
    ////////////////////////////////////////////////////////////////////////////
    //
    // Secondary data sourcing
    //
    ////////////////////////////////////////////////////////////////////////////
    
    void OnMsgRcv(RFLinkHeader * /*hdr*/, uint8_t *buf, uint8_t bufLen)
    {
        // Data received over RF, make sure it's the right size and take a copy
        if (bufLen == sizeof(sourceStateRf_.payloadBuffer))
        {
            memcpy((uint8_t *)&sourceStateRf_.payloadBuffer, buf, bufLen);
            
            sourceStateRf_.secsSinceLast = 0;
            
            SetSourceSecondary();
        }
        else
        {
            ++sourceStateRf_.filteredCount;
            
            OnSecondaryDataRefreshed();
        }
    }
    
    ////////////////////////////////////////////////////////////////////////////
    //
    // Time Keeping
    //
    ////////////////////////////////////////////////////////////////////////////

    void OnSecondInterval()
    {
        ++sourceStateSerial_.secsSinceLast;
        if (sourceStateSerial_.secsSinceLast > 99999)
        {
            sourceStateSerial_.secsSinceLast = 0;
        }
        
        ++sourceStateRf_.secsSinceLast;
        if (sourceStateSerial_.secsSinceLast > 99999)
        {
            sourceStateSerial_.secsSinceLast = 0;
        }
        
        OnAgeDataRefreshed();
    }
    
    
    ////////////////////////////////////////////////////////////////////////////
    //
    // Source Switching
    //
    ////////////////////////////////////////////////////////////////////////////

    void SetSourcePrimary()
    {
        activeSource_ = ActiveSource::PRIMARY;
        
        OnPrimaryDataRefreshed();
    }
    
    void SetSourceSecondary()
    {
        activeSource_ = ActiveSource::SECONDARY;
        
        OnSecondaryDataRefreshed();
    }
    
    ////////////////////////////////////////////////////////////////////////////
    //
    // LCD Redraw
    //
    ////////////////////////////////////////////////////////////////////////////
    
    void OnPrimaryDataRefreshed()
    {
        if (activeSource_ == ActiveSource::PRIMARY)
        {
            PrintStateToLcd(sourceStateSerial_);
        }
        
        OnAgeDataRefreshed();
    }
    
    void OnSecondaryDataRefreshed()
    {
        if (activeSource_ == ActiveSource::SECONDARY)
        {
            PrintStateToLcd(sourceStateRf_);
        }
        
        OnAgeDataRefreshed();
    }
    
    void OnAgeDataRefreshed()
    {
        PrintAge(sourceStateSerial_.secsSinceLast,
                 sourceStateRf_.secsSinceLast);
    }

    
    void PrintStateToLcd(SourceState &s)
    {
        char buf[20];
        
        PayloadBuffer &b = s.payloadBuffer;
        
        memcpy(buf, b.timeStr, 6);
        buf[6] = '\0';
        PrintTime(buf);
        
        PrintSource(s.srcName);
        
        memcpy(buf, b.latStr, 8);
        buf[8] = '\0';
        PrintLat(buf);
        
        memcpy(buf, b.lngStr, 9);
        buf[9] = '\0';
        PrintLng(buf);
        
        memcpy(buf, b.altStr, 6);
        buf[6] = '\0';
        PrintAlt(buf);
        
        PrintFiltered(s.filteredCount);
    }

    
    /*
     * LCD display map
     *
     * Line 1: <time> <source>
     * Line 2: <lat> <lng>
     * Line 3: <alt> <filtered>
     * Line 4: <primarySecs> <secondarySecs>
     *
     * <source> = PRI or SEC
     *
     * 00000000001111111111
     * 01234567890123456789
     *                     
     * hhmmss           SRC
     * 2400.00S, 18000.00E 
     * aaaaaa       (fffff)
     * (sssss)      (sssss)
     */

    void PrintTime(const char *str)
    {
        lcd_.PrintAt(0, 0, str);
    }
    
    void PrintSource(const char *str)
    {
        lcd_.PrintAt(17, 0, str);
    }
    
    void PrintLat(const char *str)
    {
        lcd_.PrintAt(0, 1, str);
    }
    
    void PrintLng(const char *str)
    {
        lcd_.PrintAt(10, 1, str);
    }
    
    void PrintAlt(const char *str)
    {
        lcd_.PrintAt(0, 2, str);
    }
    
    void PrintFiltered(uint32_t filteredCount)
    {
        lcd_.PrintAt(14, 2, "    ");
        lcd_.PrintAt(14, 2, filteredCount);
    }
    
    void PrintAge(uint32_t primarySecs, uint32_t secondarySecs)
    {
        // Primary
        lcd_.PrintAt(1, 3, "     ");
        lcd_.PrintAt(1, 3, primarySecs);
        
        // Secondary
        lcd_.PrintAt(14, 3, "     ");
        lcd_.PrintAt(14, 3, secondarySecs);
    }

    
    
private:
    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;
    
    AppHABTracker1ReceiverConfig &cfg_;

    LCDFrentaly20x4 lcd_;
    
    TimedEventHandlerDelegate ted_;
    IdleTimeEventHandlerDelegate ied_;
    
    ActiveSource activeSource_;
    
    Queue<uint8_t, BUF_BYTES> q_;
    SourceState sourceStateSerial_;
    
    RFLink       rfLink_;
    SourceState  sourceStateRf_;
};





#endif  // __APP_HAB_TRACKER_1_RECEIVER_H__



