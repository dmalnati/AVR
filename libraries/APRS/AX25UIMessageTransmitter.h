#ifndef __AX_25_UI_MESSAGE_TRANSMITTER_H__
#define __AX_25_UI_MESSAGE_TRANSMITTER_H__


#include "PAL.h"
#include "Function.h"
#include "AX25UIMessage.h"
#include "ModemBell202.h"


/*
 *
 * Default sizing of buffer will reflect:
 * - Single routing path
 * - Information field capable of max APRS Position Report Message size
 *
 * This leads to a buffer of 95 bytes.
 *
 * At 1200 baud, and an average of 8.5 bits per byte (due to stuffing, made up),
 * transmission of actual non-flag data would take:
 * 1000 / 1200 * 95 * 8.5 = 672.92ms, so call it 700ms
 *
 * Flags, if left as defaults, will add another ~600ms.
 *
 * Total transmission time including flags therefore is ~1300ms.
 *
 */
template <typename ModemBell202Type     = ModemBell202Pwm,
          uint8_t NUM_ROUTING_PATHS     =  1,
          uint8_t INFORMATION_FIELD_LEN = 70>
class AX25UIMessageTransmitter
{
    public:
    static const uint8_t BUF_SIZE = AX25UIMessage::MIN_BUF_SIZE +
                                    (7 * NUM_ROUTING_PATHS)     +
                                    INFORMATION_FIELD_LEN;
    
    static const uint8_t  DEFAULT_TRANSMIT_COUNT             = 1;
    static const uint32_t DEFAULT_DELAY_MS_BETWEEN_TRANSMITS = 6000;
    static const uint32_t DEFAULT_DURATION_MS_FLAG_START     =  500;
    static const uint32_t DEFAULT_DURATION_MS_FLAG_END       =    1;
    
    
public:

    AX25UIMessageTransmitter()
    : transmitCount_(DEFAULT_TRANSMIT_COUNT)
    , delayMsBetweenTransmits_(DEFAULT_DELAY_MS_BETWEEN_TRANSMITS)
    {
        SetFlagStartDurationMs(DEFAULT_DURATION_MS_FLAG_START);
        SetFlagEndDurationMs(DEFAULT_DURATION_MS_FLAG_END);
    }

    void Init(function<void()> fnBeforeModemStart,
              function<void()> fnAfterModemEnd)
    {
        fnBeforeModemStart_ = fnBeforeModemStart;
        fnAfterModemEnd_    = fnAfterModemEnd;

        msg_.Init(buf_, BUF_SIZE);
        modem_.Init();
    }
    
    AX25UIMessage *GetAX25UIMessage()
    {
        msg_.Reset();
        
        return &msg_;
    }
    
    void SetRadioWarmupDurationMs(uint32_t radioWarmupDurationMs)
    {
        radioWarmupDurationMs_ = radioWarmupDurationMs;
    }
    
    void SetFlagStartDurationMs(uint32_t durationMs)
    {
        const uint8_t durationMs8FlagPack = 56;
        
        flagPackStartCount_ = (durationMs / durationMs8FlagPack) + 1;
    }
    
    void SetFlagEndDurationMs(uint32_t durationMs)
    {
        const uint8_t durationMs8FlagPack = 56;
        
        flagPackEndCount_ = (durationMs / durationMs8FlagPack) + 1;
    }
    
    void SetTransmitCount(uint8_t transmitCount)
    {
        transmitCount_ = transmitCount ? transmitCount : 1;
    }
    
    void SetDelayMsBetweenTransmits(uint32_t delayMsBetweenTransmits)
    {
        delayMsBetweenTransmits_ = delayMsBetweenTransmits;
    }
    
    void Transmit()
    {
        Serial.println(F("1")); PAL.Delay(20);
        // Complete message and determine the size of the data
        uint8_t bytesUsed = msg_.Finalize();
        
        Serial.println(F("2")); PAL.Delay(20);
        // Transmit and ReTransmit according to configuration
        uint8_t transmitCountRemaining = transmitCount_;
        while (transmitCountRemaining)
        {
            Serial.println(F("3")); PAL.Delay(20);
            fnBeforeModemStart_();
            PAL.Delay(radioWarmupDurationMs_);
            Serial.println(F("4")); PAL.Delay(20);
            TransmitPrivate(buf_, bytesUsed);
            Serial.println(F("5")); PAL.Delay(20);
            fnAfterModemEnd_();
            
            --transmitCountRemaining;
            
            if (transmitCountRemaining)
            {
                PAL.Delay(delayMsBetweenTransmits_);
            }
        }
        
        Serial.write(buf_, bytesUsed);
        Serial.println();
        PAL.Delay(500);
        
        Serial.println(F("6")); PAL.Delay(20);
        msg_.Reset();
        Serial.println(F("7")); PAL.Delay(20);
    }
    
private:

    void TransmitPrivate(uint8_t *buf, uint8_t bufLen)
    {
        // Prepare Flag bytes for Start and End
        uint8_t flagList[]  = { 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E };
        uint8_t flagListLen = sizeof(flagList);

        // Keep reusable indicator for whether modem should bit-stuff
        uint8_t bitStuff = 0;
        
        // Start up modem
        modem_.Start();

        // Send Flag delimiters to start
        for (uint32_t i = 0; i < flagPackStartCount_; ++i)
        {
            bitStuff = 0;
            modem_.Send(flagList, flagListLen, bitStuff);
        }
        
        // Send actual AX25UI data, plus the checksum which lives within that
        // buffer (despite the checksum being part of HDLC technically)
        bitStuff = 1;
        modem_.Send(buf, bufLen, bitStuff);
        
        // Send Flag delimiters to end
        for (uint32_t i = 0; i < flagPackEndCount_; ++i)
        {
            bitStuff = 0;
            modem_.Send(flagList, flagListLen, bitStuff);
        }
        
        // Shut down modem
        modem_.Stop();
    }


private:

    uint8_t buf_[BUF_SIZE];

    AX25UIMessage      msg_;
    ModemBell202Type   modem_;
    
    function<void()> fnBeforeModemStart_;
    function<void()> fnAfterModemEnd_;
    
    uint32_t radioWarmupDurationMs_;
    
    uint8_t  transmitCount_;
    uint32_t delayMsBetweenTransmits_;
    
    uint32_t flagPackStartCount_;
    uint32_t flagPackEndCount_;
};


#endif  // __AX_25_UI_MESSAGE_TRANSMITTER_H__

























