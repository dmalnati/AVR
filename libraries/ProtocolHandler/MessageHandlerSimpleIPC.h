#ifndef __MESSAGE_HANDLER_SIMPLE_IPC_H__
#define __MESSAGE_HANDLER_SIMPLE_IPC_H__


#include "ProtocolHandlerSimpleIPC.h"


class MessageHandlerSimpleIPC
{
protected:
    using Message = ProtocolHandlerSimpleIPCMessage;
    
public:
    virtual uint8_t HandleMessage(Message *msgIn, Message *msgOut) = 0;
};


#endif  // __MESSAGE_HANDLER_SIMPLE_IPC_H__