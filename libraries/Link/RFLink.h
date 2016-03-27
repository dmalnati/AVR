#ifndef __RF_LINK_H__
#define __RF_LINK_H__


#include "PAL.h"
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


template <typename T>
class RFLink_Raw
: private TimedEventHandler
{
    typedef void (T::*OnRxAvailableCbFn)(uint8_t *buf, uint8_t bufSize);
    
public:
    static const uint8_t C_IDLE  = 1;
    static const uint8_t C_TIMED = 0;
    static const uint8_t C_INTER = 0;
    
    static const uint16_t DEFAULT_BAUD = 2000;
    
    static const uint8_t  POLL_PERIOD_MS = 10;

    RFLink_Raw()
    : obj_(NULL)
    , rxCb_(NULL)
    , txEnabled_(0)
    {
        // Nothing to do
    }
    
    virtual ~RFLink_Raw()
    {
        if (rxCb_) { vw_rx_stop(); }
    }
    
    uint8_t Init(T                 *obj,
                 int8_t             rxPin,
                 OnRxAvailableCbFn  rxCb,
                 int8_t             txPin,
                 uint16_t           baud = DEFAULT_BAUD)
    {
        obj_       = obj;
        rxCb_      = (rxPin != -1 && obj_ ? rxCb : NULL);
        txEnabled_ = (txPin != -1);
        
        // Determine Arduino pin for handoff to VirtualWire
        uint8_t arduinoRxPin = PAL.GetArduinoPinFromPhysicalPin(rxPin);
        uint8_t arduinoTxPin = PAL.GetArduinoPinFromPhysicalPin(txPin);
        
        // Handle RX functions
        if (rxCb_) { vw_set_rx_pin(arduinoRxPin); }
        
        // Handle TX functions
        if (txEnabled_) { vw_set_tx_pin(arduinoTxPin); }
        
        // Configure bitrate, common between RX and TX
        if (rxCb_ || txEnabled_) { vw_setup(baud); }
        
        // Handle RX functions
        if (rxCb_) { vw_rx_start(); }
        
        // Start Idle processing if necessary
        if (rxCb_) { RegisterForTimedEventInterval(POLL_PERIOD_MS); }
        
        return (rxCb_ || txEnabled_);
    }
    
    uint8_t Send(uint8_t* buf, uint8_t len)
    {
        uint8_t retVal = 0;

        // VirtualWire synchronously finishes sending the prior message if the
        // next is sent before completion.
        // That is an appropriate simplification.
        if (txEnabled_)
        {
            // pass-through to VirtualWire
            retVal = vw_send(buf, len);
        }
        
        return retVal;
    }
    
    uint8_t GetTxBuf(uint8_t **buf, uint8_t *bufLen)
    {
        *buf    = txBuf_;
        *bufLen = VW_MAX_MESSAGE_LEN;
        
        return 1;
    }
    
    uint8_t SendFromTxBuf(uint8_t bufLen)
    {
        return Send(txBuf_, bufLen);
    }
    
private:
    // Implement the Timed event
    virtual void OnTimedEvent()
    {
        if (rxCb_) { CheckForRxData(); }
    }
    
    void CheckForRxData()
    {
        uint8_t *buf    = NULL;
        uint8_t  bufLen = VW_MAX_MESSAGE_LEN;
        
        if (vw_get_message(&buf, &bufLen))
        {
            // Call back listener
            ((*obj_).*rxCb_)(buf, bufLen);
            
            vw_get_message_completed();
        }
    }
        
    // Object owning the callback functions
    T *obj_;
    
    // RX Members
    OnRxAvailableCbFn rxCb_;

    // TX Members
    uint8_t txBuf_[VW_MAX_MESSAGE_LEN];
    uint8_t txEnabled_;
};








// Stand in the way of RFLink_Raw and data handed back to the application

struct RFLinkHeader
{
    uint8_t realm;
    uint8_t srcAddr;
    uint8_t dstAddr;
    uint8_t protocolId;
};

