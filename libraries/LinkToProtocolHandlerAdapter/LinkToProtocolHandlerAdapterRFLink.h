#ifndef __LINK_TO_PROTOCOL_HANDLER_ADAPTER_RF_LINK_H__
#define __LINK_TO_PROTOCOL_HANDLER_ADAPTER_RF_LINK_H__


#include "SerialLink.h"
#include "ProtocolHandler.h"
#include "LinkToProtocolHandlerAdapter.h"


template <uint8_t PROTOCOL_HANDLER_COUNT>
class LinkToProtocolHandlerAdapterRFLink
: public LinkToProtocolHandlerAdapter
{
    using ThisClass = LinkToProtocolHandlerAdapterRFLink<PROTOCOL_HANDLER_COUNT>;
    
    using RFLinkClass = RFLink<ThisClass>;
    
public:

    uint8_t Init(uint8_t realm,
                 uint8_t srcAddr,
                 uint8_t dstAddr,
                 int8_t  rxPin,
                 int8_t  txPin)
    {
        uint8_t retVal =
            rfLink_.Init(realm,
                         srcAddr,
                         this,
                         rxPin,
                         &ThisClass::OnRxAvailable,
                         txPin);

        // Bind to a specific dst addr so Send (not SendTo) works
        rfLink_.SetDstAddr(dstAddr);
        
        return retVal;
    }
    
    uint8_t RegisterProtocolHandler(ProtocolHandler *ph)
    {
        return phList_.Push(ph);
    }
    
    virtual uint8_t Send(uint8_t protocolId, uint8_t *buf, uint8_t bufSize)
    {
        return rfLink_.Send(protocolId, buf, bufSize);
    }


private:
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
           rfLink_.Send(hdr->protocolId, bufReply, bufReplySize);
       }
   }

    RFLinkClass rfLink_;
    
    Queue<ProtocolHandler *, PROTOCOL_HANDLER_COUNT> phList_;
};


#endif  // __LINK_TO_PROTOCOL_HANDLER_ADAPTER_RF_LINK_H__


