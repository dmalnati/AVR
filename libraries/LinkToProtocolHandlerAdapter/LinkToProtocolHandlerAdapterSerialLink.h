#ifndef __LINK_TO_PROTOCOL_HANDLER_ADAPTER_SERIAL_LINK_H__
#define __LINK_TO_PROTOCOL_HANDLER_ADAPTER_SERIAL_LINK_H__


#include "SerialLink.h"
#include "ProtocolHandler.h"
#include "LinkToProtocolHandlerAdapter.h"


template <uint8_t PAYLOAD_CAPACITY, uint8_t PROTOCOL_HANDLER_COUNT>
class LinkToProtocolHandlerAdapterSerialLink
: public LinkToProtocolHandlerAdapter
{
    using ThisClass = LinkToProtocolHandlerAdapterSerialLink<PAYLOAD_CAPACITY,
                                                             PROTOCOL_HANDLER_COUNT>;
    
    using SerialLinkClass = SerialLink<ThisClass, PAYLOAD_CAPACITY>;
    
public:

    void Init()
    {
        serialLink_.Init(this, &ThisClass::OnRxAvailable);
    }
    
    uint8_t RegisterProtocolHandler(ProtocolHandler *ph)
    {
        return phList_.Push(ph);
    }
    
    virtual uint8_t Send(uint8_t protocolId, uint8_t *buf, uint8_t bufSize)
    {
        return serialLink_.Send(protocolId, buf, bufSize);
    }


public:
    void OnRxAvailable(SerialLinkHeader *hdr,
                       uint8_t          *buf,
                       uint8_t           bufSize)
   {
       uint8_t *bufReply     = NULL;
       uint8_t  bufReplySize = 0;
       
       uint8_t handled = 0;
       for (uint8_t i = 0; i < phList_.Size() && !handled; ++i)
       {
           ProtocolHandler *ph = phList_[i];
           
           if (ph->GetProtocolId() == hdr->protocolId)
           {
               handled = ph->OnProtocolDataEvent(buf,
                                                 bufSize,
                                                 &bufReply,
                                                 &bufReplySize);
           }
       }
       
       // Reply if data is returned
       if (handled && bufReply)
       {
           serialLink_.Send(hdr->protocolId, bufReply, bufReplySize);
       }
   }

    SerialLinkClass serialLink_;
    
    Queue<ProtocolHandler *, PROTOCOL_HANDLER_COUNT> phList_;
};


#endif  // __LINK_TO_PROTOCOL_HANDLER_ADAPTER_SERIAL_LINK_H__


