#ifndef __RF_LINK_4463_H__
#define __RF_LINK_4463_H__


#include "PAL.h"
#include "Function.h"
#include "TimedEventHandler.h"
#include "InterruptEventHandler.h"
#include "RH_RF24_mod.h"
#include "Log.h"


//
// How to use:
// - TX and RX can be both be used, but only one at a time.
//   - lib should wait for module to signal send complete before waiting to
//     get signal for message received (both using IRQ pin)
// - Send can be synchronous or not.
// - Low power can be enabled at any point.
// - If a receiver sends, on send complete the mode goes back to receiving.
// - If a receiver goes to low power, it must manually emerge by either
//   sending or receiving again.
//
//
// A pure receiver:
// ----------------
// RFLink4463_Raw radio();
// radio.Init();
// radio.SetOnMessageReceivedCallback();
//   [async wait for message receive]
//
//
// A pure sender:
// --------------
// RFLink4463_Raw radio();
// radio.Init();
// radio.ModeLowPower();
// radio.Send();
// radio.ModeLowPower();
// radio.Send();
//
//
// A hybrid sender/receiver:
// -------------------------
// RFLink4463_Raw radio();
// radio.Init();
// radio.Send();
// radio.SetOnMessageReceivedCallback();
// radio.ModeReceive();
//   [async wait for message receive]
// radio.Send(syncronous=1);
//   [async wait for message receive]
// radio.Send(synchronous=0)
//   [async wait for send complete]
//   [async wait for message receive]
// 



class RFLink_Raw
{
public:
    static const uint8_t MAX_PACKET_SIZE = 64;

public:
    RFLink_Raw(uint8_t pinIrq, uint8_t pinSdn, uint8_t pinSel)
    : radio_(pinSel, pinIrq, pinSdn)
    , goBackToReceiveOnSendComplete_(0)
    {
        // Nothing to do
    }

    uint8_t Init()
    {
        return radio_.init();
    }
    
    void SetOnMessageReceivedCallback(function<void(uint8_t *buf, uint8_t bufSize)> rxCb)
    {
        rxCb_ = rxCb;

        radio_.SetOnMessageReceivedCallback([this](uint8_t *buf, uint8_t bufLen){
            // Stay in receiving mode
            ModeReceive();

            if (bufLen)
            {
                rxCb_(buf, bufLen);
            }
        });
    }

    void SetOnMessageTransmittedCallback(function<void()> txCb)
    {
        txCb_ = txCb;

        radio_.SetOnMessageTransmittedCallback([this](){
            // Go back to receive first, as callback may decide to send
            // again and we don't want to cancel that immediately afterward
            if (goBackToReceiveOnSendComplete_)
            {
                ModeReceive();
            }

            txCb_();
        });
    }
    
    uint8_t ModeReceive()
    {
        goBackToReceiveOnSendComplete_ = 1;

        radio_.setModeRx();

        return 1;
    }

    // Send behavior:
    // - async
    //   - get notified
    //     - callback when message completed and you can send another
    //   - don't get notified
    //
    // You cannot send a subsequent message until prior message send callback
    // is executed and the function will return false.
    //
    // If you try to receive or go low power before completion of prior send,
    // no protection is in place and behavior undefined, including never
    // getting a callback about send complete.
    //
    uint8_t Send(uint8_t *buf, uint8_t len)
    {
        uint8_t retVal = 0;

        if (len <= MAX_PACKET_SIZE && len != 0)
        {
            retVal = radio_.send(buf, len);
        }

        return retVal;
    }

    RH_RF24_mod &GetRadio()
    {
        return radio_;
    }

private:

    RH_RF24_mod radio_;

    uint8_t goBackToReceiveOnSendComplete_;

    function<void(uint8_t *buf, uint8_t bufSize)> rxCb_;
    function<void()>                              txCb_;
};








// Provide addressing/filtering on top of raw RF

struct RFLinkHeader
{
    uint8_t realm;
    uint8_t srcAddr;
    uint8_t dstAddr;
    uint8_t protocolId;
};

