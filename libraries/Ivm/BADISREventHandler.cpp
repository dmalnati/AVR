#include "BADISREventHandler.h"


uint8_t
BADISREventHandler::RegisterForBADISREvent()
{
    return Ivm::RegisterBADISREventHandler(this);
}

uint8_t
BADISREventHandler::DeRegisterForBADISREvent()
{
    return Ivm::DeRegisterBADISREventHandler(this);
}
