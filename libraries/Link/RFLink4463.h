#ifndef __RF_LINK_4463_H__
#define __RF_LINK_4463_H__


#include "PAL.h"
#include "Function.h"
#include "TimedEventHandler.h"
#include "InterruptEventHandler.h"
#include "RF4463.h"


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



class RFLink4463_Raw
{
public:
    static const uint8_t MAX_PACKET_SIZE = 64;

public:
    RFLink4463_Raw(uint8_t pinIrq, uint8_t pinSdn, uint8_t pinSel)
    : radio_(
        PAL.GetArduinoPinFromPhysicalPin(pinIrq),
        PAL.GetArduinoPinFromPhysicalPin(pinSdn),
        PAL.GetArduinoPinFromPhysicalPin(pinSel)
    )
    , ied_(pinIrq, LEVEL_FALLING)
    , sendSync_(1)
    , state_(State::IDLE)
    , goBackToReceiveOnSendComplete_(0)
    {
        ied_.SetCallback([this](uint8_t){
            OnPinChange();
        });
    }

    uint8_t Init()
    {
        ChangeState(State::IDLE);
        
        uint8_t retVal = radio_.init();

        return retVal;
    }
    
    void SetOnMessageReceivedCallback(function<void(uint8_t *buf, uint8_t bufSize)> rxCb)
    {
        rxCb_ = rxCb;
    }

    void SetOnMessageTransmittedCallback(function<void()> txCb)
    {
        txCb_ = txCb;
    }
    
    void SetSendSync(uint8_t val)
    {
        sendSync_ = val;
    }

    uint8_t ModeLowPower()
    {
        uint8_t retVal = 0;

        ChangeState(State::LOW_POWER);

        retVal = radio_.enterStandbyMode();

        return retVal;
    }

    uint8_t ModeReceive()
    {
        uint8_t retVal = 0;

        PreChangeState(State::RECEIVING);

        goBackToReceiveOnSendComplete_ = 1;

        retVal = radio_.rxInit();

        ied_.RegisterForInterruptEvent();

        PostChangeState(State::RECEIVING);

        return retVal;
    }

    // Send behavior:
    // - sync
    //   - function doesn't return until message completely sent
    // - async
    //   - get notified
    //     - callback when message completed and you can send another
    //   - don't get notified
    //
    // If you async send a new message before completion of prior send, function
    // will return an error.
    //
    // If you try to receive or go low power before completion of prior send,
    // no protection is in place and behavior undefined, including never
    // getting a callback about send complete.
    //
    uint8_t Send(uint8_t* buf, uint8_t len)
    {
        uint8_t retVal = 0;

        if (state_ != State::SENDING)
        {
            if (len <= MAX_PACKET_SIZE && len != 0)
            {
                if (sendSync_)
                {
                    retVal = radio_.txPacket(buf, len, sendSync_);

                    ChangeState(State::IDLE);
                }
                else
                {
                    PreChangeState(State::SENDING);

                    ied_.RegisterForInterruptEvent();

                    retVal = radio_.txPacket(buf, len, sendSync_);

                    PostChangeState(State::SENDING);
                }
            }
            else
            {
                retVal = 0;
            }
        }
        else
        {
            // Was already in async sending state, caller tried to send again,
            // so return failure.
            // Continue to allow callback for completion of prior send.
            retVal = 0;
        }
        
        return retVal;
    }

    // Convenience access to buffer for those who want to make use of it between
    // uses within this class.
    // Perfectly ok to use as the buffer to fill before passing to Send().
    void GetTxBuf(uint8_t **buf, uint8_t *bufSize)
    {
        *buf     = buf_;
        *bufSize = MAX_PACKET_SIZE;
    }
    
    
private:

    enum class State : uint8_t
    {
        IDLE = 0,
        LOW_POWER = 1,
        SENDING = 2,
        RECEIVING = 3,
    };

