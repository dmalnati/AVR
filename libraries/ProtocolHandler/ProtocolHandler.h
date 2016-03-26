#ifndef __PROTOCOL_HANDLER_H__
#define __PROTOCOL_HANDLER_H__


class ProtocolHandler
{
public:
    virtual uint8_t GetProtocolId() = 0;
    virtual uint8_t OnProtocolDataEvent(uint8_t  *buf,
                                        uint8_t   bufSize,
                                        uint8_t **bufReply,
                                        uint8_t  *bufReplySize) = 0;
};


#endif  // __PROTOCOL_HANDLER_H__