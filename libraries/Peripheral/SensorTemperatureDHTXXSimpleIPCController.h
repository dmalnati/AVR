#ifndef __SENSOR_TEMPERATURE_DHTXX_SIMPLE_IPC_CONTROLLER_H__
#define __SENSOR_TEMPERATURE_DHTXX_SIMPLE_IPC_CONTROLLER_H__


#include "SensorTemperatureDHTXX.h"
#include "MessageHandlerSimpleIPC.h"


class SensorTemperatureDHTXXSimpleIPCController
: public MessageHandlerSimpleIPC
{
    enum MessageType
    {
        REQ_GET_TEMPERATURE = 10,
        REP_GET_TEMPERATURE = 11
    };
    
    // Nothing -- not valid, so commented out
    // struct MessageReqGetTemperature
    // {
    //
    // };
    
    struct MessageRepGetTemperature
    {
        uint8_t tempF;
        uint8_t tempC;
        uint8_t pctHumidity;
        uint8_t heatIndex;
    };
    
public:
    SensorTemperatureDHTXXSimpleIPCController()
    : s_(NULL)
    {
        // Nothing to do
    }
    
    void Init(SensorTemperatureDHTXX *s)
    {
        s_ = s;
    }
    
    uint8_t OnMessageReqGetTemperature(MessageRepGetTemperature *msgRep)
    {
        uint8_t retVal = 0;
        
        SensorTemperatureDHTXX::Measurement m;
        
        if (s_->GetMeasurement(&m))
        {
            retVal = 1;
            
            msgRep->tempF       = m.tempF;
            msgRep->tempC       = m.tempC;
            msgRep->pctHumidity = m.pctHumidity;
            msgRep->heatIndex   = m.heatIndex;
        }
        
        return retVal;
    }

private:

    // Implement MessageHandler interface
    virtual uint8_t HandleMessage(Message *msgIn, Message *msgOut)
    {
        uint8_t retVal = 1;
        
        if (s_)
        {
            switch (msgIn->messageType)
            {
                case MessageType::REQ_GET_TEMPERATURE:
                {
                    MessageRepGetTemperature *msgRep = (MessageRepGetTemperature *)msgOut->buf;
                    
                    if (OnMessageReqGetTemperature(msgRep))
                    {
                        msgOut->messageType = MessageType::REP_GET_TEMPERATURE;
                        msgOut->bufLen      = sizeof(MessageRepGetTemperature);
                    }
                    
                    break;
                }
                
                default: retVal = 0; break;
            }
        }
        
        
        return retVal;
    }







    SensorTemperatureDHTXX *s_;
};



#endif  // __SENSOR_TEMPERATURE_DHTXX_SIMPLE_IPC_CONTROLLER_H__