template <typename T>
class RFLink
: private RFLink_Raw<RFLink<T>>
{
    typedef void (T::*OnRxAvailableCbFn)(RFLinkHeader *hdr,
                                         uint8_t      *buf,
                                         uint8_t       bufSize);
    
public:

    static const uint8_t C_IDLE  = RFLink_Raw<RFLink<T>>::C_IDLE;
    static const uint8_t C_TIMED = RFLink_Raw<RFLink<T>>::C_TIMED;
    static const uint8_t C_INTER = RFLink_Raw<RFLink<T>>::C_INTER;


    RFLink()
    : obj_(NULL)
    , rxCb_(NULL)
    , txEnabled_(0)
    , realm_(0)
    , srcAddr_(0)
    , dstAddr_(0)
    , promiscuousMode_(0)
    {
        // Nothing to do
    }

    uint8_t Init(
         T                 *obj,
         int8_t             rxPin,
         OnRxAvailableCbFn  rxCb,
         int8_t             txPin,
         uint16_t           baud = RFLink_Raw<RFLink<T>>::DEFAULT_BAUD)
    {
        uint8_t retVal = RFLink_Raw<RFLink>::Init(
            this,
            rxPin,
            rxCb ? &RFLink<T>::OnRxAvailable : NULL, // intercepted
            txPin,
            baud
        );
        
        if (retVal)
        {
            obj_       = obj;
            rxCb_      = rxCb;
            txEnabled_ = (txPin != -1),
            
            retVal = (rxCb_ || txEnabled_);
        }
        
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
    
    void EnablePromiscuousMode()
    {
        promiscuousMode_ = 1;
    }
    
    void DisablePromiscuousMode()
    {
        promiscuousMode_ = 0;
    }
    
    // Encapsulate
    uint8_t SendTo(uint8_t  dstAddr,
                   uint8_t  protocolId,
                   uint8_t *buf,
                   uint8_t  bufSize)
    {
        uint8_t retVal = 0;
        
        // First check to see if it can all fit
        if (((sizeof(RFLinkHeader) + bufSize) <= VW_MAX_MESSAGE_LEN) && txEnabled_)
        {
            // Reserve space to craft outbound message
            uint8_t *bufNew     = NULL;
            uint8_t  bufSizeNew = 0;
            RFLink_Raw<RFLink<T>>::GetTxBuf(&bufNew, &bufSizeNew);
            
            bufSizeNew = sizeof(RFLinkHeader) + bufSize;
            
            // Fill out header
            RFLinkHeader *hdr = (RFLinkHeader *)&(bufNew[0]);
            
            hdr->realm      = realm_;
            hdr->srcAddr    = srcAddr_;
            hdr->dstAddr    = dstAddr;
            hdr->protocolId = protocolId;

            // Copy in user data
            memcpy(&(bufNew[sizeof(RFLinkHeader)]), buf, bufSize);
            
            // Hand off to RFLink_Raw, note success value
            retVal = RFLink_Raw<RFLink<T>>::SendFromTxBuf(bufSizeNew);
        }
        
        return retVal;
    }
    
    uint8_t Send(uint8_t protocolId, uint8_t *buf, uint8_t bufSize)
    {
        return SendTo(dstAddr_, protocolId, buf, bufSize);
    }

    
private:
    // Intercepted from RFLink_Raw
    void OnRxAvailable(uint8_t *buf, uint8_t bufSize)
    {
        // Filter before passing up.  Must have at least full header.
        if (bufSize >= sizeof(RFLinkHeader))
        {
            RFLinkHeader *hdr = (RFLinkHeader *)buf;

            if ((hdr->realm == realm_ && hdr->dstAddr == srcAddr_) || 
                promiscuousMode_)
            {
                // Filter criteria passed.
                
                // Pass data upward, but stripping off the header data
                ((*obj_).*rxCb_)(hdr,
                                 &(buf[sizeof(RFLinkHeader)]),
                                 bufSize - sizeof(RFLinkHeader));
            }
        }
    }
    
    // Receive members
    T                 *obj_;
    OnRxAvailableCbFn  rxCb_;
    
    // Send members
    uint8_t txEnabled_;
    
    // Addressing members
    uint8_t realm_;
    uint8_t srcAddr_;
    uint8_t dstAddr_;
    uint8_t promiscuousMode_;
};










#endif // __RF_LINK_H__

















