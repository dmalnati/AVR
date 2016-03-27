#ifndef __COMMS_SIMPLE_IPC_ON_SERIAL_LINK_H__
#define __COMMS_SIMPLE_IPC_ON_SERIAL_LINK_H__


#include "LinkToProtocolHandlerAdapterSerialLink.h"
#include "MessageHandlerSimpleIPC.h"


template <uint8_t PAYLOAD_CAPACITY, uint8_t NUM_MSG_HANDLERS, uint8_t MAX_MSG_SIZE>
class CommsSimpleIPCOnSerialLink
{
public:

    uint8_t Init()
    {
        uint8_t retVal = 1;
        
        // Initialize Adapter
        retVal &= ltpha_.Init();
        
        // Register the Protocol Handler with the Adapter
        retVal &= ltpha_.RegisterProtocolHandler(&ph_);
        
        // Tell the Protocol Handler to use this Adapter to send messages
        retVal &= ph_.BindLinkToProtocolAdapter(&ltpha_);
        
        return retVal;
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