    void OnPinChange()
    {
        if (state_ == State::SENDING)
        {
            ChangeState(State::IDLE);

            txCb_();
        }
        else if (state_ == State::RECEIVING)
        {
            radio_.clrInterrupts();

            uint8_t bufSize = radio_.rxPacket(buf_);

            radio_.rxInit();

            if (bufSize)
            {
                rxCb_(buf_, bufSize);
            }
        }
    }

    // Basically kill events associated with moving oldState->IDLE but don't
    // actually change the state.
    void PreChangeState(State)
    {
        ied_.DeRegisterForInterruptEvent();
    }

    void PostChangeState(State newState)
    {
        if (newState == State::IDLE)
        {
            if (goBackToReceiveOnSendComplete_)
            {
                ModeReceive();
            }
            else
            {
                state_ = newState;
            }
        }
        else
        {
            state_ = newState;
        }
    }

    void ChangeState(State newState)
    {
        PreChangeState(newState);
        PostChangeState(newState);
    }
    

private:

    RF4463 radio_;

    InterruptEventHandlerDelegate ied_;
        
    function<void(uint8_t *buf, uint8_t bufSize)> rxCb_;
    function<void()>                              txCb_;

    uint8_t sendSync_;

    State state_;

    uint8_t goBackToReceiveOnSendComplete_;

    uint8_t buf_[MAX_PACKET_SIZE];
};








// Stand in the way of RFLink4463_Raw and data handed back to the application

struct RFLinkHeader
{
    uint8_t realm;
    uint8_t srcAddr;
    uint8_t dstAddr;
    uint8_t protocolId;
};

class RFLink
: public RFLink4463_Raw
{
public:

    RFLink(uint8_t pinIrq, uint8_t pinSdn, uint8_t pinSel)
    : RFLink4463_Raw(pinIrq, pinSdn, pinSel)
    , realm_(0)
    , srcAddr_(0)
    , dstAddr_(0)
    , protocolId_(0)
    , promiscuousMode_(0)
    {
        RFLink4463_Raw::SetOnMessageReceivedCallback([this](uint8_t *buf, uint8_t bufSize){
            OnRxAvailable(buf, bufSize);
        });
    }
    
    void SetOnMessageReceivedCallback(function<void(RFLinkHeader *hdr,
                                                    uint8_t      *buf,
                                                    uint8_t       bufSize)> rxCb)
    {
        rxCb_ = rxCb;
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
        RFLink4463_Raw::SetSendSync(val);
    }
    
    // Encapsulate
    uint8_t SendTo(uint8_t  dstAddr,
                   uint8_t *buf,
                   uint8_t  bufSize)
    {
        uint8_t retVal = 0;
        
        // First check to see if it can all fit
        if (((sizeof(RFLinkHeader) + bufSize) <= RFLink4463_Raw::MAX_PACKET_SIZE))
        {
            // Reserve space to craft outbound message
            uint8_t *bufNew     = NULL;
            uint8_t  bufSizeNew = 0;
            RFLink4463_Raw::GetTxBuf(&bufNew, &bufSizeNew);
            
            bufSizeNew = sizeof(RFLinkHeader) + bufSize;
            
            // Fill out header
            RFLinkHeader *hdr = (RFLinkHeader *)&(bufNew[0]);
            
            hdr->realm      = realm_;
            hdr->srcAddr    = srcAddr_;
            hdr->dstAddr    = dstAddr;
            hdr->protocolId = protocolId_;

            // Copy in user data
            memcpy(&(bufNew[sizeof(RFLinkHeader)]), buf, bufSize);
            
            // Hand off to RFLink4463_Raw, note success value
            retVal = RFLink4463_Raw::Send(bufNew, bufSizeNew);
        }
        
        return retVal;
    }
    
    // Hide RFLink4463_Raw::Send
    uint8_t Send(uint8_t *buf, uint8_t bufSize)
    {
        return SendTo(dstAddr_, buf, bufSize);
    }

    RFLink4463_Raw *GetLinkRaw()
    {
        return this;
    }

    
private:

    // Intercepted from RFLink4463_Raw
    void OnRxAvailable(uint8_t *buf, uint8_t bufSize)
    {
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










#endif // __RF_LINK_4463_H__

















