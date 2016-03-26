#ifndef __PROTOCOL_HANDLER_SIMPLE_IPC_H__
#define __PROTOCOL_HANDLER_SIMPLE_IPC_H__


#include "Container.h"
#include "PAL.h"
#include "ProtocolHandler.h"
#include "LinkToProtocolHandlerAdapter.h"
#include "MessageHandlerSimpleIPC.h"


// Forward declaration
class MessageHandlerSimpleIPC;

struct ProtocolHandlerSimpleIPCMessage
{
    uint8_t  messageType;
    
    uint8_t  bufLen;
    uint8_t *buf;
};
    



template <uint8_t NUM_MSG_HANDLERS, uint8_t MAX_MSG_SIZE = 20>
class ProtocolHandlerSimpleIPC
: public ProtocolHandler
{
private:
    static const uint8_t PROTOCOL_ID = 1;
    
    static const uint8_t BUF_CAPACITY = MAX_MSG_SIZE + 2;
    
    using Message = ProtocolHandlerSimpleIPCMessage;

public:
    ProtocolHandlerSimpleIPC()
    : ltpha_(NULL)
    {
        // nothing to do
    }
    
    ~ProtocolHandlerSimpleIPC() { }

    ////////////////////////////////////////////////////////////////////////////
    //
    // Setup functions
    //
    ////////////////////////////////////////////////////////////////////////////
    uint8_t BindLinkToProtocolAdapter(LinkToProtocolHandlerAdapter *ltpha)
    {
        ltpha_ = ltpha;
        
        return 1;
    }
    
    uint8_t RegisterProtocolMessageHandler(MessageHandlerSimpleIPC *msgHandler)
    {
        return msgHandlerList_.Push(msgHandler);
    }
    
    virtual uint8_t GetProtocolId()
    {
        return PROTOCOL_ID;
    }

    ////////////////////////////////////////////////////////////////////////////
    //
    // Send (via configured outbound interface)
    //
    ////////////////////////////////////////////////////////////////////////////

    Message GetTxMessageBuffer()
    {
        Message msg;
        
        // Set to default values, as well as indicating capacity in the bufLen
        msg.messageType = 0;
        msg.bufLen      = MAX_MSG_SIZE;
        msg.buf         = &(bufTx_[2]);
        
        return msg;
    }
    
    uint8_t Send(Message msg)
    {
        uint8_t retVal = 0;
        
        if (ltpha_)
        {
            // Actually set the message type
            uint16_t messageTypeNetworkByteOrder = PAL.htons(msg.messageType);
            memcpy(bufTx_, &messageTypeNetworkByteOrder, 2);
            
            retVal = ltpha_->Send(PROTOCOL_ID, bufTx_, msg.bufLen + 2);
        }
        
        return retVal;
    }
    
    ////////////////////////////////////////////////////////////////////////////
    //
    // Receive from link (not necessarily the one you send on)
    //
    ////////////////////////////////////////////////////////////////////////////
    
    virtual uint8_t OnProtocolDataEvent(uint8_t  *bufIn,
                                        uint8_t   bufInLen,
                                        uint8_t **bufOut,
                                        uint8_t  *bufOutLen)
    {
        uint8_t retVal = 0;
        
        if (bufInLen >= 2)
        {
            // Create inbound and optional outbound message objects
            Message msgIn;
            Message msgOut = GetTxMessageBuffer();
            
            // Fill out inbound message structure using buffer
            uint16_t messageTypeNetworkByteOrder;
            memcpy(&messageTypeNetworkByteOrder, bufIn, 2);
            
            msgIn.messageType = PAL.ntohs(messageTypeNetworkByteOrder);
            msgIn.bufLen      = bufInLen - 2;
            msgIn.buf         = &(bufIn[2]);
            
            // Find handler for this message
            uint8_t handled = 0;
            for (uint8_t i = 0; i < msgHandlerList_.Size() && !handled; ++i)
            {
                handled = msgHandlerList_[i]->HandleMessage(&msgIn, &msgOut);
            }
            
            // Check if handler has a reply to send now
            if (handled && msgOut.messageType != 0)
            {
                // Adjust TX buffer
                uint16_t messageTypeNetworkByteOrder = PAL.htons(msgOut.messageType);
                memcpy(bufTx_, &messageTypeNetworkByteOrder, 2);
                
                // Set return parameters
                *bufOut    = bufTx_;
                *bufOutLen = msgOut.bufLen + 2;
            }
            
            retVal = handled;
        }
        else
        {
            // Data does not validate, ignore.
        }
        
        return retVal;
    }
    
private:
    
    LinkToProtocolHandlerAdapter                       *ltpha_;
    Queue<MessageHandlerSimpleIPC *, NUM_MSG_HANDLERS>  msgHandlerList_;
    
    uint8_t bufTx_[BUF_CAPACITY];
};



#endif  // __PROTOCOL_HANDLER_SIMPLE_IPC_H__



