#ifndef __LINK_TO_PROTOCOL_HANDLER_ADAPTER_H__
#define __LINK_TO_PROTOCOL_HANDLER_ADAPTER_H__


class LinkToProtocolHandlerAdapter
{
public:
    virtual uint8_t Send(uint8_t protocolId, uint8_t *buf, uint8_t bufSize) = 0;
};



#endif  // __LINK_TO_PROTOCOL_HANDLER_ADAPTER_H__


