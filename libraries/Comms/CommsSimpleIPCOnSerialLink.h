#ifndef __COMMS_SIMPLE_IPC_ON_SERIAL_LINK_H__
#define __COMMS_SIMPLE_IPC_ON_SERIAL_LINK_H__


#include "LinkToProtocolHandlerAdapterSerialLink.h"
#include "MessageHandlerSimpleIPC.h"


template <uint8_t PAYLOAD_CAPACITY,
          uint8_t NUM_MSG_HANDLERS,
          uint8_t MAX_MSG_SIZE>
class CommsSimpleIPCOnSerialLink
{
public:

    void Init()
    {
        // Register the Protocol Handler with the Adapter
        ltpha_.RegisterProtocolHandler(&ph_);
        
        // Tell the Protocol Handler to use this Adapter to send messages
        ph_.BindLinkToProtocolAdapter(&ltpha_);
    }

    uint8_t RegisterProtocolMessageHandler(MessageHandlerSimpleIPC *msgHandler)
    {
        return ph_.RegisterProtocolMessageHandler(msgHandler);
    }

private:

    LinkToProtocolHandlerAdapterSerialLink<PAYLOAD_CAPACITY, 1>  ltpha_;
    ProtocolHandlerSimpleIPC<NUM_MSG_HANDLERS, MAX_MSG_SIZE>     ph_;
};





#endif  // __COMMS_SIMPLE_IPC_ON_SERIAL_LINK_H__


