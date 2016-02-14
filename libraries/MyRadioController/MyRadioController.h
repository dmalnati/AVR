#ifndef __MY_RADIO_CONTROLLER_H__
#define __MY_RADIO_CONTROLLER_H__


#include "PAL.h"
#include <EvmEventHandler.h>
#include <VirtualWire.h>


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


class MyRadioControllerTxCallbackIface
{
public:
    virtual void OnTxComplete() = 0;
};

class MyRadioControllerRxCallbackIface
{
public:
    virtual void OnRxAvailable(uint8_t *buf, uint8_t bufSize) = 0;
};


class MyRadioController : private IdleTimeEventHandler
{
public:
    MyRadioController(int8_t                            rxPin,
                      MyRadioControllerRxCallbackIface *rxCb,
                      int8_t                            txPin,
                      MyRadioControllerTxCallbackIface *txCb,
                      uint16_t                          baud = DEFAULT_BAUD)
    : rxCb_(rxPin != -1 ? rxCb : NULL)
    , txCb_(txPin != -1 ? txCb : NULL)
    , txActive_(0)
    , txActiveLast_(0)
    {
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
    }
    
    ~MyRadioController()
    {
        if (rxCb_) { vw_rx_stop(); }
    }
    
    uint8_t Send(uint8_t* buf, uint8_t len)
    {
        uint8_t retVal = !txActive_;
        
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
    
private:
    static const uint16_t DEFAULT_BAUD = 2000;
    
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
            rxCb_->OnRxAvailable(buf, bufLen);
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
            txCb_->OnTxComplete();
            
            // Stop being an idle process for now since the one thing TX is
            // trying to be responsive to just happened.
            MaybeStopIdleProcessing();
        }
    }
    
    // RX Members
    MyRadioControllerRxCallbackIface *rxCb_;

    // TX Members
    MyRadioControllerTxCallbackIface *txCb_;
    uint8_t                           txActive_;
    uint8_t                           txActiveLast_;
};




#endif // __MY_RADIO_CONTROLLER_H__

















