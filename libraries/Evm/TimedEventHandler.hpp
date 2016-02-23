


//////////////////////////////////////////////////////////////////////
//
// Timed Events
//
//////////////////////////////////////////////////////////////////////

template <typename EvmT>
uint8_t TimedEventHandler<EvmT>::RegisterForTimedEvent(uint32_t timeout)
{
    // Don't allow yourself to be scheduled more than once.
    // Cache whether this is an interval callback since that
    // gets reset during cancel.
    uint8_t isIntervalCache = isInterval_;
    DeRegisterForTimedEvent();
    isInterval_ = isIntervalCache;
    
    return evm_.RegisterTimedEventHandler(this, timeout);
}

template <typename EvmT>
uint8_t TimedEventHandler<EvmT>::RegisterForTimedEventInterval(uint32_t timeout)
{
    isInterval_ = 1;
    
    return RegisterForTimedEvent(timeout);
}

template <typename EvmT>
uint8_t TimedEventHandler<EvmT>::DeRegisterForTimedEvent()
{
    uint8_t retVal = evm_.DeRegisterTimedEventHandler(this);
    
    // make sure this isn't re-scheduled if interval
    isInterval_ = 0;
    
    return retVal;
}











