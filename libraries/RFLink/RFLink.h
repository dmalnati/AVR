#ifndef __RF_LINK_H__
#define __RF_LINK_H__


#include "PAL.h"
#include <IdleTimeEventHandler.h>
#include <VirtualWireModified.h>


// Notes about features:
// - Abstracts libs and hw behind the implementation
//   - VirtualWire at first implementation.
// - Synchronous operations unavailable, async callbacks only.
//
// How to use:
// - TX and RX can be used independently or together.
//   - pass -1 for any pin not used
// - TX
//   - Lets caller register for callback when message completes being sent
// - RX
//   - Lets caller register for callback when message arrives
//


template <typename T>
class RFLink_Raw
: private IdleTimeEventHandler
{
    typedef void (T::*OnRxAvailableCbFn)(uint8_t *buf, uint8_t bufSize);
    typedef void (T::*OnTxCompleteCbFn)();
    
public:
    RFLink_Raw()
    : obj_(NULL)
    , rxCb_(NULL)
    , txCb_(NULL)
    , txActive_(0)
    , txActiveLast_(0)
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
                 OnTxCompleteCbFn   txCb,
                 uint16_t           baud = DEFAULT_BAUD)
    {
        obj_  = obj;
        rxCb_ = (rxPin != -1 && obj_ ? rxCb : NULL);
        txCb_ = (txPin != -1 && obj_ ? txCb : NULL);
        
        // Determine Arduino pin for handoff to VirtualWire
        uint8_t arduinoRxPin = PAL.GetArduinoPinFromPhysicalPin(rxPin);
        uint8_t arduinoTxPin = PAL.GetArduinoPinFromPhysicalPin(txPin);
        
        // Handle RX functions
        if (rxCb_) { vw_set_rx_pin(arduinoRxPin); }
        
        // Handle TX functions
        if (txCb_) { vw_set_tx_pin(arduinoTxPin); }
        
        // Configure bitrate, common between RX and TX
        if (rxCb_ || txCb_) { vw_setup(baud); }
        
        // Handle RX functions
        if (rxCb_) { vw_rx_start(); }
        
        // Start Idle processing if necessary
        MaybeStartIdleProcessing();
        
        return (rxCb_ || txCb_);
    }
    
    uint8_t Send(uint8_t* buf, uint8_t len)
    {
        uint8_t retVal = !txActive_ && txCb_;
        
        // Only send if there are no ongoing transmissions.
        //
        // Insanely, VirtualWire seems to lock the entire core to
        // finish sending the prior message.
        // That isn't acceptable, so the burden falls to senders to cope
        // with not being able to send.
        if (retVal)
        {
            // pass-through to VirtualWire
            retVal = vw_send(buf, len);
            
            // if sending not possible, then don't take any further action
            if (retVal)
            {
                // declare that tx is active, need to force it to be true
                // at least once so that subsequent checks will notice a
                // change when it's done
                txActive_ = 1;

                // Become Idle process
                MaybeStartIdleProcessing();
            }
        }
        
        return retVal;
    }
    
    static const uint16_t DEFAULT_BAUD = 2000;
    
private:
    // Implement the Idle event
    virtual void OnIdleTimeEvent()
    {
        if (rxCb_) { CheckForRxData();     }
        if (txCb_) { CheckForTxComplete(); }
    }
    
    void MaybeStartIdleProcessing()
    {
        // When suggested, we should be an Idle process under two conditions:
        // - We are doing RX  -or-
        // - We are doing TX, and a message is currently being sent
        
        if (rxCb_ || txActive_) { RegisterForIdleTimeEvent(); }
    }
    
    void MaybeStopIdleProcessing()
    {
        // The only way this function can be called is if we're either doing
        // TX or RX or both.
        //
        // Given that, the only conditions where we can stop being an Idle
        // process are:
        // - NOT doing RX -and- are doing TX, but no active msg
        
        if (!rxCb_ && !txActive_) { DeRegisterForIdleTimeEvent(); }
    }
    
    void CheckForRxData()
    {
        uint8_t buf[VW_MAX_MESSAGE_LEN];
        uint8_t bufLen = VW_MAX_MESSAGE_LEN;
        
        if (vw_get_message(buf, &bufLen))
        {
            // Call back listener
            ((*obj_).*rxCb_)(buf, bufLen);
        }
    }
    
    void CheckForTxComplete()
    {
        // Record prior TX state
        txActiveLast_ = txActive_;
        
        // Update status of TX -- currently going on?
        txActive_ = vx_tx_active();
        
        // If TX finished since the last time checked, this is an event we care
        // about.
        if (txActive_ == 0 && txActive_ != txActiveLast_)
        {
            // Call back listener
            ((*obj_).*txCb_)();
            
            // Stop being an idle process for now since the one thing TX is
            // trying to be responsive to just happened.
            MaybeStopIdleProcessing();
        }
    }
    
    // Object owning the callback functions
    T                 *obj_;
    
    // RX Members
    OnRxAvailableCbFn rxCb_;

    // TX Members
    OnTxCompleteCbFn  txCb_;
    uint8_t           txActive_;
    uint8_t           txActiveLast_;
};








