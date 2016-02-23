#ifndef __TEST_NO_DYNAMIC_MEMORY_HPP__
#define __TEST_NO_DYNAMIC_MEMORY_HPP__







template <typename SysT>
IdleTimeEventHandler<SysT>::
IdleTimeEventHandler(SysT &sys)
: sys_(sys)
{
    sys.evm.DoNothingPrivate();
}



#endif  // __TEST_NO_DYNAMIC_MEMORY_HPP__