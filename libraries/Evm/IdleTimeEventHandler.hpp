

//////////////////////////////////////////////////////////////////////
//
// Idle Events
//
//////////////////////////////////////////////////////////////////////

template <typename EvmT>
uint8_t IdleTimeEventHandler<EvmT>::RegisterForIdleTimeEvent()
{
    // Don't allow yourself to be scheduled more than once
    DeRegisterForIdleTimeEvent();
    
    return evm_.RegisterIdleTimeEventHandler(this);
}

template <typename EvmT>
uint8_t IdleTimeEventHandler<EvmT>::DeRegisterForIdleTimeEvent()
{
    return evm_.DeRegisterIdleTimeEventHandler(this);
}


