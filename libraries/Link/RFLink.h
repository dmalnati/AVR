#ifndef __RF_LINK_H__
#define __RF_LINK_H__


#include "PAL.h"
#include "Function.h"
#include "TimedEventHandler.h"
#include "InterruptEventHandler.h"
#include "RFSI4463PROPacket.h"
#include "Log.h"
#include "StrFormat.h"


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

    static const uint8_t MAX_PACKET_SIZE = RFSI4463PROPacket::MAX_PACKET_SIZE;


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

            rxCb_(buf, bufLen);
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

    function<void()> GetOnMessageTransmittedCallback() const
    {
        return txCb_;
    }
    
    uint8_t ModeReceive()
    {
        uint8_t retVal = 1;

        goBackToReceiveOnSendComplete_ = 1;

        radio_.setModeRx();

        return retVal;
    }

    // max 127
    void SetTxPower(uint8_t power)
    {
        radio_.setTxPower(power);
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
        IOVec iov;

        iov.buf    = buf;
        iov.bufLen = len;

        return Sendv(&iov, 1);
    }

    uint8_t Sendv(IOVec *iovList, uint8_t iovListLen)
    {
        return radio_.sendv(iovList, iovListLen);
    }

    uint8_t IsOperable()
    {
        return radio_.IsOperable();
    }

    void DumpStatus()
    {
        Log(P("Operable: "), radio_.IsOperable());
        Log(P("Mode    : "), radio_._mode);
        DumpStats();
    }

    void DumpStats()
    {
        RFSI4463PROPacket::Stats stats = radio_.GetStats();

        char buf[StrFormat::COMMAS_BUF_SIZE_NEEDED_32];

        Log(P("countErrReInitReqd : "), StrFormat::U32ToStrCommasPadLeft(buf, stats.countErrReInitReqd));
        Log(P("countErrInvalidSync: "), StrFormat::U32ToStrCommasPadLeft(buf, stats.countErrInvalidSync));
        Log(P("countErrCrc        : "), StrFormat::U32ToStrCommasPadLeft(buf, stats.countErrCrc));
        Log(P("countErrRxOverflow : "), StrFormat::U32ToStrCommasPadLeft(buf, stats.countErrRxOverflow));
        Log(P("countErrTxOverflow : "), StrFormat::U32ToStrCommasPadLeft(buf, stats.countErrTxOverflow));
        Log(P("countErrTxEarly    : "), StrFormat::U32ToStrCommasPadLeft(buf, stats.countErrTxEarly));

        Log(P("countIrqModem      : "), StrFormat::U32ToStrCommasPadLeft(buf, stats.countIrqModem));
        Log(P("countIrqPh         : "), StrFormat::U32ToStrCommasPadLeft(buf, stats.countIrqPh));

        Log(P("countPacketsRx     : "), StrFormat::U32ToStrCommasPadLeft(buf, stats.countPacketsRx));
        Log(P("countBytesRx       : "), StrFormat::U32ToStrCommasPadLeft(buf, stats.countBytesRx));

        Log(P("countPacketsTx     : "), StrFormat::U32ToStrCommasPadLeft(buf, stats.countPacketsTx));
        Log(P("countBytesTx       : "), StrFormat::U32ToStrCommasPadLeft(buf, stats.countBytesTx));
    }

    RFSI4463PROPacket &GetRadio()
    {
        return radio_;
    }

private:

    RFSI4463PROPacket radio_;

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

    uint8_t SendTo(uint8_t  dstAddr,
                   uint8_t *buf,
                   uint8_t  bufSize)
    {
        uint8_t retVal = 0;
        
        // Fill out header
        RFLinkHeader hdr;
        
        hdr.realm      = realm_;
        hdr.srcAddr    = srcAddr_;
        hdr.dstAddr    = dstAddr;
        hdr.protocolId = protocolId_;

        // Prepare IO vector to send
        IOVec iov[] = {
            {
                .buf    = (uint8_t *)&hdr,
                .bufLen = sizeof(hdr),
            },
            {
                .buf    = buf,
                .bufLen = bufSize,
            }
        };
        
        // Hand off to RFLink4463_Raw
        retVal = RFLink_Raw::Sendv(iov, 2);
        
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
};










#endif // __RF_LINK_H__

















