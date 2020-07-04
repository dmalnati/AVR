#ifndef __RF_LINK_H__
#define __RF_LINK_H__


#include "PAL.h"
#include "Function.h"
#include "TimedEventHandler.h"
#include "VirtualWireModified.h"


// Notes about features:
// - Abstracts libs and hw behind the implementation
//   - VirtualWire at first implementation.
// - Synchronous read unavailable, async callbacks only.
//
// How to use:
// - TX and RX can be used independently or together.
//   - pass -1 for any pin not used
// - RX
//   - Lets caller register for callback when message arrives
//


class RFLink_Raw
{
public:
    static const uint8_t C_IDLE  = 0;
    static const uint8_t C_TIMED = 1;
    static const uint8_t C_INTER = 0;
    
    //static const uint16_t DEFAULT_BAUD = 2000;
    static const uint16_t DEFAULT_BAUD = 100;
    
    static const uint8_t  POLL_PERIOD_MS = 10;

    RFLink_Raw()
    : sendSync_(1)
    {
        // Nothing to do
    }
    
    void SetOnMessageReceivedCallback(function<void(uint8_t *buf, uint8_t bufSize)> rxCb)
    {
        rxCb_ = rxCb;
    }
    
    uint8_t Init(int8_t   rxPin,
                 int8_t   txPin,
                 uint16_t baud = DEFAULT_BAUD)
    {
        // Determine Arduino pin for handoff to VirtualWire
        uint8_t arduinoRxPin = PAL.GetArduinoPinFromPhysicalPin(rxPin);
        uint8_t arduinoTxPin = PAL.GetArduinoPinFromPhysicalPin(txPin);
        
        // Handle RX functions
        if (rxPin != -1) { vw_set_rx_pin(arduinoRxPin); }
        
        // Handle TX functions
        if (txPin != -1) { vw_set_tx_pin(arduinoTxPin); }
        
        // Configure bitrate, common between RX and TX
        if (rxPin != -1 || txPin != -1) { vw_setup(baud); }
        
        // Handle RX functions
        if (rxPin != -1) { vw_rx_start(); }
        
        // Start Idle processing if necessary
        ted_.SetCallback([this](){ CheckForRxData(); });
        if (rxPin != -1) { ted_.RegisterForTimedEventInterval(POLL_PERIOD_MS); }
        
        return (rxPin != -1 || txPin != -1);
    }

    void SetSendSync(uint8_t val)
    {
        sendSync_ = val;
    }
    
    uint8_t Send(uint8_t* buf, uint8_t len)
    {
        uint8_t retVal = 0;

        // VirtualWire synchronously finishes sending the prior message if the
        // next is sent before completion, unless sendSync is enabled, at which
        // point this function doesn't return until the message is fully sent.

        // pass-through to VirtualWire
        retVal = vw_send(buf, len);

        if (sendSync_)
        {
            vw_wait_tx();
        }
        
        return retVal;
    }
    
    uint8_t GetTxBuf(uint8_t **buf, uint8_t *bufLen)
    {
        *buf    = txBuf_;
        *bufLen = VW_MAX_PAYLOAD;
        
        return 1;
    }
    
    uint8_t SendFromTxBuf(uint8_t bufLen)
    {
        return Send(txBuf_, bufLen);
    }
    
private:
    
    void CheckForRxData()
    {
        uint8_t *buf    = NULL;
        uint8_t  bufLen = VW_MAX_PAYLOAD;
        
        if (vw_get_message(&buf, &bufLen))
        {
            // Call back listener
            rxCb_(buf, bufLen);
            
            vw_get_message_completed();
        }
    }
        
    // RX Members
    function<void(uint8_t *buf, uint8_t bufSize)> rxCb_;

    // TX Members
    uint8_t txBuf_[VW_MAX_PAYLOAD];
    
    // Misc
    TimedEventHandlerDelegate ted_;

    uint8_t sendSync_;
};








// Stand in the way of RFLink_Raw and data handed back to the application

struct RFLinkHeader
{
    uint8_t realm;
    uint8_t srcAddr;
    uint8_t dstAddr;
    uint8_t protocolId;
};

