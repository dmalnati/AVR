#include "BADISREventHandler.h"


uint8_t
BADISREventHandler::RegisterForBADISREvent()
{
    return Ivm::GetInstance().RegisterBADISREventHandler(this);
}

uint8_t
BADISREventHandler::DeRegisterForBADISREvent()
{
    return Ivm::GetInstance().DeRegisterBADISREventHandler(this);
}
