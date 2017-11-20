#ifndef __APP_HAB_TRACKER_1_SECONDARY_H__
#define __APP_HAB_TRACKER_1_SECONDARY_H__



#include "Container.h"
#include "Evm.h"
#include "RFLink.h"



/*
 * Designed to:
 * - watch the serial output of the primary tracker
 * - decode relevant fields and cache
 * - transmit that data periodically
 *
 */



struct AppHABTracker1SecondaryConfig
{
    uint8_t   pinRfTx;
    uint32_t  reportIntervalMs;
};


class AppHABTracker1Secondary
{
    static const uint8_t C_IDLE  =  0;
    static const uint8_t C_TIMED = 10;
    static const uint8_t C_INTER =  0;
    
    static const uint8_t RF_REALM    = 2;
    static const uint8_t RF_SRC_ADDR = 1;
    static const uint8_t RF_DST_ADDR = 2;
    static const uint8_t PROTOCOL_ID = 2;
    
    static const uint8_t BUF_BYTES = 200;
    
    struct PayloadBuffer
    {
        char timeStr[6];    // 191148
        char latStr[8];     // 4044.23N
        char lngStr[9];     // 07402.04W
        char altStr[6];     // 000123
    };

    
public:

    AppHABTracker1Secondary(AppHABTracker1SecondaryConfig &cfg)
    : cfg_(cfg)
    {
        // Nothing to do
    }

    void Run()
    {
        Serial.begin(9600);
        Serial.println("Starting");
        
        rfLink_.SetRealm(RF_REALM);
        rfLink_.SetSrcAddr(RF_SRC_ADDR);
        rfLink_.SetDstAddr(RF_DST_ADDR);
        rfLink_.Init(-1, cfg_.pinRfTx);
        
        ied_.SetCallback([this](){ OnSerialPoll(); });
        ied_.RegisterForIdleTimeEvent();
        
        tedTransmit_.SetCallback([this](){ OnTransmit(); });
        tedTransmit_.RegisterForTimedEventInterval(cfg_.reportIntervalMs);
        
        evm_.MainLoop();
    }

private:

    void OnSerialPoll()
    {
        while (Serial.available())
        {
            uint8_t b = Serial.read();
            Serial.write(b);
            AddByteToQueue(b);
            
            if (b == '\n')
            {
                GetBytesFromQueue( 1, payloadBuffer_.timeStr, 6);
                GetBytesFromQueue( 8, payloadBuffer_.latStr,  8);
                GetBytesFromQueue(17, payloadBuffer_.lngStr,  9);
                GetBytesFromQueue(37, payloadBuffer_.altStr,  6);
                
                PrintPayload();
                
                // blow away buffer, either not the line we want or we just used
                // it.
                q_.Clear();
            }
        }
    }
    
    void GetBytesFromQueue(uint8_t  idxStart,
                           char    *buf,
                           uint8_t  bufSize)
    {
        for (uint8_t i = 0; i < bufSize; ++i)
        {
            buf[i] = q_[idxStart + i];
        }
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
    
    void PrintPayload()
    {
        Serial.print("Time: "); Serial.write(payloadBuffer_.timeStr, 6); Serial.println();
        Serial.print("Lat : "); Serial.write(payloadBuffer_.latStr, 8); Serial.println();
        Serial.print("Lng : "); Serial.write(payloadBuffer_.lngStr, 9); Serial.println();
        Serial.print("Alt : "); Serial.write(payloadBuffer_.altStr, 6); Serial.println();
    }

    void OnTransmit()
    {
        Serial.println("Transmitting");
        PrintPayload();
        
        rfLink_.Send(PROTOCOL_ID,
                     (uint8_t *)&payloadBuffer_,
                     sizeof(payloadBuffer_));
    }

    Evm::Instance<C_IDLE, C_TIMED, C_INTER> evm_;
    
    AppHABTracker1SecondaryConfig &cfg_;
    
    IdleTimeEventHandlerDelegate ied_;
    Queue<uint8_t, BUF_BYTES>    q_;

    PayloadBuffer              payloadBuffer_;
    TimedEventHandlerDelegate  tedTransmit_;
    RFLink rfLink_;
};






#endif  // __APP_HAB_TRACKER_1_SECONDARY_H__