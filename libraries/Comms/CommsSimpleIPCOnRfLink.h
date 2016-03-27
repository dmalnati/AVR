#ifndef __COMMS_SIMPLE_IPC_ON_RF_LINK_H__
#define __COMMS_SIMPLE_IPC_ON_RF_LINK_H__


#include "LinkToProtocolHandlerAdapterRFLink.h"
#include "MessageHandlerSimpleIPC.h"


template <uint8_t NUM_MSG_HANDLERS, uint8_t MAX_MSG_SIZE>
class CommsSimpleIPCOnRFLink
{
public:

    uint8_t Init(uint8_t realm,
                 uint8_t srcAddr,
                 uint8_t dstAddr,
                 int8_t  rxPin,
                 int8_t  txPin)
    {
        uint8_t retVal = 1;
        
        // Initialize Adapter
        retVal &= ltpha_.Init(realm, srcAddr, dstAddr, rxPin, txPin);
        
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

    LinkToProtocolHandlerAdapterRFLink<1>  ltpha_;
    ProtocolHandlerSimpleIPC<NUM_MSG_HANDLERS, MAX_MSG_SIZE>     ph_;
};





#endif  // __COMMS_SIMPLE_IPC_ON_RF_LINK_H__