class RFLink
: public RFLink_Raw
{
public:

    static const uint8_t MAX_PACKET_SIZE = RFLink_Raw::MAX_PACKET_SIZE - sizeof(RFLinkHeader);


public:

    RFLink(uint8_t pinIrq, uint8_t pinSdn, uint8_t pinSel)
    : RFLink_Raw(pinIrq, pinSdn, pinSel)
    , realm_(0)
    , srcAddr_(0)
    , dstAddr_(0)
    , receiveBroadcast_(0)
    , protocolId_(0)
    , promiscuousMode_(0)
    {
        // Nothing to do
    }
    
    void SetOnMessageReceivedCallback(function<void(RFLinkHeader *hdr,
                                                    uint8_t      *buf,
                                                    uint8_t       bufSize)> rxCb)
    {
        RFLink_Raw::SetOnMessageReceivedCallback([this](uint8_t *buf, uint8_t bufSize){
            OnRxAvailable(buf, bufSize);
        });

        rxCb_ = rxCb;
    }
    
    void SetRealm(uint8_t realm)
    {
        realm_ = realm;
    }

    uint8_t GetRealm() const
    {
        return realm_;
    }
    
    void SetSrcAddr(uint8_t srcAddr)
    {
        srcAddr_ = srcAddr;
    }

    uint8_t GetSrcAddr() const
    {
        return srcAddr_;
    }
    
    void SetDstAddr(uint8_t dstAddr)
    {
        dstAddr_ = dstAddr;
    }

    uint8_t GetDstAddr() const
    {
        return dstAddr_;
    }
    
    void SetProtocolId(uint8_t protocolId)
    {
        protocolId_ = protocolId;
    }

    uint8_t GetProtocolId() const
    {
        return protocolId_;
    }

    void SetReceiveBroadcast(uint8_t receiveBroadcast)
    {
        receiveBroadcast_ = receiveBroadcast;
    }

    uint8_t GetReceiveBroadcast() const
    {
        return receiveBroadcast_;
    }
    
    void SetPromiscuousMode(uint8_t val)
    {
        promiscuousMode_ = val;
    }

    uint8_t GetPromiscuousMode() const
    {
        return promiscuousMode_;
    }

    // Encapsulate
    uint8_t SendTo(uint8_t  dstAddr,
                   uint8_t *buf,
                   uint8_t  bufSize)
    {
        uint8_t retVal = 0;
        
        // First check to see if it can all fit
        if (((sizeof(RFLinkHeader) + bufSize) <= RFLink_Raw::MAX_PACKET_SIZE))
        {
            // Reserve space to craft outbound message
            uint8_t bufSizeNew = sizeof(RFLinkHeader) + bufSize;
            
            // Fill out header
            RFLinkHeader *hdr = (RFLinkHeader *)buf_;
            
            hdr->realm      = realm_;
            hdr->srcAddr    = srcAddr_;
            hdr->dstAddr    = dstAddr;
            hdr->protocolId = protocolId_;

            // Copy in user data
            memcpy(&(buf_[sizeof(RFLinkHeader)]), buf, bufSize);
            
            // Hand off to RFLink4463_Raw, note success value
            retVal = RFLink_Raw::Send(buf_, bufSizeNew);
        }
        
        return retVal;
    }
    
    // Hide RFLink4463_Raw::Send
    uint8_t Send(uint8_t *buf, uint8_t bufSize)
    {
        return SendTo(dstAddr_, buf, bufSize);
    }

    RFLink_Raw *GetLinkRaw()
    {
        return this;
    }

    
private:

    void OnRxAvailable(uint8_t *buf, uint8_t bufSize)
    {
        // Filter before passing up.  Must have at least full header.
        if (bufSize >= sizeof(RFLinkHeader))
        {
            RFLinkHeader *hdr = (RFLinkHeader *)buf;

            // Calculate whether to pass along this message
            uint8_t realmOk    = (hdr->realm == realm_);
            uint8_t addrOk     = (hdr->dstAddr == srcAddr_) ||
                                 (hdr->dstAddr == 255 && receiveBroadcast_);
            uint8_t protocolOk = (hdr->protocolId == protocolId_);

            if ((realmOk && addrOk && protocolOk) || promiscuousMode_)
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
    uint8_t receiveBroadcast_;
    uint8_t protocolId_;
    uint8_t promiscuousMode_;

    uint8_t buf_[RFLink_Raw::MAX_PACKET_SIZE + sizeof(RFLinkHeader)];
};










#endif // __RF_LINK_4463_H__

















