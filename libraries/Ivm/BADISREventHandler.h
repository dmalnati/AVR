#ifndef __BADISR_EVENT_HANDLER_H__
#define __BADISR_EVENT_HANDLER_H__


#include "Ivm.h"


class BADISREventHandler
{
public:
    BADISREventHandler() { }
    virtual ~BADISREventHandler() { DeRegisterForBADISREvent(); }
    
    uint8_t RegisterForBADISREvent();
    uint8_t DeRegisterForBADISREvent();
    
    virtual void OnBADISREvent() = 0;

private:
};


#endif