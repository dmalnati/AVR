#ifndef __MY_RADIO_CONTROLLER_H__
#define __MY_RADIO_CONTROLLER_H__


#include <EvmCallback.h>
#include <VirtualWire.h>


// Notes about features:
// - Abstracts libs and hw behind the implementation
//   - VirtualWire at first implementation.
// - Lets caller be told when a message has been sent in three ways:
//   - boolean pollable value
//   - subclass this object and wait for internal callback
// - Other
//   - synchronous wait not offered, have to use one of the above


class MyRadioControllerCallbackIface
{
public:
    virtual void OnTxFinished() = 0;
};


class MyRadioController : private IdleCallback
{
public:
    MyRadioController(uint8_t pin, uint16_t baud = DEFAULT_BAUD)
    : pin_(pin)
    , txActive_(0)
    , txActiveLast_(0)
    {
        pinMode(pin, OUTPUT);
        
        // init the VirtualWire libs
        vw_set_tx_pin(pin);
        vw_setup(baud);
    }
    
    uint8_t Send(uint8_t* buf, uint8_t len)
    {
        uint8_t txActive = GetTxActive();
        uint8_t retVal   = txActive;
        
        // Only send if there are no ongoing transmissions.
        //
        // Insanely, VirtualWire seems to lock the entire core to
        // (I'm guessing) finish sending the prior message.
        // That isn't acceptable, so the burden falls to senders to cope
        // with not being able to send.
        if (!txActive)
        {
            // pass-through to VirtualWire
            retVal = vw_send(buf, len);
            
            // update status
            txActive_ = vx_tx_active();

            // Become Idle process
            Start();
        }
        
        return retVal;
    }
    
    void SetCallback(MyRadioControllerCallbackIface *cb) { cb_ = cb; }
    
    // Users can poll this to find out state of TX
    uint8_t GetTxActive() const { return txActive_; }
    
    // Sub-classes should implement this to find out about this event
    virtual void OnTxFinished() { }

private:
    static const uint16_t DEFAULT_BAUD = 2000;
    
    // Implement the IdleCallback notification
    virtual void OnCallback()
    {
        // debug, watch when this thing is acting as an idle proc
        digitalWrite(0, HIGH);
        digitalWrite(0, LOW);
        
        // Check status of TX -- currently going on?
        txActive_ = vx_tx_active();
        
        // If TX finished since the last time checked, this is an event we care
        // about.
        if (txActive_ == 0 && txActive_ != txActiveLast_)
        {
            // Stop being an idle process for now since the one thing you're
            // trying to be responsive to just happened.
            Stop();
            
            // Call back any child classes
            OnTxFinished();
            
            // Call back any listeners
            if (cb_) { cb_->OnTxFinished(); }
        }
        
        txActiveLast_ = txActive_;
    }
    
    uint8_t pin_;
    uint8_t txActive_;
    uint8_t txActiveLast_;
    
    MyRadioControllerCallbackIface *cb_;
};




#endif // __MY_RADIO_CONTROLLER_H__

















