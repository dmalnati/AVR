#ifndef __COMMS_SIMPLE_IPC_ON_SERIAL_AND_RF_LINK_H__
#define __COMMS_SIMPLE_IPC_ON_SERIAL_AND_RF_LINK_H__


#include "LinkToProtocolHandlerAdapterSerialLink.h"
#include "LinkToProtocolHandlerAdapterRFLink.h"
#include "MessageHandlerSimpleIPC.h"


template <uint8_t PAYLOAD_CAPACITY, uint8_t NUM_MSG_HANDLERS, uint8_t MAX_MSG_SIZE>
class CommsSimpleIPCOnSerialAndRFLink
{
public:

    uint8_t Init(uint8_t realm,
                 uint8_t srcAddr,
                 uint8_t dstAddr,
                 int8_t  rxPin,
                 int8_t  txPin)
    {
        uint8_t retVal = 1;
        
        
        // Initialize SerialLink Adapter
        retVal &= ltphaSerialLink_.Init();
        
        // Register the Protocol Handler with the SerialLink Adapter
        retVal &= ltphaSerialLink_.RegisterProtocolHandler(&ph_);
        
        
        // Initialize RFLink Adapter
        retVal &= ltphaRFLink_.Init(realm, srcAddr, dstAddr, rxPin, txPin);
        
        // Register the Protocol Handler with the RFLink Adapter
        retVal &= ltphaRFLink_.RegisterProtocolHandler(&ph_);
        
        
        // Tell the Protocol Handler to use the RFLink Adapter to send messages
        // by default
        retVal &= ph_.BindLinkToProtocolAdapter(&ltphaRFLink_);
        
        return retVal;
    }

    uint8_t RegisterProtocolMessageHandler(MessageHandlerSimpleIPC *msgHandler)
    {
        return ph_.RegisterProtocolMessageHandler(msgHandler);
    }

private:

    LinkToProtocolHandlerAdapterSerialLink<PAYLOAD_CAPACITY, 1>  ltphaSerialLink_;
    LinkToProtocolHandlerAdapterRFLink<1>                        ltphaRFLink_;
    ProtocolHandlerSimpleIPC<NUM_MSG_HANDLERS, MAX_MSG_SIZE>     ph_;
};





#endif  // __COMMS_SIMPLE_IPC_ON_SERIAL_AND_RF_LINK_H__