// Stand in the way of RFLink_Raw and data handed back to the application
template <typename T>
class RFLink
: private RFLink_Raw<RFLink<T>>
{
    typedef void (T::*OnRxAvailableCbFn)(uint8_t  srcAddr,
                                         uint8_t  protocolId,
                                         uint8_t *buf,
                                         uint8_t  bufSize);
    typedef void (T::*OnTxCompleteCbFn)();
    
public:
    RFLink()
    : obj_(NULL)
    , realm_(0)
    , srcAddr_(0)
    , rxCb_(NULL)
    , txCb_(NULL)
    {
        // Nothing to do
    }

    uint8_t Init(
         uint8_t            realm,
         uint8_t            srcAddr,
         T                 *obj,
         int8_t             rxPin,
         OnRxAvailableCbFn  rxCb,
         int8_t             txPin,
         OnTxCompleteCbFn   txCb,
         uint16_t           baud = RFLink_Raw<RFLink<T>>::DEFAULT_BAUD)
    {
        uint8_t retVal = RFLink_Raw<RFLink>::Init(
            this,
            rxPin,
            rxCb ? &RFLink<T>::OnRxAvailable : NULL, // intercepted
            txPin,
            txCb ? &RFLink<T>::OnTxComplete : NULL, // intercepted
            baud
        );
        
        if (retVal)
        {
            obj_     = obj;
            realm_   = realm;
            srcAddr_ = srcAddr;
            rxCb_    = rxCb;
            txCb_    = txCb;
            
            retVal = (rxCb_ || txCb_);
        }
        
        return retVal;
    }
    
    // Encapsulate
    uint8_t SendTo(uint8_t  dstAddr,
                   uint8_t  protocolId,
                   uint8_t *buf,
                   uint8_t  bufSize)
    {
        uint8_t retVal = 0;
        
        // First check to see if it can all fit
        if (((sizeof(Header) + bufSize) <= VW_MAX_MESSAGE_LEN) && txCb_)
        {
            // Reserve space to craft outbound message
            uint8_t bufSizeNew = sizeof(Header) + bufSize;
            uint8_t bufNew[bufSizeNew];
            
            // Fill out header
            Header *hdr = (Header *)&(bufNew[0]);
            
            hdr->realm      = realm_;
            hdr->srcAddr    = srcAddr_;
            hdr->dstAddr    = dstAddr;
            hdr->protocolId = protocolId;

            // Copy in user data
            memcpy(&(bufNew[sizeof(Header)]), buf, bufSize);
            
            // Hand off to RFLink_Raw, note success value
            retVal = RFLink_Raw<RFLink<T>>::Send(bufNew, bufSizeNew);
        }
        
        return retVal;
    }
    
    void SetSrcAddr(uint8_t srcAddr)
    {
        srcAddr_ = srcAddr;
    }
    
    
private:
    struct Header
    {
        uint8_t realm;
        uint8_t srcAddr;
        uint8_t dstAddr;
        uint8_t protocolId;
    };

    // Intercepted from RFLink_Raw
    void OnRxAvailable(uint8_t *buf, uint8_t bufSize)
    {
        // Filter before passing up.  Must have at least full header.
        if (bufSize >= sizeof(Header))
        {
            Header *hdr = (Header *)buf;

            if (hdr->realm == realm_ && hdr->dstAddr == srcAddr_)
            {
                // Filter criteria passed.
                
                // Pass data upward, but stripping off the header data
                ((*obj_).*rxCb_)(hdr->srcAddr,
                                 hdr->protocolId,
                                 &(buf[sizeof(Header) - 1]),
                                 bufSize - sizeof(Header));
            }
        }
    }

    // No need to filter this, simply relay    
    void OnTxComplete()
    {
        ((*obj_).*txCb_)();
    }
    
    
    T                 *obj_;
    uint8_t            realm_;
    uint8_t            srcAddr_;
    OnRxAvailableCbFn  rxCb_;
    OnTxCompleteCbFn   txCb_;
};










#endif // __RF_LINK_H__

