class RFLink
: private RFLink_Raw
{
public:

    static const uint8_t C_IDLE  = RFLink_Raw::C_IDLE;
    static const uint8_t C_TIMED = RFLink_Raw::C_TIMED;
    static const uint8_t C_INTER = RFLink_Raw::C_INTER;


    RFLink()
    : realm_(0)
    , srcAddr_(0)
    , dstAddr_(0)
    , protocolId_(0)
    , promiscuousMode_(0)
    {
        // Nothing to do
    }
    
    void SetOnMessageReceivedCallback(function<void(RFLinkHeader *hdr,
                                                    uint8_t      *buf,
                                                    uint8_t       bufSize)> rxCb)
    {
        rxCb_ = rxCb;
    }

    uint8_t Init(
         int8_t   rxPin,
         int8_t   txPin,
         uint16_t baud = RFLink_Raw::DEFAULT_BAUD)
    {
        uint8_t retVal = RFLink_Raw::Init(rxPin, txPin, baud);
        
        // Intercept this interface
        RFLink_Raw::SetOnMessageReceivedCallback([this](uint8_t *buf, uint8_t bufSize){ OnRxAvailable(buf, bufSize); });
        
        return retVal;
    }
    
    void SetRealm(uint8_t realm)
    {
        realm_ = realm;
    }
    
    void SetSrcAddr(uint8_t srcAddr)
    {
        srcAddr_ = srcAddr;
    }
    
    void SetDstAddr(uint8_t dstAddr)
    {
        dstAddr_ = dstAddr;
    }
    
    void SetProtocolId(uint8_t protocolId)
    {
        protocolId_ = protocolId;
    }
    
    void EnablePromiscuousMode()
    {
        promiscuousMode_ = 1;
    }
    
    void DisablePromiscuousMode()
    {
        promiscuousMode_ = 0;
    }

    void SetSendSync(uint8_t val)
    {
        RFLink_Raw::SetSendSync(val);
    }
    
    // Encapsulate
    uint8_t SendTo(uint8_t  dstAddr,
                   uint8_t *buf,
                   uint8_t  bufSize)
    {
        uint8_t retVal = 0;
        
        // First check to see if it can all fit
        if (((sizeof(RFLinkHeader) + bufSize) <= VW_MAX_PAYLOAD))
        {
            // Reserve space to craft outbound message
            uint8_t *bufNew     = NULL;
            uint8_t  bufSizeNew = 0;
            RFLink_Raw::GetTxBuf(&bufNew, &bufSizeNew);
            
            bufSizeNew = sizeof(RFLinkHeader) + bufSize;
            
            // Fill out header
            RFLinkHeader *hdr = (RFLinkHeader *)&(bufNew[0]);
            
            hdr->realm      = realm_;
            hdr->srcAddr    = srcAddr_;
            hdr->dstAddr    = dstAddr;
            hdr->protocolId = protocolId_;

            // Copy in user data
            memcpy(&(bufNew[sizeof(RFLinkHeader)]), buf, bufSize);
            
            // Hand off to RFLink_Raw, note success value
            retVal = RFLink_Raw::SendFromTxBuf(bufSizeNew);
        }
        
        return retVal;
    }
    
    uint8_t Send(uint8_t *buf, uint8_t bufSize)
    {
        return SendTo(dstAddr_, buf, bufSize);
    }

    
private:
    // Intercepted from RFLink_Raw
    void OnRxAvailable(uint8_t *buf, uint8_t bufSize)
    {
        Log("RX Available");

        // Filter before passing up.  Must have at least full header.
        if (bufSize >= sizeof(RFLinkHeader))
        {
            RFLinkHeader *hdr = (RFLinkHeader *)buf;

            if ((hdr->realm      == realm_   &&
                 hdr->dstAddr    == srcAddr_ &&
                 hdr->protocolId == protocolId_) || 
                promiscuousMode_)
            {
                // Filter criteria passed.
                
                // Pass data upward, but stripping off the header data
                rxCb_(hdr,
                      &(buf[sizeof(RFLinkHeader)]),
                      bufSize - sizeof(RFLinkHeader));
            }
        }
    }
    
    // Receive members
    function<void(RFLinkHeader *hdr, uint8_t *buf, uint8_t bufSize)> rxCb_;
    
    // Addressing members
    uint8_t realm_;
    uint8_t srcAddr_;
    uint8_t dstAddr_;
    uint8_t protocolId_;
    uint8_t promiscuousMode_;
};










#endif // __RF_LINK_H__

















