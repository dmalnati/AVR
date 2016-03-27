#ifndef __PIN_STATE_SIMPLE_IPC_CONTROLLER_H__
#define __PIN_STATE_SIMPLE_IPC_CONTROLLER_H__


#include "PinState.h"
#include "MessageHandlerSimpleIPC.h"


class PinStateSimpleIPCController
: public MessageHandlerSimpleIPC
{
private:

    enum MessageType
    {
        GET_PIN_STATE = 1,
        SET_PIN_STATE = 2,
        PIN_STATE     = 3
    };

    struct MessageGetPinState
    {
        uint8_t pin;
    };

    struct MessageSetPinState
    {
        uint8_t pin;
        uint8_t value;
    };

    struct MessagePinStateValue
    {
        uint8_t pin;
        uint8_t value;
    };
    

    
public:
    PinStateSimpleIPCController()
    : ps_(NULL)
    {
        // Nothing to do
    }
    ~PinStateSimpleIPCController() { }
    
    void Init(PinState *ps)
    {
        ps_ = ps;
    }
    
    uint8_t OnMessageGetPinState(MessageGetPinState   *msgReq,
                                 MessagePinStateValue *msgRep)
    {
        msgRep->pin   = msgReq->pin;
        msgRep->value = ps_->GetPinState(msgReq->pin);
        
        return 1;
    }
    
    uint8_t OnMessageSetPinState(MessageSetPinState *msg)
    {
        ps_->SetPinState(msg->pin, msg->value);
        
        return 1;
    }
    
    // Implement MessageHandler interface
    virtual uint8_t HandleMessage(Message *msgIn, Message *msgOut)
    {
        uint8_t retVal = 1;
        
        if (ps_)
        {
            switch (msgIn->messageType)
            {
                case MessageType::GET_PIN_STATE:
                {
                    if (msgIn->bufLen >= sizeof(MessageGetPinState))
                    {
                        MessageGetPinState   *msgReq = (MessageGetPinState *)msgIn->buf;
                        MessagePinStateValue *msgRep = (MessagePinStateValue *)msgOut->buf;
                        
                        if (OnMessageGetPinState(msgReq, msgRep))
                        {
                            msgOut->messageType = MessageType::PIN_STATE;
                            msgOut->bufLen      = sizeof(MessagePinStateValue);
                        }
                    }
                    
                    break;
                }
                
                case MessageType::SET_PIN_STATE:
                {
                    if (msgIn->bufLen >= sizeof(MessageSetPinState))
                    {
                        MessageSetPinState *msg = (MessageSetPinState *)msgIn->buf;
                        
                        OnMessageSetPinState(msg);
                    }
                    
                    break;
                }
                
                default: retVal = 0; break;
            }
        }
        else
        {
            retVal = 0;
        }
        
        return retVal;
    }

    PinState *ps_;
};



#endif  // __PIN_STATE_SIMPLE_IPC_CONTROLLER_H__



